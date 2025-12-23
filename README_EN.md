中文版: [README.md](README.md)

# AirGuard-STM32-ESP32-Air-Purifier

## AirGuard

**AirGuard** is a production-grade intelligent air purifier system that integrates air-quality sensing, fan control, a local serial touchscreen interface, and cloud/mobile app remote control.

This project was independently developed end-to-end by the author and includes:

- Schematics and circuit design  
- PCB design and fabrication  
- Component soldering and assembly  
- 3D enclosure modeling (partially customized)  
- Embedded firmware for STM32 and ESP32  

![Image](https://github.com/user-attachments/assets/5f31760d-eaf1-4f8a-bf7d-a0cb9202702d)  
![Image](https://github.com/user-attachments/assets/f19a8c96-d2ad-45ae-b819-883eadf85577)  
![Image](https://github.com/user-attachments/assets/f3cf7ea7-bd90-4831-ad09-f17ea2459161)

---

## Key Features

- Dual-MCU architecture: **STM32 + ESP32**
- Real-time air-quality monitoring:
  - PM2.5 / PM10
  - VOC
  - Temperature & humidity
- Local serial touchscreen control (Nextion)
- Remote control and state synchronization via the Blinker mobile/cloud app
- Custom communication protocol and state synchronization logic
- 3D printed enclosure tuned for a finished, product-like appearance

> Enclosure model credit: **tatzhao** — https://oshwhub.com/tatzhao/smart-colorful-air-purifier-spark-plan

---

## Implementation / Build Steps

### 1. PCB fabrication

- Generate Gerber files and order boards (recommended: JLCPCB / 嘉立创).  
- Target PCB size: **100 mm × 40 mm**.  
- JLCPCB coupons or promotions can be used where applicable.

---

### 2. Component procurement

- Purchase parts according to the BOM.  
- **Carefully verify part numbers and electrical specifications** before ordering.

---

### 3. Soldering & assembly notes

**Important:**

- **Remove R3** on the board — leaving it in place prevents the STM32 from booting correctly.  
- **Do not populate the CH340 USB-to-UART chip** on the board if you plan to use the serial touchscreen — the CH340 can interfere with the screen’s serial communication.

---

### 4. STM32 firmware programming

#### UART (FLYMCU) flashing

- Use UART pins **PA9 (TX) / PA10 (RX)** for serial downloads.  
- Boot pin configuration to enter system bootloader:
  - `BOOT0 = HIGH`, `BOOT1 = LOW` (VCC → BOOT0, GND → BOOT1)
- After flashing, return pins to normal operation:
  - `BOOT0 = LOW`, `BOOT1 = LOW`

#### ST-Link / SWD flashing

- With ST-Link, ensure `BOOT0 = LOW`, `BOOT1 = LOW`.  
- SWD pins are exposed for programming/debugging.

---

### 5. ESP32 firmware programming

- Development environment: **Arduino IDE**  
- Target board: **ESP32-S3 Dev Module**  
- Serial connections: `RX ↔ TX`, `TX ↔ RX`

#### Entering the download mode (procedure)

1. Hold **SW1** (RESET) down.  
2. Hold **SW2** (IO pulled low) down.  
3. Release **SW1**.  
4. Release **SW2**.  
5. Upload firmware from the IDE.  
6. Reset the board after upload completes.

#### Notes / recommendations

- Firmware stack: **Blinker SDK** (uses **MQTT** for cloud communication).  
- **ESP8266 is not recommended** for this design.  
- If you encounter memory issues on ESP32, change the **Partition Scheme** in Arduino IDE and choose an application partition larger than 1 MB (e.g., app > 1 MB).  
  ![Image](https://github.com/user-attachments/assets/4f6f3cdc-26cc-45c2-8ebc-af6a43fe3ebd)

---

### 6. 3D enclosure

- Base the enclosure on the referenced model and modify it to suit your parts and finish.  
- 3D print the enclosure and assemble the complete unit.

---

### 7. Display

- Chosen display: **Taojingchi (淘晶驰) T1 series, 4.3" serial touchscreen** (Nextion-style).  
- Available from common online marketplaces or substitute with a compatible serial touchscreen.

**Important:** Do not populate the CH340 USB-to-UART chip if using the serial screen — the CH340 may disrupt the STM32 ↔ screen serial bus.

---

### 8. Fan

- Example fan used: **Thermalright / Limin TL-B12W EXTREM 120 mm**.  
- Provides satisfactory airflow with low noise in this build.

---

### 9. Air sensor module

- Used a ready-made integrated sensor module from **Kangfuo (康福尔)** for PM/VOC/temperature/humidity.  
- The module sends protocol frames via serial — the project parses those frames directly.  
- Using an integrated module reduces design complexity and BOM cost compared with designing a custom sensor board.

---

## Important Safety & Usage Notice

- **This project does not include fail-safe protections.**  
- Double-check wiring and power connections before applying power.  
- Incorrect wiring may damage components or cause property loss.  
- This project is provided for learning and open-source exchange only — **not for commercial use**.

---

## License

This repository is released under an open-source license for learning and research purposes only. Refer to the `LICENSE` file in the repository for details.
