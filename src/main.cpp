#include <Arduino.h>
#include <LiteLED.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <stdio.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ModbusClientRTU.h>
#include <WiFi.h>
#include <WebServer.h>
#include <RtcDS1302.h>
#include "BYD.h"
#include "PYLON-CAN.h"
#include "datalayer.h"
#include <config.h>

#define LED_TYPE LED_STRIP_WS2812
#define LED_TYPE_IS_RGBW 0
LiteLED myLED( LED_TYPE, LED_TYPE_IS_RGBW );

ThreeWire myWire(DS1302_IO_PIN, DS1302_SCLK_PIN, DS1302_CE_PIN); // IO, SCLK, CE
RtcDS1302 Rtc(myWire);

int64_t core_task_time_us;
TaskHandle_t main_loop_task;

// Interval settings

void init_CAN();
void receive_can_native();
void core_loop(void *task_time_us);
const char *ssid = SSID;
const char *pass = PASSWORD;
float webvolt = 0.0, webamp = 0.0, webah = 0.0, websoc = 0.0;
WebServer server(80); // creating a webserver object
IPAddress ip(IP), ipgw(GATEWAY), ipsn(NETMASK);
String headString = "<head><style>" // head and CSS style elements
                    ".blueBox {"
                    "background-color: blue;"
                    "color: white;"
                    "width: 600px;"
                    "padding: 20px;"
                    "text-align: center;"
                    "font-size: 50px;"
                    "font-family: arial;"
                    "margin: 20px 35px;"
                    "}"
                    "</style>";

String refreshString = "<meta http-equiv=\"refresh\" content=\"10\"></head>";
String trackTempString = "</BR><h1 align=\"center\">Batteriedaten</h1></div>";

CAN_device_t CAN_cfg;             // CAN Config

const int rx_queue_size = 10;     // Receive Queue size

u_int32_t mamps = BATTERY_AH_MAX*3600;
SemaphoreHandle_t TaskMutex;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
ModbusClientRTU mymodbus;
void handleData(ModbusMessage msg, uint32_t token)
{
  int16_t volt, amp;
  static int16_t storecnt = 0;
  static uint16_t max_volt_int = BATTERY_BULK_VOLTAGE;
  float amph, voltf, ampf, proz;
  static bool tog = 0;
  if (token = 1234)
  {
    msg.get(3, volt);
    msg.get(11, amp);
    xSemaphoreTake(TaskMutex, portMAX_DELAY);
    datalayer.battery.status.voltage_dV = volt;
    datalayer.battery.status.current_dA = amp / 10;
    if ((volt > (datalayer.battery.info.max_design_voltage_dV - 1)) && (amp < 0) && (amp > -BATTERY_BULK_FLOAT_SP))
    {
      mamps = BATTERY_AH_MAX * 3600;
    }
    voltf = volt / 10.0;
    ampf = amp < 0 ? amp / 120.0 : amp / 100.0;
    if(mamps > 0) mamps = mamps - amp * 10;
    if(mamps > BATTERY_AH_MAX * 3600) mamps=BATTERY_AH_MAX * 3600;
    amph = mamps / 3600000.0;
    proz = amph / (BATTERY_AH_MAX / 1000.0) * 100.0;
    datalayer.battery.status.Remain_Ah = amph * 10.0;
    if (proz > 98.0)
    {
      datalayer.battery.status.reported_soc = 98 * 100;
    }
    else
    {
      datalayer.battery.status.reported_soc = proz * 100;
    }
    if ((volt > (datalayer.battery.info.max_design_voltage_dV - 10)) && (amp < 0) && (amp > BATTERY_BULK_FLOAT_SP))
    {
      datalayer.battery.status.reported_soc = 100 * 100;
      max_volt_int = BATTERY_FLOAT_VOLTAGE;
    }
    if (volt < BATTERY_FLOAT_BULK_SP)
    {
      max_volt_int = BATTERY_BULK_VOLTAGE;
    }
    if (datalayer.battery.info.max_design_voltage_dV < max_volt_int)
    {
      datalayer.battery.info.max_design_voltage_dV++;
    }
    if (datalayer.battery.info.max_design_voltage_dV > max_volt_int)
    {
      datalayer.battery.info.max_design_voltage_dV--;
    }
    xSemaphoreGive(TaskMutex);
    if(max_volt_int = BATTERY_BULK_VOLTAGE)myLED.setPixel( 0, 0x00ff00, 1 );
    else myLED.setPixel( 0, 0x0000ff, 1 );
    webah = amph;
    webamp = ampf;
    webvolt = voltf;
    websoc = proz;
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print(voltf);
    display.println(" V");
    display.print(ampf);
    display.println(" A");
    display.print(amph);
    display.println(" Ah");
    display.print("SOC ");
    display.print(proz);
    display.print("%");
    display.display();
    if(tog) {
      myLED.brightness( 10 );
      myLED.show();
    }
    else{
      myLED.brightness( 50 );
      myLED.show();
    }
    tog=!tog;
    if (storecnt >= 30)
    {
      storecnt = 0;
      Rtc.SetIsWriteProtected(false);
      Rtc.SetMemory((uint8_t *)&mamps, 4);
    }
    else
      storecnt++;
  }
}

void trackTemperatureScreen()
{
  String message = "";
  message += headString;
  message += refreshString;
  message += trackTempString;
  message += "<div align=\"center\";>";
  message += "</BR><h2>Spannung [V]: ";
  message += String(webvolt, 1);
  message += "</h2></div>";
  message += "<div align=\"center\";>";
  message += "</BR><h2>Strom [A]: ";
  message += String(webamp, 2);
  message += "</h2></div>";
  message += "<div align=\"center\";>";
  message += "</BR><h2>Restladung [Ah]: ";
  message += String(webah, 2);
  message += "</h2></div>";
  message += "<div align=\"center\";>";
  message += "</BR><h2>SOC [%]: ";
  message += String(websoc, 2);
  message += "</h2></div>";

  server.send(200, "text/html", message);
}
void setup()
{
  u_int32_t temp = 0;
  Serial.begin(115200);
  myLED.begin( WS2812_PIN, 1,0 );
  myLED.brightness( 10 );
  myLED.setPixel( 0, 0xff0000, 1 );    
  pinMode(CHARGE_DISCHARGE_LOCK_PIN, INPUT_PULLDOWN);
  Wire.setPins(DISPLAY_SDA_PIN, DISPLAY_SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
  }
  delay(200);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("INIT");
  display.display();

  init_CAN();

  WiFi.config(ip, ipgw, ipsn);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    display.print(".");
    display.display();
  }
  myLED.brightness( 30,1 );
  display.println(".");
  display.println("WLAN AKT");
  display.print(WiFi.localIP());
  display.display();

  pinMode(RS485_EN_PIN, OUTPUT);
  digitalWrite(RS485_EN_PIN, HIGH);
  pinMode(RS485_SE_PIN, OUTPUT);
  digitalWrite(RS485_SE_PIN, HIGH);
  pinMode(PIN_5V_EN, OUTPUT);
  digitalWrite(PIN_5V_EN, HIGH);
  RTUutils::prepareHardwareSerial(Serial2);
  Serial2.begin(9600, SERIAL_8N1,RS485_RX_PIN,RS485_TX_PIN);
  mymodbus.onDataHandler(&handleData);
  mymodbus.begin(Serial2);
  Rtc.Begin();
  Rtc.GetMemory((uint8_t *)&temp, 4);
  if (temp <= BATTERY_AH_MAX*3600) mamps = temp;
  server.on("/bat", trackTemperatureScreen);
  server.begin(80);
  xTaskCreatePinnedToCore((TaskFunction_t)&core_loop, "core_loop", 4096, &core_task_time_us, 4, &main_loop_task, 1);
  TaskMutex=xSemaphoreCreateMutex();
  myLED.brightness( 50 ,1);
}
void loop()
{
  static int reconcnt=0;
  static unsigned long previousMillis = 0; // will store last time a Modbus Message was send
  server.handleClient();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 999)
  {
    Serial.println("modreq");
    previousMillis = currentMillis;
    mymodbus.addRequest(1234, 1, READ_HOLD_REGISTER, 587, 5);
    if (WiFi.status() == WL_CONNECTION_LOST)
    {
      if(reconcnt>30){
        WiFi.reconnect();
        reconcnt=0;
      }
      else reconcnt++;
    }
  }
}

void core_loop(void *task_time_us)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  unsigned long previousMillisUpdateVal = 0;
  while (true)
  {
    // Input, Runs as fast as possible
    receive_can_native();                                           // Receive CAN messages from native CAN port
    if (millis() - previousMillisUpdateVal >= INTERVAL_5_S) // Every 5s normally
    {
      previousMillisUpdateVal = millis(); // Order matters on the update_loop!
      // update_values_battery();             // Fetch battery values
      // update_scaled_values();  // Check if real or calculated SOC% value should be sent
      xSemaphoreTake(TaskMutex, portMAX_DELAY);
      update_values_can_inverter(); // Update values heading towards inverter
      xSemaphoreGive(TaskMutex);
    }
    // Output
    send_can_inverter(); // Send CAN messages to all components
    vTaskDelayUntil(&xLastWakeTime, INTERVAL_10_MS);
  }
}

void init_CAN()
{
  // CAN pins
  pinMode(CAN_SE_PIN, OUTPUT);
  digitalWrite(CAN_SE_PIN, LOW);
  CAN_cfg.speed = CAN_SPEED_1000KBPS; //Because of an error in CAN-Lib configured speed must be double the real speed
  CAN_cfg.tx_pin_id = (gpio_num_t) CAN_TX_PIN;
  CAN_cfg.rx_pin_id = (gpio_num_t) CAN_RX_PIN;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();
}
void receive_can_native()
{ // This section checks if we have a complete CAN message incoming on native CAN port
  CAN_frame_t rx_frame_native;
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame_native, 0) == pdTRUE)
  {
    CAN_frame rx_frame;
    rx_frame.ID = rx_frame_native.MsgID;
    if (rx_frame_native.FIR.B.FF == CAN_frame_std)
    {
      rx_frame.ext_ID = false;
    }
    else
    { // CAN_frame_ext == 1
      rx_frame.ext_ID = true;
    }
    rx_frame.DLC = rx_frame_native.FIR.B.DLC;
    for (uint8_t i = 0; i < rx_frame.DLC && i < 8; i++)
    {
      rx_frame.data.u8[i] = rx_frame_native.data.u8[i];
    }
    // message incoming, pass it on to the handler
    receive_can_inverter(rx_frame);
  }
}
void transmit_can(CAN_frame *tx_frame, int interface)
{
  CAN_frame_t frame;
  frame.MsgID = tx_frame->ID;
  frame.FIR.B.FF = tx_frame->ext_ID ? CAN_frame_ext : CAN_frame_std;
  frame.FIR.B.DLC = tx_frame->DLC;
  frame.FIR.B.RTR = CAN_no_RTR;
  for (uint8_t i = 0; i < tx_frame->DLC; i++)
  {
    frame.data.u8[i] = tx_frame->data.u8[i];
  }
  // Serial.println("vor-can-write");
  ESP32Can.CANWriteFrame(&frame);
  // Serial.println("nach-can-write");
}