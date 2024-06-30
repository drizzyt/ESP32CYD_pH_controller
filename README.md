# ESP32CYD_pH_controller
This repo for code related to building a touchscreen interface for a pH controller, using the 2.8" inch ESP32 Display ESP32-2432S028R ILI9341 Resistive Touch Screen 240x320 TFT LCD Module ESP-WROOM-32. The controller will also add the ability to add a pH buffer solution, upon selection. 

Also, before this repo was created, i was creating my own graphics lib, then i realized how far LVGL has come to maturity. So the code has it outlined, that any version greater than 0.3.1, is running LVGL. 


Once i get a production working version in the Arduino Environment, would like to work on the ESP-IDF framework

  Testing Framework: Arduino

MAIN PAGE
  ^Home button
  ^pH button
  ^Settings button
    -Icon to display wifi status
    -Three buttons across the top screen: Home Icon, pH, Settings
    -Temperature data displayed on left side
    -pH data displayed on right side

  

Settings Page
      ^Home Button
      ^Wifi button
        -Shows Firmware Version, Wifi SSID(if connected), MAC Address, LVGL Version

      Wifi Page
        ^Home button
          -two column table: SSID, Signal Strength(dBm)(doesnt discern between any of the networks, and your connected networks)
