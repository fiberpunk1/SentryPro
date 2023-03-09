#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "esp_task_wdt.h"

#include "wifinode.h"
#include "nodeconfig.h"
#include "rBase64.h"

//wifi重连
unsigned long previousMillis = 0;
unsigned long interval = 30000;
void setWifiiNFO(String ssid, String password, String device_name);
void rgbLED(uint8_t r);
void connectWifi();
void Write_String(int a,int b,String str);
String getValue(String data, char separator, int index);

//RGB灯，低电平亮，高电平灭
void rgbLED(uint8_t r,uint8_t g, uint8_t b)
{
  digitalWrite(RED_LED, r);
  digitalWrite(GREEN_LED, g);
  digitalWrite(BLUE_LED, b);
}
String readConfig(File& file)
{
  String ret="";
  if(file.available())
  {
    ret = file.readStringUntil('\n');
    ret.replace("\r", "");
  }
  
  return ret;
}
//a写入字符串长度，b是起始位，str为要保存的字符串
void Write_String(int a,int b,String str){
    EEPROM.write(a, str.length());//EEPROM第a位，写入str字符串的长度
    //把str所有数据逐个保存在EEPROM
    for (int i = 0; i < str.length(); i++){
        EEPROM.write(b + i, str[i]);
    }
    EEPROM.commit();
}
//a位是字符串长度，b是起始位
String Read_String(int a, int b){ 
    String data = "";
    //从EEPROM中逐个取出每一位的值，并链接
    for (int i = 0; i < a; i++){
        data += char(EEPROM.read(b + i));
    }
    return data;
}
void connectWifi()
{
   cf_ssid = Read_String(EEPROM.read(1),30);
   cf_password = Read_String(EEPROM.read(5),60);
   cf_node_name = Read_String(EEPROM.read(9),90);
    WiFi.mode(WIFI_STA);

    WiFi.persistent(false); 
    WiFi.setAutoConnect(false);

    WiFi.begin((const char*)cf_ssid.c_str(), (const char*)cf_password.c_str());

    uint8_t i = 0;
    //红灯闪
    while ((WiFi.status() != WL_CONNECTED)&& (i++ < 10)) 
    {
        //totaly wait 5 seconds
        rgbLED(0,1,1);
        delay(500);
        rgbLED(1,1,1);
        delay(500);
    }
    if (i == 11) 
    {
         DBG_OUTPUT_PORT.print("Faile connect to:");
         DBG_OUTPUT_PORT.println(cf_ssid.c_str());
         DBG_OUTPUT_PORT.println(cf_password.c_str());
         rgbLED(0,1,1); 
    }
    else
    {
      DBG_OUTPUT_PORT.print("Connected! IP address: ");
      DBG_OUTPUT_PORT.println(WiFi.localIP()); 
      rgbLED(1,0,1); //绿灯常亮
      delay(1500);
      rgbLED(1,1,1);
    }
 
}
void setWifiiNFO(String ssid, String password, String device_name)
{
    Write_String(1,30,ssid);
    delay(100);
    Write_String(5,60,password);
    delay(100);
    Write_String(9,90,device_name);
    delay(100);
    connectWifi();
}


String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;


    for (int i = 0; i <= maxIndex && found <= index; i++) 
    {
        if (data.charAt(i) == separator || i == maxIndex) 
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

 
WifiNode::WifiNode()
{

}



void WifiNode::init()
{
    //开启双核设置
	uint8_t pw_exist = 0;
  String stop_x_str = "";
  String stop_y_str = "";
  String react_l_str = "";
  String b_time_l_str = "";
	EEPROM.begin(256);
  uint8_t connect_count = 0;

  while(connect_count<3)
  {
    connect_count++;
    cf_ssid = Read_String(EEPROM.read(1),30);
    cf_password = Read_String(EEPROM.read(5),60);
    cf_node_name = Read_String(EEPROM.read(9),90);
    if((cf_password.length()<3)&&(cf_node_name.length()<2))
    {
      break;
    }
    WiFi.mode(WIFI_STA);

    WiFi.persistent(false); 
    WiFi.setAutoConnect(false);

    WiFi.begin((const char*)cf_ssid.c_str(), (const char*)cf_password.c_str());

    uint8_t i = 0;
    while ((WiFi.status() != WL_CONNECTED)&& (i++ < 10)) 
    {
        //totaly wait 5 seconds
        rgbLED(0,1,1);
        delay(250);
        rgbLED(1,1,1);
        delay(250);

    }
    if (i == 11) 
    {
         DBG_OUTPUT_PORT.print("Faile connect to:");
         DBG_OUTPUT_PORT.println(cf_ssid.c_str());
         DBG_OUTPUT_PORT.println(cf_password.c_str()); 
         rgbLED(0,1,1); 
    }
    else
    {
      DBG_OUTPUT_PORT.print("Connected! IP address: ");
      DBG_OUTPUT_PORT.println(WiFi.localIP()); 
      rgbLED(1,0,1); //绿灯常亮
      delay(1500);
      rgbLED(1,1,1);
      break;
    }
  }

}
//定时检测
void WifiNode::checkwifi()
{
    unsigned long currentMillis = millis();
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) 
    {
        WiFi.disconnect();
        WiFi.reconnect();
        previousMillis = currentMillis;
    }
}
void WifiNode::process()
{
   checkwifi();
}
