#include "esp_camera.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "soc/rtc_wdt.h"
#include "esp_task_wdt.h"

#include <WiFi.h>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

//    FRAMESIZE_96X96,    // 96x96
//    FRAMESIZE_QQVGA,    // 160x120
//    FRAMESIZE_QCIF,     // 176x144
//    FRAMESIZE_HQVGA,    // 240x176
//    FRAMESIZE_240X240,  // 240x240
//    FRAMESIZE_QVGA,     // 320x240
//    FRAMESIZE_CIF,      // 400x296
//    FRAMESIZE_HVGA,     // 480x320
//    FRAMESIZE_VGA,      // 640x480
//    FRAMESIZE_SVGA,     // 800x600
//    FRAMESIZE_XGA,      // 1024x768
//    FRAMESIZE_HD,       // 1280x720
//    FRAMESIZE_SXGA,     // 1280x1024
//    FRAMESIZE_UXGA,     // 1600x1200

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPI.h>

WebServer server(88);
#include "camera_pins.h"
#include "wifinode.h"
#include "nodeconfig.h"

WifiNode node;
void startCameraServer();
void readLoop(void * parameter);
extern void connectWifi();
extern void setWifiiNFO(String ssid, String password, String device_name);
extern void Write_String(int a,int b,String str);
extern String getValue(String data, char separator, int index);

bool recvl_ok = false;
String inData="";

void setWifiConfigByPort(String config_str)
{
  String cmd_str = config_str.substring(2);
  String cmd_name = getValue(cmd_str, ':', 0);
  String cmd_value = getValue(cmd_str, ':', 1);
  if(cmd_name=="SSID")
  {
    cf_ssid = cmd_value.substring(0,cmd_value.length()-1);
    Serial.print("SSID");
  }
  else if(cmd_name=="PASS")
  {
    cf_password = cmd_value.substring(0,cmd_value.length()-1);
    Serial.print("PASS");
  }
  else if(cmd_name=="NAME")
  {
    cf_node_name = cmd_value.substring(0,cmd_value.length()-1);
    Serial.print("NAME");
  }
  else if(cmd_name=="SAVE")
  {
    Write_String(1,30,cf_ssid);
    delay(100);
    Write_String(5,60,cf_password);
    delay(100);
    Write_String(9,90,cf_node_name);
    delay(100);
    Serial.print("SAVE");
    connectWifi();
  }
  else if(cmd_name=="IPADDRESS")
  {
 
    if ((WiFi.status() == WL_CONNECTED)) 
    {
      Serial.print("&&"); 
      Serial.print(WiFi.localIP()); 
      Serial.println("##"); 
    }
  }
  else
  {
    Serial.print("ERROR");
  }
  
}
void reportVersion()
{
  String version = "version:"+String(VERSION);
  server.send(200, "text/plain",version);
}
void reportDevice()
{
  String ip = "Sentry:Camera-"+cf_node_name+":"+ WiFi.localIP().toString();
  server.send(200, "text/plain",ip);
}

void getRSSI()
{
  int rssi =  0;
  rssi = WiFi.RSSI();
  String rssi_str = "RSSI:"+String(rssi);
  server.send(200, "text/plain", rssi_str);
}
void returnOK() 
{
  server.send(200, "text/plain", "ok");
}
void returnFail(String msg) 
{
  server.send(500, "text/plain", msg + "\r\n");
}
void sendGcode()
{
   if (!server.hasArg("gc")) 
    {
        return returnFail("No ARGS");
    }
    String op = server.arg("gc")+"\n";
    {
       Serial.print(op);
    }
}


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("start config...");
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  node.init();
  rtc_wdt_set_length_of_reset_signal(RTC_WDT_SYS_RESET_SIG, RTC_WDT_LENGTH_3_2us);
  rtc_wdt_set_stage(RTC_WDT_STAGE0, RTC_WDT_STAGE_ACTION_RESET_SYSTEM);
  rtc_wdt_set_time(RTC_WDT_STAGE0, 2000);
  xTaskCreatePinnedToCore(readLoop, "Task0", 10000, NULL, 2, NULL,  0); 
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 12000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    Serial.printf("PSRAM found！");
  } else {
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.printf("PSRAM not found！");
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_XGA);
  s->set_quality(s, 10);
  s->set_whitebal(s, 1);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  startCameraServer();

  server.on("/find", HTTP_GET, reportDevice);
  server.on("/version", HTTP_GET, reportVersion);
  server.on("/gcode", HTTP_GET, sendGcode);
  server.on("/getrssi",HTTP_GET,getRSSI);
  
  server.begin();
  Serial.println("config finish...");

}

void readPrinterBack()
{
  //读取所有的数据
  while (PRINTER_PORT.available() > 0 && recvl_ok == false)
  {
    char recieved = PRINTER_PORT.read();
    if (recieved == '\n')
    {
      recvl_ok = true;
    }
    inData += recieved; 
  }
   if (recvl_ok == true)
   {
     if(inData.length()>=2)
     {
         if(inData.startsWith("&&"))
        {
          setWifiConfigByPort(inData);
        }
     }
     recvl_ok = false;
     inData="";
   }
}

void readLoop(void * parameter)
{
  // const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
  for(;;)
  {
    readPrinterBack();
    rtc_wdt_feed();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void loop() {
  // put your main code here, to run repeatedly:
//  delay(10000);
  server.handleClient();
  
}
