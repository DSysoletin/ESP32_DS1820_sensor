README

This sketch will help you to create simple temperature sensor from your ESP32.
For now, it supports two DS18B20 sensors on same 1 wire bus (sensors number is hardcoded for now).

HOW TO USE

This sketch contains ESPbase part, so it is very simple to configure. You need only download sketch to ESP32 board, connect 1wire bus with two ESP32 sensors, and apply power. 
For first time, or if it will be not able to connect to your WiFi, it will create WiFi AP for you. You should connect to that AP and enter "192.168.4.1" address in your browser - and you can configure this device without editing code.
After configuration, sketch will connect to your WiFi, then connect to MQTT sensor, then try to get temperatures from sensor, publish them, and go to sleep. Sketch optimized for battery power, so it will do its job and go to sleep.
If it can't connect to WiFi, it will create WiFi AP for configuration, and will go to sleep after timeout (default is 180s). LED will blink while it is in AP mode.
In case if there is error with connecting to MQTT, board will do several retries, and if there is no success - it will go to sleep.

This sketch will use two topics:
ESP32+(MQTT prefix from config)+0
ESP32+(MQTT prefix from config)+1

MQTT prefix can be edited in web configuration  page.

If you have compilation problems - pleace sure that you're applied patch for ESPbase package.

LED status
This sketch will display its own status with onboard LED. Since sketch is optimized for low power, normal behavior - is to do job quickly and sleep. 
While board doing its job, LED will lit solid. When board will come to sleep, LED will be turned off.
In case if there is no WiFi available - board will enter AP mode for 180 seconds by defaul - and LED will blink with 1s period while board is in AP mode.
This timeout is needed to handle situations when all fine with your home WiFi, but for some reason (interference) board wasn't connected at this time. Without timeout, it will enable AP and stay there until battery is drained.

