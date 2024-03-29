# Heart-Rate-Smart-Fan

Control your fan speed with your heartbeat. [**Check this out on YouTube!**](https://youtu.be/DLp7hjtuU8w)


![full parts illustration](https://user-images.githubusercontent.com/2805640/98443948-09f1ce00-210f-11eb-8a64-25ad9aa61905.png)

## Use case

I created this while setting up a home trainer for bike training in my garage during the lockdown in 2020. One issue with home trainers, unlike biking outdoors, is that you don't get the benefit of the wind generated by your speed that cools you down. So you sweat a lot, and this creates dehydration. Not cool (pun intended).

The solution is to use a fan. But you don't want to start the fan until you are warmed up. And when you are lazy (and focused on your workout) you don't want to have to get up to reach your fan. It's much _cooler_ to have it automated instead!


## Parts

![pasts illustration](https://user-images.githubusercontent.com/2805640/98443935-eaf33c00-210e-11eb-9ded-5c89fa68d8bf.png)

### SmartFan

- ESP32 [~10€ on amazon](https://www.amazon.fr/gp/product/B074RGW2VQ/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1), [~3€ on AliExpress](https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20201104000602&origin=y&SearchText=esp32)
- cheap used fan, aim for ~100W (mine was ~20€ on [Leboncoin](https://www.leboncoin.fr/recherche/?category=20&text=ventilateur&price=5-100))
- Light dimmer module with PWM control [~12€ on Amazon](https://www.amazon.fr/gp/product/B07FCF1YSY/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1), [~2€ on Aliexpress](https://es.aliexpress.com/item/32802025086.html?spm=a219c.search0104.3.1.55b65ca8lFJTgG&ws_ab_test=searchweb0_0,searchweb201602_2_10065_10068_10547_319_10891_317_10)
- Dual (BT + Ant+) Heart Rate Monitor (HRM)
- wires

### Other equipments of my setup

- Garmin Forerunner 945 watch
- Elite Direto XR home trainer
- Samsung S5e tablet or Apple TV to run Zwift
- OnePlus 6 to run the Zwift companion app

## Wiring

### For the dimmer

4 wires between AC Dimmer and ESP32 :

|AC dimmer | ESP32|
|---|---|
| VCC | 3V3 |
| GND | GND |
| Z-C | Pin 4 |
| PWM | Pin 23 |

### For the LED

- ESP32 pin 16 -> LED's flat edge pin
- GND -> 100Ohm resistance -> LED's rounded edge pin 

Beware : I realized different implementations of the ESP32 don't have the same pin pattern, so you'd better rely on pin readings rather than blindlessly following a wiring schema on the internet. That's why there is no schema here.

⚠️☠️⚠️ **be extra careful while manipulating ⚡ 220v wires ⚡ : make sure the wire is unplug first !** ⚠️☠️⚠️

For the 220V part, I just cut a power extension cord in 2, and wired the "incoming" part to "AC-IN", and the "outgoing" part to "LOAD". I didn't find any indication regarding which wire (phase/neutrol) should go where... So either I got lucky, either it doesn't matter... If you are reading this and you have the answer, please [help me](https://github.com/edelans/Heart-Rate-Smart-Fan/issues) !

⚠️☠️⚠️ **be extra careful while manipulating ⚡ 220v wires ⚡ : make sure the wire is unplug first !** ⚠️☠️⚠️

## Why ESP32

- It's cheap
- It's small
- It's popular

## Why driving the fan with heart rate

### Speed-based smartFan

- ❌ You **don't** get wind when you need it the most. Good idea to mimic the outdoor experience, but we can do better: when you realize you are @170bpm attempting Alpe d'Huez ascension at 12km/h and you don't get a lot of fan wind.

### Power-based smartFan

- 👍 You get wind when you need it the most
- ❌ Technical obstable : your home trainer can probably connect to 1 device in BT and 1 device in ANT+. ESP32 doesn't support ANT+ but supports bluetooth natively. Pairing your home trainer with zwift in ANT+ can only be done if you have an ANT+ compatible device running zwift (which is not the case of the Apple TV, unless their should be a workaroung with the Zwift companion app running on your phone if it supports ANT+).

### Heartrate-based smartFan

- 👍 You get wind when you need it the most
- 👍 Easy to find an HRM compatible with ANT+ and Bluetooth at the same time. Be careful as if you pair your heart rate monitor to the esp32 with bluetooth, you probably won't be able to pair it to something else with BT (unless you have some very recent heart rate monitor like the Wahoo tickr which can pair to up to 3 devices simultaneously).

➡️ Seems to be the most versatile setup!

## Who connects to what

### Tablet setup

- Home trainer connects to tablet via ANT+
- HRM connects to the smartFan via BT
- HRM connects to garmin forerunner watch via ANT+, which in turn connects to tablet via BT (I could skip the garmin watch intermediary byt connecting the HRM to the tablet via BT, but I want to keep my records clean in garmin connect. Maybe zwift integration could be enough ?)

### Apple TV setup

You only have 2 bouetooth connections available to the Apple TV, the workaround is to use the Zwift companion app on your spartphone to proxy all connected devices through a single BT connection to the Apple TV.

- Home trainer connects to smartphone (Zwift companion app) via BT
- HRM connects to smartFan via BT
- HRM connects to garmin watch with ANT+, which in turn connects to smartphone (Zwift companion app) via BT
- smartphone (Zwift companion app) connects to Apple TV via BT

That's a lot of conenctions... I can simplify by removing the garmin watch and connecting the HRM directly to the phone/tablet via ANT+.

## Inspiration

Thanks a lot to @jmlopezdona and [his project](https://github.com/jmlopezdona/smartfan-esp32) for a speed controlled smart fan for paving the way for the present project (Heart rate controlled).


## Improvements

- 👍 add a LED for bluetooth connection status, with [jled lib](https://github.com/jandelgado/jled)
- [power the esp32 with Mains Voltage using Hi-Link HLK-PM03 Converter](https://randomnerdtutorials.com/esp8266-hi-link-hlk-pm03/) instead of having a second cable (micro-USB)
- add a switch to turn on/off/bypass PWM ?
- work on form factor : put all this on a single [PCB board](https://www.amazon.fr/Ensemble-cartes-circuit-imprim%C3%A9-Luwanz/dp/B075ZN455T/ref=sr_1_7?__mk_fr_FR=%C3%85M%C3%85%C5%BD%C3%95%C3%91&crid=27SBXQ0O37795&dchild=1&keywords=pcb+board&qid=1604477790&sprefix=pcb+b%2Caps%2C162&sr=8-7)
- put it in a [project box enclosure](https://makeradvisor.com/tools/project-box-enclosure/)
- put it in a custom 3D printed casing
- go back in scanning mode when BT connexion is lost. Currently, if you move too far away from the ESP32 and BT connexion is lost, the state is frozen, even when you come back into the BT range : the heart rate is not updated anymore. It would be nicer if the ESP32 could just go back into BT scanning mode when no heart rate notification is received for more than 10s.



# Epilogue

In the end,  I replaced this with a smartplug and an integration with Google Assistant. When I want to start the fan, I can say "Hey Google, turn on the fan". It's dumber (just 1 fan speed, no automation) but much simpler (form factor, less cables involved), less brittle, and works good enough  ¯\\_(ツ)_/¯.

I don't regret building this though, as it was super fun to hack, and I learnt a lot.
