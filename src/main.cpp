#include <main.h>

#define DEBUG_MODE

COMM cm;
WiFiClient client;
String redirectURL="/dashboard.html";
//ESP8266WebServer server(80);

byte ByteArrayTx[127]={0};
byte ByteArrayRx[127]={0};
long start_time=0;
byte resetCount=0;
byte wifiMod=STA_MOD;
boolean flagClientConnected = 0;
byte byteFN = MB_FC_NONE;
bool end_of_work_trigger=false;

bool update=false;
AsyncWebServer server(80);

void setup(){
  //ESP.wdtEnable(1000);
  Serial.begin(115200); 
  delay(1000); 
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);

  ////
    // cm.device_register[wifi_ok][0]=0;
    // cm.st_update(wifi_ok);

    // while(!cm.wifi_begin_check())
    // delay(1500);
    // while(!cm.wifi_config_check())
    // delay(1000);
    // while(!cm.device_config_check())
    // delay(500);
    

    // IPAddress STA_IP(cm.wifi_config[sta_ip]);
    // IPAddress STA_DNS(cm.wifi_config[sta_dns]);
    // IPAddress STA_GATEWAY(cm.wifi_config[sta_gateway]);
    // IPAddress SUBNET(cm.wifi_config[subnet]);
  

 
    // WiFi.config(STA_IP,STA_DNS,STA_GATEWAY,SUBNET);  
    // WiFi.begin((const char*)cm.wifi_begin[sta_ssid],(const char*)cm.wifi_begin[sta_password]);
  ///

  WiFi.disconnect();
    delay(1000) ;
    WiFi.mode(WIFI_STA);
   // WiFi.config(IPAddress(10,7,11,100),IPAddress(10,7,11,250),IPAddress(255,255,255,0));
    WiFi.config(IPAddress(192,168,1,150),IPAddress(192,168,1,1),IPAddress(255,255,255,0));
    WiFi.begin("REALTEKNO","20192019");
    delay(1000) ;  
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);  
    // Wait for connection
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 6)
    { //wait 10 seconds
      delay(500);
    }
    if (i == 7)
      {
        #ifdef DEBUG_MODE
        Serial.print("Could not connect to");  
        #endif    
        ESP.restart();
      }
      else
      {
        #ifdef DEBUG_MODE
        Serial.print("Connected! IP address: ");
        Serial.println(WiFi.localIP());
       #endif
      
      }
 
  //aktif edilecek 
     server.on("/",handlePage);
     server.on("/public.html", handlePage);
     server.on("/wifi_settings.html", handlePage);
     server.on("/device_settings.html", handlePage);
     server.on("/data", handleData);     
     server.onNotFound(handleNotFound);
     server.begin(); 
    // //here the list of headers to be recorded
    // const char * headerkeys[] = {"User-Agent", "Cookie"} ;
    // size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    // //ask server to track these headers
    // server.collectHeaders(headerkeys, headerkeyssize);
    // server.begin(); 


    // cm.device_register[ap_sta_mod][0]=1;
    // cm.st_update(ap_sta_mod);
   // cm.device_register[wifi_ok][0]=1;
    //cm.st_update(wifi_ok);
  //aktif edilecek


update=true;
}

void loop(){

  
  //server.handleClient();
  if(receive_update()){

      update=true;
         
  }
  byteFN= MB_FC_WRITE_REGISTER;
  // if(WiFi.status() != WL_CONNECTED){    
  //   cm.device_register[wifi_ok][0]=0;
  //   cm.st_update(wifi_ok);    
  // }
  if (client&&client.connected()) 
  {   
    if(update)
    {
      if(end_of_work_trigger){      

        end_of_work_trigger=false;      
        mb_write_holding_register(end_of_work,1);               
        checkLoop(end_of_work,20);

      }
      mb_write_holding_register(cwidth_val,4);     
      checkLoop(cwidth_val,20);
    }
    else if((millis()-start_time)>3000)
    {
        mb_read_holding_register(end_of_work,1);        
        checkLoop(end_of_work,20);
        start_time =millis();
    }
    delay(10);
  }
  else
  {
   // Serial.println("client not connect");
     client.connect(IPAddress(10,7,11,190),502);
      delay(5000);
  }
    
}


String dataTypeGet(String path)
{
  String dataType = "text/plain";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";
  else if (path.endsWith(".csv"))
    dataType = "text/csv";
  else if (path.endsWith(".gz"))
    dataType = "application/x-gzip";
  return dataType;
}
bool loadFromSdCard(AsyncWebServerRequest *request, String path)
{

  AsyncWebServerResponse *response;
  if (path.endsWith("/"))
    path += "dashboard.html";
  String dataType = dataTypeGet(path);

  String pathWithGz = path + ".gz";
  if ((SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)))
  {
    if (SPIFFS.exists(pathWithGz))
    {
      path = pathWithGz;
      dataType = "application/x-gzip";
    }
  }

  if (!SPIFFS.exists(path.c_str()))
    return false;
   if (dataType == "text/html")
     response = request->beginResponse(SPIFFS, path, dataType, false, processorRead);
  else
  response = request->beginResponse(SPIFFS, path, dataType, false);
  request->send(response);

  return true;
}

void handleNotFound(AsyncWebServerRequest *request)
{

  if (loadFromSdCard(request, request->url()))
  {
    return;
  }
  String message = "\nNo Handler\r\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nParameters: ";
  message += request->params();
  message += "\n";
  for (uint8_t i = 0; i < request->params(); i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    message += String(p->name().c_str()) + " : " + String(p->value().c_str()) + "\r\n";
  }
  request->send(404, "text/plain", message);
  Serial.print(message);
}
void handlePage(AsyncWebServerRequest *request)
{

  
  if (request->authenticate("admin", "realtekno", "Measuring Width"))
  {
    if (request->method() == HTTP_POST)
    {
      for (uint8_t i = 0; i < request->params(); i++)
      {
        AsyncWebParameter *p = request->getParam(i);
        processorWrite(p->name(), p->value());
      }
    }

    loadFromSdCard(request, request->url());
    return;
  }
  else
  {
    request->requestAuthentication("Measuring Width", true, redirectURL);
    ///AsyncWebServerResponse * r = beginResponse(401,"text/html","<script> window.location.href='"+redirectUrl+"'</script>");//  requestAuthentication overload
    return;
  }
}
void handleData(AsyncWebServerRequest *request){
  JSONVar myObject; 
//  cm.device_register[updt][0]=1;
//  cm.st_update(updt);
//****st_update in success ini seçenekli yapıp dene
//  while(!cm.device_register_check())
//  delay(100);
  myObject["cwidth_val"]=word(cm.device_register[cwidth_val][1],cm.device_register[cwidth_val][0]);
  myObject["mwidth_val"]=word(cm.device_register[mwidth_val][1],cm.device_register[mwidth_val][0]);
  myObject["number_of_samples"]=word(cm.device_register[number_of_samples][1],cm.device_register[number_of_samples][0]);
  myObject["fabric_ref"]=word(cm.device_register[fabric_ref][1],cm.device_register[fabric_ref][0]);
  String jsonString = JSON.stringify(myObject);
  
  request->send(200, "text/plane",jsonString);

}
void processorWrite(String name, String value)
{
  
  // short address = name.toInt();
  // Serial.print(address);
  // Serial.print("--");
  // Serial.println((uint8_t)value.toInt());
  // if (address > 0)
  // {
  //   if (address <= WIFI_BEGIN_END)
  //     EEPROM.writeString(address, value);
  //   else if (address <= WIFI_CONFIG_END)
  //     EEPROM.writeByte(address, (uint8_t)value.toInt());
  //   else if (address <= ETH_CONFIG_END)
  //     EEPROM.writeByte(address, (uint8_t)value.toInt());
  //   else if (address <= DEVICE_CONFIG_END)
  //     EEPROM.writeShort(address, (int16_t)value.toInt());
  //   else if (address <= PLASTIK_END)
  //     EEPROM.writeShort(address, (int16_t)value.toInt());

  //   EEPROM.commit();
  //   registerUpdate();
  // }

  
}

String processorRead(const String &var)
{
 //todo:last

  short kategori_index=var.indexOf('_');
  String kategori = var.substring(0,kategori_index);
  short register_index1=var.indexOf('_',kategori_index+1);
  short register_num1 = var.substring(kategori_index+1,register_index1).toInt();
  short register_index2=var.indexOf('_',register_index1+1);
  short register_num2 = var.substring(register_index1+1,register_index2).toInt();
  Serial.println("processor Read");
  
  if (kategori=="wb")
    return String((const char*)cm.wifi_begin[register_num1]);
  else if (kategori=="wc")
    return String(cm.wifi_config[register_num1][register_num2]);
  else if (kategori=="dc")
    return String(cm.device_config[register_num1][register_num2]);
  else if (kategori=="dr")
    return String(cm.device_register[register_num1][register_num2]);

   return String();
}


bool checkRX(int Start){

  if (client&&client.connected()) {
    byteFN=MB_FC_NONE;
    if(client.available())
    {
      flagClientConnected = 1;
      int i = 0;
      while(client.available())
      {
        ByteArrayRx[i] = client.read();
        #ifdef DEBUG_MODE
       // Serial.println(ByteArrayRx[i]);
       #endif
        i++;
      }
      
      client.flush();

      //// rutine Modbus TCP
      byteFN = ByteArrayRx[MB_TCP_FUNC];      
      
     
    }
     
     // Handle request
     
     switch(byteFN) {
        case MB_FC_NONE:
        return false; 
        break;

        case MB_FC_READ_REGISTERS: // 03 Read Holding Registers
          
          for(int i = 0; i < (ByteArrayRx[MB_TCP_RX_BYTE_LEN]/2); i++)
          {
            byteFN = MB_FC_NONE; 
            
            
            if((Start+i)==end_of_work){
              byte tempLow=ByteArrayRx[(MB_TCP_RX_DATA+1) + (i * 2)];
              byte tempHigh=ByteArrayRx[ MB_TCP_RX_DATA + (i * 2)];
              if((tempLow>0)&&(!end_of_work_trigger)){
                cm.device_register[end_of_work][1]=tempHigh;
                cm.device_register[end_of_work][0]=tempLow;
                  cm.st_update(end_of_work); 
                  end_of_work_trigger=true;
              }
              
              
            }
            
          }
          
        return true; 
        break;
  
        case MB_FC_WRITE_REGISTER: case MB_FC_WRITE_MULTIPLE_REGISTERS: // 06 Write Holding Register
        update=false; 
        return true;       
        break;

     } 

     
  }
  return false;
}


///timeout_count per time 10ms
void checkLoop(int start_address,int timeout_count){
  int i=0;
  while(!checkRX(start_address)&&i<20){
      delay(10);
      i++;
  }
  if(i==20)
  client.stop();

}



void mb_read_holding_register(int start_address,int number_of_value)
{
  int ByteDataLength = number_of_value * 2;
  int MessageLength = ByteDataLength + 10;
  ByteArrayTx[MB_TCP_LEN] =highByte(ByteDataLength + 4);
  ByteArrayTx[MB_TCP_LEN+1] =lowByte(ByteDataLength + 4);
  ByteArrayTx[MB_TCP_FUNC]=MB_FC_READ_REGISTERS;
  ByteArrayTx[MB_TCP_REGISTER_START]=highByte(start_address);
  ByteArrayTx[MB_TCP_REGISTER_START+1]=lowByte(start_address);  
  
  ByteArrayTx[MB_TCP_REGISTER_NUMBER]=0;
  ByteArrayTx[MB_TCP_REGISTER_NUMBER+1]=1;
  
  client.write((const uint8_t *)ByteArrayTx,MessageLength);

}

void mb_write_holding_register(int start_address,int number_of_value)
{

  int ByteDataLength=number_of_value * 2;
  int MessageLength = ByteDataLength + 13;
  ByteArrayTx[MB_TCP_FUNC]=MB_FC_WRITE_MULTIPLE_REGISTERS;
  ByteArrayTx[MB_TCP_REGISTER_START]=highByte(start_address);  
  ByteArrayTx[MB_TCP_REGISTER_START+1]=lowByte(start_address);  

   
  ByteArrayTx[MB_TCP_LEN] =highByte(ByteDataLength + 7);
  ByteArrayTx[MB_TCP_LEN+1] =lowByte(ByteDataLength + 7); 

  ByteArrayTx[MB_TCP_REGISTER_NUMBER] = highByte(number_of_value);
  ByteArrayTx[MB_TCP_REGISTER_NUMBER+1] = lowByte(number_of_value);
  ByteArrayTx[MB_TCP_BYTE_DATA_LEN] = ByteDataLength;
  for(int i = 0; i < number_of_value; i++)
  {
  ByteArrayTx[ MB_TCP_WRITE_DATA + (i * 2)] = cm.device_register[start_address + i][1];// high byte
  ByteArrayTx[MB_TCP_WRITE_DATA+1 + (i * 2)] = cm.device_register[start_address + i][0];// low byte
  }
  client.write((const uint8_t *)ByteArrayTx,MessageLength); 

}

bool receive_update() {

   //if(cm.serialEventEN){
    if(Serial.available())
    {      
      cm.stRxData[FUNC]=Serial.read();
      delay(1);
      if(cm.stRxData[FUNC]==UPDATE){
        int i=START_ADDRESS;
        while(Serial.available()){
        cm.stRxData[i]=Serial.read(); 
        //delayMicroseconds(100);
        i++; 
        }
        cm.device_register_update(cm.stRxData);
        return true;
      }
    
      else if(cm.stRxData[FUNC]==MOD){
        mod();        
      }
    }
    else
    return false;

    return false;
  //}
}

void mod(){
    delay(1);
    byte modData=Serial.read();
    if(modData==AP_STA_MOD){
      WiFi.disconnect();
      WiFi.softAP((const char*)cm.wifi_begin[ap_ssid],(const char*)cm.wifi_begin[ap_password]); 
      WiFi.mode(WIFI_AP_STA);     
      WiFi.begin((const char*)cm.wifi_begin[sta_ssid],(const char*)cm.wifi_begin[sta_password]);
      delay(1000);      
         
      Serial.flush();
      cm.device_register[ap_sta_mod][0]=1;
      cm.st_update(ap_sta_mod);
    }
}


// bool is_authenticated() {
//    if (server.hasHeader("Cookie")) {    
//     String cookie = server.header("Cookie");
    
//     if (cookie.indexOf("ESPSESSIONID=1") != -1 ) {      
//       return true;
//     }

//   }  
//   return false;
// }


//login page, also called for disconnect

// void css(){
 
// server.send(200, "text/css", style_css);
// }
// void handleLogin() {
//   String msg;
//   if (server.hasHeader("Cookie")) {    
//     String cookie = server.header("Cookie");    
//   }
//   if (server.hasArg("DISCONNECT")) {    
//     server.sendHeader("Location", "/login");
//     server.sendHeader("Cache-Control", "no-cache");
//     server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
//     server.send(301);
//     return;
//   }
//   if (server.hasArg("uname") && server.hasArg("psw")) {

//     if (server.arg("uname") == "admin" &&  server.arg("psw") == "realtekno") {
//       server.sendHeader("Location", "/");
//       server.sendHeader("Cache-Control", "no-cache");
//       server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
//       server.send(301);
//       return;
//     }
//   }
//   String content=login_html;  
//   content.replace("@uname",server.arg("uname"));
//   server.send(200, "text/html", content);
// }

// //root page can be accessed only if authentication is ok
// void handleRoot() {
//   if (!is_authenticated()) {
//     server.sendHeader("Location", "/login");
//     server.sendHeader("Cache-Control", "no-cache");
//     server.send(301);
//     return;
//   }
//   server.send(200, "text/html", dashboard_html);
// }
// void handlePublic(){
//   if (server.hasArg("reset")) {
//     if(server.arg("reset")=="1"&&server.arg("pswd")=="martur1234")    
//       cm.device_register[end_of_work][0]=1;
//       cm.st_update(end_of_work);
//       while(!cm.device_register_check())
//       delay(100);
//    }
//   String content=public_html;
//   word max=word(cm.device_config[distance_val1][1],cm.device_config[distance_val1][0]);
//   word min=max-1230;
//   content.replace("@min",String(min));
//   content.replace("@max",String(max));
//   server.send(200, "text/html", content);
// }
// void handleWifiSet() {
  
//   if (!is_authenticated()) {
//     server.sendHeader("Location", "/login");
//     server.sendHeader("Cache-Control", "no-cache");
//     server.send(301);
//     return;
//   }

//   if (server.hasArg("form")) {
//     cm.wifi_begin_update(sta_ssid,server.arg("sta_ssid"));
//     cm.wifi_begin_update(sta_password,server.arg("sta_password"));
//     cm.wifi_begin_update(ap_ssid,server.arg("ap_ssid"));
//     cm.wifi_begin_update(ap_password,server.arg("ap_password"));
//     cm.wifi_config[sta_ip][0]=server.arg("sta_ip1").toInt();
//     cm.wifi_config[sta_ip][1]=server.arg("sta_ip2").toInt();
//     cm.wifi_config[sta_ip][2]=server.arg("sta_ip3").toInt();
//     cm.wifi_config[sta_ip][3]=server.arg("sta_ip4").toInt();
//     cm.wifi_config[subnet][0]=server.arg("subnet1").toInt();
//     cm.wifi_config[subnet][1]=server.arg("subnet2").toInt();
//     cm.wifi_config[subnet][2]=server.arg("subnet3").toInt();
//     cm.wifi_config[subnet][3]=server.arg("subnet4").toInt();
//     cm.wifi_config[sta_gateway][0]=server.arg("sta_gateway1").toInt();
//     cm.wifi_config[sta_gateway][1]=server.arg("sta_gateway2").toInt();
//     cm.wifi_config[sta_gateway][2]=server.arg("sta_gateway3").toInt();
//     cm.wifi_config[sta_gateway][3]=server.arg("sta_gateway4").toInt();
//     cm.wifi_config[sta_dns][0]=server.arg("dns1").toInt();
//     cm.wifi_config[sta_dns][1]=server.arg("dns2").toInt();
//     cm.wifi_config[sta_dns][2]=server.arg("dns3").toInt();
//     cm.wifi_config[sta_dns][3]=server.arg("dns4").toInt();
//     cm.write(WIFI_CONFIG,wifi_config_size*wifi_config_lenght,(byte*)cm.wifi_config,DATA_TYPE_8);
//     cm.write(WIFI_BEGIN,wifi_begin_size*wifi_begin_lenght,(byte*)cm.wifi_begin,DATA_TYPE_8);  
//     while(!cm.save());
//   }
//   String content=wifi_set_html;
//   content.replace("@sta_ssid",(const char*)cm.wifi_begin[sta_ssid]);
//   content.replace("@sta_password",(const char*)cm.wifi_begin[sta_password]);
//   content.replace("@ap_ssid",(const char*)cm.wifi_begin[ap_ssid]);
//   content.replace("@ap_password",(const char*)cm.wifi_begin[ap_password]);
//   content.replace("@sta_ip1",String(cm.wifi_config[sta_ip][0]));
//   content.replace("@sta_ip2",String(cm.wifi_config[sta_ip][1]));
//   content.replace("@sta_ip3",String(cm.wifi_config[sta_ip][2]));
//   content.replace("@sta_ip4",String(cm.wifi_config[sta_ip][3]));
//   content.replace("@subnet1",String(cm.wifi_config[subnet][0]));
//   content.replace("@subnet2",String(cm.wifi_config[subnet][1]));
//   content.replace("@subnet3",String(cm.wifi_config[subnet][2]));
//   content.replace("@subnet4",String(cm.wifi_config[subnet][3]));
//   content.replace("@sta_gateway1",String(cm.wifi_config[sta_gateway][0]));
//   content.replace("@sta_gateway2",String(cm.wifi_config[sta_gateway][1]));
//   content.replace("@sta_gateway3",String(cm.wifi_config[sta_gateway][2]));
//   content.replace("@sta_gateway4",String(cm.wifi_config[sta_gateway][3]));
//   content.replace("@sta_dns1",String(cm.wifi_config[sta_dns][0]));
//   content.replace("@sta_dns2",String(cm.wifi_config[sta_dns][1]));
//   content.replace("@sta_dns3",String(cm.wifi_config[sta_dns][2]));
//   content.replace("@sta_dns4",String(cm.wifi_config[sta_dns][3]));
//   server.send(200, "text/html", content);
// }
// void handleDeviceSet() {
//   if (!is_authenticated()) {
//     server.sendHeader("Location", "/login");
//     server.sendHeader("Cache-Control", "no-cache");
//     server.send(301);
//     return;
//   }
//   if (server.hasArg("form")) {
//     cm.device_config_update(distance_val1,server.arg("distance_val1"));
//     cm.device_config_update(sleep_t,server.arg("sleep_t"));
//     cm.device_config_update(sampling_range,server.arg("sampling_range"));
//     cm.device_config_update(number_of_samples_for_fabric,server.arg("number_of_samples_for_fabric"));
//     cm.device_config_update(tolerance,server.arg("tolerance"));
//     cm.device_config_update(encoder_rev_pulse,server.arg("encoder_rev_pulse"));
//     cm.device_config_update(circle_dim,server.arg("circle_dim"));
//     cm.device_config_update(step_rev_pulse,server.arg("step_rev_pulse"));
//     cm.write(DEVICE_CONFIG,device_config_size,(byte*)cm.device_config,DATA_TYPE_16);
//     while(!cm.save());
//   }
//   String content=device_set_html;
//   content.replace("@distance_val1",cm.DCToStr(distance_val1));
//   content.replace("@sleep_t",cm.DCToStr(sleep_t));  
//   content.replace("@sampling_range",cm.DCToStr(sampling_range)); 
//   content.replace("@number_of_samples_for_fabric",cm.DCToStr(number_of_samples_for_fabric));     
//   content.replace("@tolerance",cm.DCToStr(tolerance)); 
//   content.replace("@encoder_rev_pulse",cm.DCToStr(encoder_rev_pulse)); 
//   content.replace("@circle_dim",cm.DCToStr(circle_dim)); 
//   content.replace("@step_rev_pulse",cm.DCToStr(step_rev_pulse)); 
//   server.send(200, "text/html", content);
// }

// void handleNotFound() {
//   String message = "File Not Found\n\n";
//   message += "URI: ";
//   message += server.uri();
//   message += "\nMethod: ";
//   message += (server.method() == HTTP_GET) ? "GET" : "POST";
//   message += "\nArguments: ";
//   message += server.args();
//   message += "\n";
//   for (uint8_t i = 0; i < server.args(); i++) {
//     message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
//   }
//   server.send(404, "text/plain", message);
// }