
# IoT-Irrigation-Data-Collection-System

This repository contains the Arduino firmware for an IoT-based data collection system, utilizing the **ESP32 microcontroller**, to measure:

- **Soil Moisture**
- **Soil Temperature**
- **Relative Humidity**
- **Air Temperature**
- **Rain Probability**

The system is designed to collect real-time environmental data to enable smart irrigation and agricultural monitoring.

## Features
- **Local Data Storage**: Collected data is stored locally on an SD card for redundancy.
- **Cloud Integration**: Data is transmitted to the ThingSpeak IoT platform for remote access and analysis.
- **Multi-Sensor Support**:
  - Soil moisture and temperature data are collected using HW-390 and DS18B20 sensors, respectively.
  - Relative humidity and air temperature are collected using the AM2302 sensor.
  - Rain probability data is retrieved via the OpenWeatherMap API.
- **Low-Power Mode**: The system leverages ESP32 deep sleep mode to optimize energy consumption.

## Requirements
### Hardware
- **ESP32 Microcontroller** (Programmed using Arduino IDE)
- **HW-390 Soil Moisture Sensor**
- **DS18B20 Temperature Sensor**
- **AM2302 (DHT22) Humidity and Temperature Sensor**
- **RTC Module (e.g., DS3231)**
- **SD Card Module**
- **Power Supply**

### Software
- **Arduino IDE**
- Required Libraries:
  - `OneWire.h` and `DallasTemperature.h` for DS18B20
  - `DHT.h` for AM2302
  - `RTClib.h` for RTC module
  - `SD.h` for SD card operations
  - `WiFi.h` and `ThingSpeak.h` for cloud connectivity

## Getting Started
1. Clone this repository:
   ```bash
   git clone https://github.com/Lane-Romuald/IoT-irrigation-data-collection-system.git
   ```

2. Open the project in Arduino IDE.

3. Install the required libraries using the Arduino Library Manager.

4. Configure your WiFi credentials and ThingSpeak API keys in the source code:
   ```cpp
   const char* ssid = "your_wifi_ssid";
   const char* password = "your_wifi_password";
   unsigned long myChannelNumber = your_channel_number;
   const char* myWriteAPIKey = "your_api_key";
   ```

5. Upload the firmware to your **ESP32 microcontroller**.

6. Connect the sensors as per the pin configuration specified in the code.


## Contribution
Contributions are welcome! Feel free to fork the repository and submit pull requests for enhancements or bug fixes.

## License
This project is licensed under the MIT License. See the `LICENSE` file for more details.

## Author
Developed by **Tamekeng Lane Romuald**  
Contact: [laneromuald@gmail.com](mailto:laneromuald@gmail.com)

--- 
