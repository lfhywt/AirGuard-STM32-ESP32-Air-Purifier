#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#include <Blinker.h>

/* ================= Blinker 配置 ================= */
char auth[] = "5caa4c9bdcb2";
char ssid[] = "wifi";
char pswd[] = "1234567890";

/* ================= Blinker 组件 ================= */
BlinkerButton ButtonFan("btn-abc");
BlinkerNumber NumFan("num-fan");
BlinkerNumber NumTemp("num-temp");
BlinkerNumber NumHumi("num-humi");
BlinkerNumber NumPM25("num-pm25");
BlinkerNumber NumPM10("num-pm10");
BlinkerNumber NumVOC("num-voc");
BlinkerNumber NumJQ("num-jq");
BlinkerNumber NumScore("num-score");

/* ================= STM32 串口 ================= */
#define STM32_SERIAL Serial1
#define STM32_RX_PIN 16
#define STM32_TX_PIN 17

/* ================= 风扇状态 ================= */
// STM32 权威状态
static uint8_t fanState = 0;      // 0=OFF, 1=ON

// APP 操作中的 pending 状态
static bool    fanWaitingAck = false;
static uint8_t fanPendingState = 0;
static unsigned long fanAckTimer = 0;
static uint8_t fanRetryCnt = 0;

#define FAN_ACK_TIMEOUT_MS 2000
#define FAN_RETRY_MAX      2

/* ================= 空气数据结构 ================= */
#pragma pack(1)
typedef struct {
  uint16_t pm25;
  uint16_t pm10;
  uint16_t voc;
  uint16_t jq;
  int16_t  temp_x10;
  uint16_t humi_x10;
  uint8_t  score;
} AIR_DATA_T;
#pragma pack()

/* =================================================
 * ESP32 → STM32：发送风扇命令
 * ================================================= */
void sendFanCmd(uint8_t on)
{
  uint8_t buf[4] = {0x33, on ? 0x01 : 0x00, 0x0D, 0x0A};
  STM32_SERIAL.write(buf, 4);
}

/* =================================================
 * Blinker 按钮回调（APP）
 * ================================================= */
void fanBtnCallback(const String &state)
{
  BLINKER_LOG("APP click:", state);

  if (state == BLINKER_CMD_ON) {
    fanPendingState = 1;
  } else if (state == BLINKER_CMD_OFF) {
    fanPendingState = 0;
  } else {
    return;
  }

  fanWaitingAck = true;
  fanRetryCnt = 0;
  fanAckTimer = millis();

  // 立刻显示 pending（乐观 UI）
  ButtonFan.print(fanPendingState ? "on" : "off");
  NumFan.print(fanPendingState);

  sendFanCmd(fanPendingState);
}

/* =================================================
 * STM32 → ESP32：风扇状态帧解析（权威）
 * ================================================= */
void onFanStateFromSTM32(uint8_t state)
{
  fanState = state ? 1 : 0;

  fanWaitingAck = false;
  fanRetryCnt = 0;

  // ★ 权威状态刷新 UI
  ButtonFan.print(fanState ? "on" : "off");
  NumFan.print(fanState);

  BLINKER_LOG("STM32 fan state:", fanState);
}

/* =================================================
 * STM32 → ESP32：空气数据解析
 * ================================================= */
void onAirData(uint8_t *buf)
{
  AIR_DATA_T air;
  memcpy(&air, buf, sizeof(AIR_DATA_T));

  NumPM25.print(air.pm25);
  NumPM10.print(air.pm10);
  NumVOC.print(air.voc);
  NumJQ.print(air.jq);
  NumTemp.print(air.temp_x10 / 10.0);
  NumHumi.print(air.humi_x10 / 10.0);
  NumScore.print(air.score);
}

/* =================================================
 * STM32 串口协议解析
 * ================================================= */
void parseSTM32Serial()
{
  static uint8_t step = 0;
  static uint8_t len = 0;
  static uint8_t buf[64];
  static uint8_t idx = 0;

  while (STM32_SERIAL.available()) {
    uint8_t c = STM32_SERIAL.read();

    switch (step) {
      case 0:
        if (c == 0x33) step = 1;
        else if (c == 0xAA) step = 10;
        break;

      // ===== 风扇帧 33 xx 0D 0A =====
      case 1:
        buf[0] = c;
        step = 2;
        break;
      case 2:
        step = (c == 0x0D) ? 3 : 0;
        break;
      case 3:
        if (c == 0x0A) onFanStateFromSTM32(buf[0]);
        step = 0;
        break;

      // ===== 空气帧 =====
      case 10:
        step = (c == 0x02) ? 11 : 0;
        break;
      case 11:
        len = c;
        idx = 0;
        step = 12;
        break;
      case 12:
        buf[idx++] = c;
        if (idx >= len) step = 13;
        break;
      case 13:
        step = (c == 0x00) ? 14 : 0;
        break;
      case 14:
        step = (c == 0x0D) ? 15 : 0;
        break;
      case 15:
        if (c == 0x0A) onAirData(buf);
        step = 0;
        break;
    }
  }
}

/* =================================================
 * Blinker 心跳（只显示，不做决策）
 * ================================================= */
void heartbeat()
{
  if (fanWaitingAck) {
    ButtonFan.print(fanPendingState ? "on" : "off");
    NumFan.print(fanPendingState);
  } else {
    ButtonFan.print(fanState ? "on" : "off");
    NumFan.print(fanState);
  }
}

/* =================================================
 * setup / loop
 * ================================================= */
void setup()
{
  Serial.begin(115200);
  STM32_SERIAL.begin(115200, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);

  Blinker.begin(auth, ssid, pswd);

  ButtonFan.attach(fanBtnCallback);
  Blinker.attachHeartbeat(heartbeat);

  BLINKER_LOG("ESP32 Fan System Ready");
}

void loop()
{
  Blinker.run();
  parseSTM32Serial();

  // ACK 超时处理
  if (fanWaitingAck) {
    if (millis() - fanAckTimer > FAN_ACK_TIMEOUT_MS) {
      if (fanRetryCnt < FAN_RETRY_MAX) {
        fanRetryCnt++;
        fanAckTimer = millis();
        sendFanCmd(fanPendingState);
        BLINKER_LOG("Resend fan cmd:", fanRetryCnt);
      } else {
        fanWaitingAck = false;
        // 回滚显示权威状态
        ButtonFan.print(fanState ? "on" : "off");
        NumFan.print(fanState);
        BLINKER_LOG("ACK failed, rollback");
      }
    }
  }

  delay(1);
}
