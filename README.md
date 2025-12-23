# AirGuard-STM32-ESP32-Air-Purifier

# AirGuard

**AirGuard** 是一款 **成品级智能空气净化器系统**，集成空气质量检测、风扇控制、本地屏幕交互以及云端 App 远程控制。

本项目从 **硬件设计到软件实现全部由本人独立完成**，包括：

- 原理图设计  
- PCB 设计与打板  
- 元器件焊接  
- 3D 外壳建模（部分）  
- STM32 / ESP32 嵌入式代码开发  
  ![Image](https://github.com/user-attachments/assets/5f31760d-eaf1-4f8a-bf7d-a0cb9202702d)

   ![Image](https://github.com/user-attachments/assets/f19a8c96-d2ad-45ae-b819-883eadf85577)

   ![Image](https://github.com/user-attachments/assets/f3cf7ea7-bd90-4831-ad09-f17ea2459161)

---

## 项目特点

- STM32 + ESP32 **双 MCU 架构**
- 空气质量实时监测  
  - PM2.5 / PM10  
  - VOC  
  - 温湿度  
- 本地串口屏交互（Nextion）
- Blinker App 云端远程控制与状态同步
- 自定义通信协议与状态同步机制
- 3D 打印外壳，完整成品形态  

> 外壳模板参考并感谢原作者：  
> **tatzhao**  
> https://oshwhub.com/tatzhao/smart-colorful-air-purifier-spark-plan

---

## 实现步骤

### 1. PCB 打板

- 根据 Gerber 文件在 **嘉立创** 打板  
- 板子尺寸：**100mm × 40mm**
- 可使用嘉立创的免费打板券

---

### 2. 元器件采购

- 根据 BOM 表购买元器件  
- **务必逐项核对型号与参数**

---

### 3. 焊接注意事项

⚠️ **非常重要**

- **需要去掉 R3 电阻**  
  否则 STM32 无法正常工作
- **需要去掉 CH340**  
  否则会与串口屏冲突，导致屏幕无法正确发送指令

---

### 4. STM32 固件烧录

#### 串口烧录（FLYMCU）

- 使用 **PA9 / PA10**
- BOOT 配置：
  - BOOT0 = 高电平
  - BOOT1 = 低电平（VCC → BOOT0，GND → BOOT1）
- 烧录完成后：
  - BOOT0 = 低电平
  - BOOT1 = 低电平

#### ST-Link 烧录

- BOOT0 = 低电平
- BOOT1 = 低电平
- 已经引出对应 SWD 引脚
  

---

### 5. ESP32 固件烧录

- 开发环境：**Arduino IDE**
- 目标型号：**ESP32-S3 Dev Module**
- 使用串口烧录：
  - RX ↔ TX
  - TX ↔ RX

#### 下载模式进入方式

1. 按住 **SW1（复位）**
2. 按住 **SW2（IO 拉低）**
3. 松开 SW1
4. 松开 SW2
5. 上传固件
6. 烧录完成后复位

#### 注意事项

- 使用 **Blinker（点灯科技）SDK**
- 通信协议：**MQTT**
- **不建议使用 ESP8266**
- 若 ESP32 内存不足：
  - 在 Arduino IDE 中调整 `Partition Scheme`
  - 建议选择 **App > 1MB**
   ![Image](https://github.com/user-attachments/assets/4f6f3cdc-26cc-45c2-8ebc-af6a43fe3ebd)

---

### 6. 3D 外壳

- 基于原作者提供的 3D 模型
- 根据自身需求进行二次修改与定制
- 3D 打印后组装成完整成品

---

### 7. 显示屏

- 型号：**淘晶驰 T1 系列 4.3 英寸串口触摸屏**
- 可淘宝购买或自行更换其他型号

⚠️ **注意**

- 使用串口屏时 **不要焊接 CH340**
- 否则会干扰 STM32 与屏幕通信

---

### 8. 风扇

- 型号：**利民 TL-B12W EXTREM 12cm**
- 风量充足，噪音较低，实际使用效果良好

---

### 9. 空气传感器模块

- 使用 **康福尔集成传感器模块**
- 通过串口接收协议帧并解析数据
- 相比自研传感器方案，成本更低、实现更简单

---

## ⚠️ 重要声明

- **本项目无任何防呆保护**
- 接线务必确认正确  
- 错误接线可能导致器件损坏或财产损失  

❗ **本项目仅用于学习与开源交流，严禁商用**

---

## License

本项目遵循开源协议，仅供学习研究使用。
