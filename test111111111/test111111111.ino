#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#include <Blinker.h>

/* ===== Blinker 配置 ===== */
char auth[] = "5caa4c9bdcb2";
char ssid[] = "wifi";
char pswd[] = "1234567890";

/* ===== Blinker 组件 ===== */
BlinkerButton Button1("btn-abc");
BlinkerNumber NumFan("num-fan");
BlinkerNumber NumTemp("num-temp");
BlinkerNumber NumHumi("num-humi");
BlinkerNumber NumPM25("num-pm25");
BlinkerNumber NumPM10("num-pm10");
BlinkerNumber NumVOC("num-voc");
BlinkerNumber NumJQ("num-jq");
BlinkerNumber NumScore("num-score");

/* ===== STM32 串口 ===== */
#define STM32_SERIAL Serial1
#define STM32_RX_PIN 16
#define STM32_TX_PIN 17

/* ===== 风扇状态 ===== */
static uint8_t fanState = 0;  // 0=OFF, 1=ON

/* ===== 空气数据结构 ===== */
#pragma pack(1)
typedef struct
{
  uint16_t pm25;
  uint16_t pm10;
  uint16_t voc;
  uint16_t jq;
  int16_t temp_x10;
  uint16_t humi_x10;
  uint8_t score;
} AIR_DATA_T;
#pragma pack()

/* =========================================================
 * STM32 ← ESP32：发送风扇控制帧
 * 33 xx 0D 0A
 * ========================================================= */
void sendFanCmdToSTM32(uint8_t on) {
  uint8_t buf[4] = {
    0x33,
    on ? 0x01 : 0x00,
    0x0D,
    0x0A
  };

  STM32_SERIAL.write(buf, 4);
}

/* =========================================================
 * Blinker 按钮回调（★关键）
 * ========================================================= */
void button1_callback(const String &state) {
  BLINKER_LOG("APP Fan Button:", state);

  if (state == BLINKER_CMD_ON) {
    fanState = 1;

    Button1.print("on");   // ★ 立刻更新 APP
    NumFan.print(1);

    sendFanCmdToSTM32(1);
  }
  else if (state == BLINKER_CMD_OFF) {
    fanState = 0;

    Button1.print("off");  // ★ 立刻更新 APP
    NumFan.print(0);

    sendFanCmdToSTM32(0);
  }
}



/* =========================================================
 * STM32 → ESP32：风扇状态解析
 * ========================================================= */
void parseFanFrame(uint8_t state) {
  if (state == 0x01) {
    fanState = 1;              // ★ 必须同步真实状态
    Button1.print("on");
    NumFan.print(1);
  } else {
    fanState = 0;              // ★ 必须同步真实状态
    Button1.print("off");
    NumFan.print(0);
  }
}

/* =========================================================
 * STM32 → ESP32：空气数据解析
 * ========================================================= */
void parseAirFrame(uint8_t *data) {
  AIR_DATA_T air;
  memcpy(&air, data, sizeof(AIR_DATA_T));

  NumPM25.print(air.pm25);
  NumPM10.print(air.pm10);
  NumVOC.print(air.voc);
  NumJQ.print(air.jq);
  NumTemp.print(air.temp_x10 / 10.0);
  NumHumi.print(air.humi_x10 / 10.0);
  NumScore.print(air.score);
}

/* =========================================================
 * 串口状态机
 * ========================================================= */
void parseSTM32Serial() {
  static uint8_t state = 0;
  static uint8_t len = 0;
  static uint8_t buf[32];
  static uint8_t idx = 0;

  while (STM32_SERIAL.available()) {
    uint8_t c = STM32_SERIAL.read();

    switch (state) {
      case 0:
        if (c == 0x33) state = 10;
        else if (c == 0xAA) state = 1;
        break;

        /* ===== 风扇帧：33 xx 0D 0A ===== */
      case 10:       // 收到 0x33
        buf[0] = 0;  // 清空
        state = 11;
        break;

      case 11:       // 收 状态字节
        buf[0] = c;  // 0x00 / 0x01
        state = 12;
        break;

      case 12:  // 等 0x0D
        if (c == 0x0D) state = 13;
        else state = 0;
        break;

      case 13:  // 等 0x0A
        if (c == 0x0A) {
          parseFanFrame(buf[0]);
        }
        state = 0;
        break;


      /* ===== 空气帧 ===== */
      case 1:
        if (c == 0x02) state = 2;
        else state = 0;
        break;
      case 2:
        len = c;
        idx = 0;
        state = 3;
        break;
      case 3:
        buf[idx++] = c;
        if (idx >= len) state = 4;
        break;
      case 4:
        if (c == 0x00) state = 5;
        else state = 0;
        break;
      case 5:
        if (c == 0x0D) state = 6;
        else state = 0;
        break;
      case 6:
        if (c == 0x0A) parseAirFrame(buf);
        state = 0;
        break;
    }
  }
}


/* =========================================================
 * Blinker 心跳：同步 APP 按钮状态（★缺的就是这个）
 * ========================================================= */
void heartbeat() {
  BLINKER_LOG("heartbeat, fanState:", fanState);

  if (fanState) {
    Button1.print("on");
  } else {
    Button1.print("off");
  }
}

/* ========================================================= */

void setup() {
  Serial.begin(115200);
  STM32_SERIAL.begin(115200, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Blinker.begin(auth, ssid, pswd);

  Button1.attach(button1_callback);  // ★ 必须有
  Blinker.attachHeartbeat(heartbeat);

  BLINKER_LOG("ESP32-S3 Air + Fan Ready");
}

void loop() {
  Blinker.run();
  parseSTM32Serial();
}
