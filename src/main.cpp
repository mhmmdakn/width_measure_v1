#include <main.h>

//#define DEBUG_MODE

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
bool post_event=false;
bool loop_state=false;
bool update=false;
bool plc_status=false;
AsyncWebServer server(80);

void setup(){
  //ESP.wdtEnable(1000);
  Serial.begin(115200); 
  delay(1000); 
  if (!SPIFFS.begin())
  {
    #ifdef DEBUG_MODE
    Serial.println("An Error has occurred while mounting SPIFFS");
    #endif
  }

  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);
  delay(500);
  ////
    cm.device_register[wifi_ok][0]=0;
    cm.st_update(wifi_ok);

    while(!cm.wifi_begin_check())
    delay(1500);
    while(!cm.wifi_config_check())
    delay(1000);
    while(!cm.device_config_check())
    delay(500);
    

    
  ///

  WiFi.disconnect();
    delay(1000) ;
    WiFi.mode(WIFI_STA);
   // WiFi.config(IPAddress(10,7,11,100),IPAddress(10,7,11,250),IPAddress(255,255,255,0));
   // WiFi.config(IPAddress(192,168,1,150),IPAddress(192,168,1,1),IPAddress(255,255,255,0));
    
    //WiFi.begin("REALTEKNO","20192019");
    IPAddress STA_IP(cm.wifi_config[sta_ip]);
    IPAddress STA_DNS(cm.wifi_config[sta_dns]);
    IPAddress STA_GATEWAY(cm.wifi_config[sta_gateway]);
    IPAddress SUBNET(cm.wifi_config[subnet]);



    WiFi.config(STA_IP,STA_DNS,STA_GATEWAY,SUBNET);  
    WiFi.begin((const char*)cm.wifi_begin[sta_ssid],(const char*)cm.wifi_begin[sta_password]);



    delay(1000) ;  
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);  
    // Wait for connection
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20)
    { //wait 10 seconds
      delay(500);
    }
    if (i == 7)
      {
        #ifdef DEBUG_MODE
        Serial.print("Could not connect to");  
        #endif    
        
      }
      else
      {
        #ifdef DEBUG_MODE
        Serial.print("Connected! IP address: ");
        Serial.println(WiFi.localIP());
       #endif
      cm.device_register[wifi_ok][0]=1;
      cm.st_update(wifi_ok);
      }
 
  //aktif edilecek 
     server.on("/",handlePage);
     server.on("/public.html", handlePage);
     server.on("/wifi_settings.html", handlePage);
     server.on("/device_settings.html", handlePage);
     server.on("/data", handleData);     
     server.onNotFound(handleNotFound);
     server.begin(); 



    // cm.device_register[ap_sta_mod][0]=1;
    // cm.st_update(ap_sta_mod);
   
  //aktif edilecek


update=true;
}

void loop(){

  
  //server.handleClient();
  if(!post_event){

    loop_state=true;
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
      plc_status=true;
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
        plc_status=false;
      // Serial.println("client not connect");
        client.connect(IPAddress(10,7,11,190),502);
        delay(1000);
    }
  }
  else
  {
      
      
      cm.write(WIFI_CONFIG,wifi_config_size*wifi_config_lenght,(byte*)cm.wifi_config,DATA_TYPE_8);
      cm.write(WIFI_BEGIN,wifi_begin_size*wifi_begin_lenght,(byte*)cm.wifi_begin,DATA_TYPE_8);
      cm.write(DEVICE_CONFIG,device_config_size,(byte*)cm.device_config,DATA_TYPE_16);  
      while(!cm.save());

      post_event=false;
    
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

  // String pathWithGz = path + ".gz";
  // if ((SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)))
  // {
  //   if (SPIFFS.exists(pathWithGz))
  //   {
  //     path = pathWithGz;
  //     dataType = "application/x-gzip";
  //   }
  // }

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
  #ifdef DEBUG_MODE
  Serial.print(message);
  #endif
}
void handlePage(AsyncWebServerRequest *request)
{

  
  if (request->authenticate("admin", "realtekno", "Measuring Width"))
  {
    if (request->method() == HTTP_POST)
    {
      post_event=true;
      
      
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
    myObject["plc_sta"]=plc_status;
    myObject["cwidth_val"]=word(cm.device_register[cwidth_val][1],cm.device_register[cwidth_val][0]);
    myObject["mwidth_val"]=word(cm.device_register[mwidth_val][1],cm.device_register[mwidth_val][0]);
    myObject["number_of_samples"]=word(cm.device_register[number_of_samples][1],cm.device_register[number_of_samples][0]);
    myObject["fabric_ref"]=word(cm.device_register[fabric_ref][1],cm.device_register[fabric_ref][0]);
    String jsonString = JSON.stringify(myObject);
    
    request->send(200, "text/plane",jsonString);

}
void processorWrite(String name, String value)
{
  if(name!=""){
    short kategori_index=name.indexOf('_');
    String kategori = name.substring(0,kategori_index);
    short register_index1=name.indexOf('_',kategori_index+1);
    short register_num1 = name.substring(kategori_index+1,register_index1).toInt();  
    short register_num2 = name.substring(register_index1+1,name.length()).toInt();
 
      if (kategori=="wb")  
        cm.wifi_begin_update(register_num1,value);
      else if (kategori=="wc")
        cm.wifi_config[register_num1][register_num2]=value.toInt();
      else if (kategori=="dc")
        cm.device_config_update(register_num1,value);
  }
}

String processorRead(const String &var)
{
 //todo:last
  if(var!=""){  
    short kategori_index=var.indexOf('_');
    String kategori = var.substring(0,kategori_index);
    short register_index1=var.indexOf('_',kategori_index+1);
    short register_num1 = var.substring(kategori_index+1,register_index1).toInt();  
    short register_num2 = var.substring(register_index1+1,var.length()).toInt();
    
    if (kategori=="wb")  
      return String((char*)cm.wifi_begin[register_num1]);
    else if (kategori=="wc")
      return String(cm.wifi_config[register_num1][register_num2]);
    else if (kategori=="dc")
      return String(cm.DCToStr(register_num1));
    else if (kategori=="dr")
      return String(cm.DRToStr(register_num1));
  }
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
 
}

void mod(){
    
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
