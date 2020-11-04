/**
 * A BLE client example that is rich in capabilities.
 */

#include "BLEDevice.h"

const String sketchName = "ESP32 HRM BLE Smart Fan";
unsigned char PWM = 23;       // Output to Opto Triac pin
unsigned char ZEROCROSS = 4;  // Output to Opto Triac pin
unsigned char dimming = 0;    // Dimming level (0-100)
boolean enable = false;       // Dimming not enabled by default. Will need to reach min HR


// The remote service we wish to connect to.
static BLEUUID serviceUUID(BLEUUID((uint16_t)0x180D));
// The characteristic of the remote service we are interested in.
// this is the standard ID of the HEART RATE characteristic
static  BLEUUID    charUUID(BLEUUID((uint16_t)0x2A37));

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static boolean notification = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;

// TypeDef to receive Heart Rate data
typedef struct {
  char ID[20];
  uint16_t HRM;
}HRM;
HRM hrm;


//--------------------------------------------------------------------------------------------
// Setup the Serial Port and output Sketch name and compile date
//--------------------------------------------------------------------------------------------
void startSerial(uint32_t baud) {
  
  // Setup Serial Port aut 115200 Baud
  Serial.begin(baud);
  
  delay(10);
  
  Serial.println();
  Serial.print(sketchName);
  Serial.print(F(" | Compiled: "));
  Serial.print(__DATE__);
  Serial.print(F("/"));
  Serial.println(__TIME__);
} // End of startSerial



static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.print(length);
    Serial.print(" data: ");
    for (int i = 0; i < length; i++) {
      Serial.print(pData[i]);
    }
    Serial.println();

    hrm.HRM = pData[1];
    Serial.print("Heart Rate ");
    Serial.print(hrm.HRM, DEC);
    Serial.println("bpm");
}

//--------------------------------------------------------------------------------------------
//  Connect to BLE HRM
//--------------------------------------------------------------------------------------------
bool connectToServer(BLEAddress pAddress) {
    Serial.print(F("Forming a connection to "));
    Serial.println(pAddress.toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(F(" - Created client"));

    // Connect to the HRM BLE Server.
    // Note that a lot of example snippets I found on the web ommitted the second argument (optionnal) in the following line
    // bug the BT connection was very slow and hang up most of the time
    // once I added "BLE_ADDR_TYPE_RANDOM" the connection was very quick and smooth
    pClient->connect(pAddress,BLE_ADDR_TYPE_RANDOM);
    Serial.println(F(" - Connected to server"));

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print(F("Failed to find our service UUID: "));
      Serial.println(serviceUUID.toString().c_str());
      return false;
    }
    Serial.println(F(" - Found our service"));


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(charUUID.toString().c_str());
      return false;
    }
    Serial.println(F(" - Found our characteristic"));

    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());

    // Register for Notify
    pRemoteCharacteristic->registerForNotify(notifyCallback);
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
      Serial.println("Found our device!"); 
      advertisedDevice.getScan()->stop();

      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true;
    }
  }
};

void zero_crosss_int()  // function to be fired at the zero crossing to dim the light
{
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing : (50Hz)-> 10ms (1/2 Cycle) For 60Hz (1/2 Cycle) => 8.33ms 
  // 10ms=10000us

  enable = true;

  if (hrm.HRM < 90) {
    enable = false;
  }
  if (hrm.HRM >= 90 and hrm.HRM < 100 ) {
    dimming = 60;
  }
  if (hrm.HRM >= 100 and hrm.HRM < 110 ) {
    dimming = 40;
  }
  if (hrm.HRM >= 110 and hrm.HRM < 120 ) {
    dimming = 30;
  }
  if (hrm.HRM >= 120 and hrm.HRM < 130 ) {
    dimming = 20;
  }
  if (hrm.HRM >= 130 and hrm.HRM < 140 ) {
    dimming = 10;
  }
  if (hrm.HRM >= 140) {
    dimming = 0;
  }

  if (enable) {
    int dimtime = (100*dimming);    // For 60Hz =>65    
    delayMicroseconds(dimtime);     // Off cycle
    digitalWrite(PWM, HIGH);        // triac firing
    delayMicroseconds(20);          // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(PWM, LOW);         // triac Off
  }
}

void setup() {
  startSerial(115200);
    
  pinMode(PWM, OUTPUT);// Set AC Load pin as output
  pinMode(ZEROCROSS, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ZEROCROSS), zero_crosss_int, RISING);

  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  // TODO : flash the LED in blue

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device. Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30); 
}

// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect. Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
      // TODO : turn on the LED in blue
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      // TODO : flash the LED in red
    }
    doConnect = false;
  }

  // Turn notification on 
  // to subscribe to heart rate updates
  if (connected) {
    if (notification == false) {
      Serial.println(F("Turning Notifocation On"));
      const uint8_t onPacket[] = {0x1, 0x0};
      pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)onPacket, 2, true);
      notification = true;
    }
    
  }

}
