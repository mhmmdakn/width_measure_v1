
#include "SerialCom.h"
#include "HardwareSerial.h"
#include "EEPROM.h"


    
    void COMM::device_register_update(byte* byteArray){
      
      int start_address=word(byteArray[START_ADDRESS],byteArray[START_ADDRESS+1]); 
      int register_start_address=DEVICE_REGISTER; 
      int point=start_address-register_start_address;
      int number_of_value=byteArray[NUMBER_OF_VALUE];
      int k=point*2;
      for(int i=0;i<number_of_value;i++){     
      *((uint8_t *)device_register+k)=((uint8_t *)byteArray)[DATA+i];
      k++;
      } 
      
    }

    void COMM::st_update(byte point){
    ///düzenlenecek
        
        stTxData[FUNC]=UPDATE;              
        stTxData[START_ADDRESS]=highByte(DEVICE_REGISTER+point);
        stTxData[START_ADDRESS+1]=lowByte(DEVICE_REGISTER+point);
        stTxData[NUMBER_OF_VALUE]=2;
        stTxData[DATA]=device_register[point][0]; 
        stTxData[DATA+1]=device_register[point][1]; 
        
        do{
        ESP.wdtFeed();
        Serial.write(stTxData,PACKAGE_LENGTH);        
        }        
        while(!success());
                
      }
    void COMM::register_int(){
            
      for(int i=0;i<wifi_begin_size;i++)
        for(int j=0;j<wifi_begin_lenght;j++)
          wifi_begin[i][j]=EEPROM.read(WIFI_BEGIN+j);  

      for(int i=0;i<wifi_config_size;i++)
        for(int j=0;j<wifi_config_lenght;j++)
          wifi_config[i][j]=EEPROM.read(WIFI_CONFIG+j);

      for(int i=0;i<device_config_size;i++)
        for(int j=0;j<device_config_lenght;j++)
          device_config[i][j]=EEPROM.read(DEVICE_CONFIG+j);
      
    }
    
    bool COMM::success(){            
    int time_c=0;
    //serialEventEN=0;
    while(time_c<TIME_OUT&&(!Serial.available())){  
    delay(1);
    time_c++;
    }
    
    if((time_c==TIME_OUT)||(OK!=Serial.read())){
    //serialEventEN=1;
    return false;
    }
    //serialEventEN=1;
    return true;
    
    }
      
    bool COMM::save(){        
      
      stTxData[FUNC]=SAVE;       
      Serial.write(stTxData,PACKAGE_LENGTH);
      delay(100);
      if(!success())
      return false;
      else
      return true;
    }
      
     bool COMM::write(int start_address,byte number_of_val,byte* byteArray,DataTypeDef dataType){
      
          
      stTxData[FUNC]=WRITE;
            
      if(dataType==DATA_TYPE_16)
      for(int i=0;i<number_of_val;i++){
      stTxData[START_ADDRESS]=highByte(start_address+i);
      stTxData[START_ADDRESS+1]=lowByte(start_address+i);
      stTxData[NUMBER_OF_VALUE]=2;
      stTxData[DATA]=byteArray[2*i];
      stTxData[DATA+1]=byteArray[(2*i)+1];  
      Serial.write(stTxData,PACKAGE_LENGTH);
      if(!success())
      i--;     
      }
      
      else if(dataType==DATA_TYPE_8)
      for(int i=0;i<number_of_val;i++){
      stTxData[START_ADDRESS]=highByte(start_address+i);
      stTxData[START_ADDRESS+1]=lowByte(start_address+i);
      stTxData[NUMBER_OF_VALUE]=1;
      stTxData[DATA]=byteArray[i];       
      Serial.write(stTxData,PACKAGE_LENGTH);
      if(!success())
      i--;            
      }   
        
      return true;
      }
  
      bool COMM::read(int start_address,byte number_of_val,byte* byteArray){
      //serialEventEN=0;      
      int time_c=0;
      
      byteArray[FUNC]=READ;
      byteArray[START_ADDRESS]=highByte(start_address);
      byteArray[START_ADDRESS+1]=lowByte(start_address);
      byteArray[NUMBER_OF_VALUE]=number_of_val;  
      Serial.write(byteArray,PACKAGE_LENGTH);
      
      
      while(time_c<TIME_OUT&&(!Serial.available())){  
      delay(1);
      time_c++;
      }
      if(time_c==TIME_OUT){
      byteArray[FUNC]=ERROR;
      byteArray[DATA]=TIME_OUT_ERROR;
      //serialEventEN=1;
      return byteArray;
      }
      
      int i=0;
      while(Serial.available()){
      byteArray[i]=Serial.read(); 
      delay(1);
      i++; 
      }
    
      if(byteArray[FUNC]!=RESPONSE){
      byteArray[FUNC]=ERROR;
      byteArray[DATA]=RESPONSE_ERROR;
      }
      //serialEventEN=1;
      return byteArray;
      }
  
     bool COMM::wifi_begin_check(){     
      byte arraySize=(wifi_begin_lenght*wifi_begin_size)+MESSAGE_LENGTH;
      byte byteArray[arraySize];     
      read(WIFI_BEGIN,arraySize,byteArray);      
      if(byteArray[FUNC]==ERROR)
      return false;

      int k=MESSAGE_LENGTH;
      for(int i=0;i<wifi_begin_size;i++){
        for(int j=0;j<wifi_begin_lenght;j++){          
         wifi_begin[i][j]=byteArray[k];
         k++;
        }
      }            
      return true;
    }
    

     bool COMM::wifi_config_check(){
      byte arraySize=(wifi_config_lenght*wifi_config_size)+MESSAGE_LENGTH;
      byte byteArray[arraySize];     
      read(WIFI_CONFIG,arraySize,byteArray);      
      if(byteArray[FUNC]==ERROR)
      return false;

      int k=MESSAGE_LENGTH;
      for(int i=0;i<wifi_config_size;i++){
        for(int j=0;j<wifi_config_lenght;j++){          
         wifi_config[i][j]=byteArray[k];
         k++;
        }
      }            
      return true;
    }
    
     bool COMM::device_config_check(){
      byte arraySize=(device_config_lenght*device_config_size)+MESSAGE_LENGTH;
      byte byteArray[arraySize];     
      read(DEVICE_CONFIG,arraySize,byteArray);      
      if(byteArray[FUNC]==ERROR)
      return false;

      int k=MESSAGE_LENGTH;
      for(int i=0;i<device_config_size;i++){
        for(int j=0;j<device_config_lenght;j++){          
         device_config[i][j]=byteArray[k];
         k++;
        }
      }            
      return true;
    }
    bool COMM::device_register_check(){
      byte arraySize=(device_register_lenght*device_register_size)+MESSAGE_LENGTH;
      byte byteArray[arraySize];     
      read(DEVICE_REGISTER,arraySize,byteArray);      
      if(byteArray[FUNC]==ERROR)
      return false;
      int k=MESSAGE_LENGTH;
      for(int i=0;i<device_register_size;i++){
        for(int j=0;j<device_register_lenght;j++){          
         device_register[i][j]=byteArray[k];
         k++;
        }
      }            
      return true;
    }
    void COMM::device_config_update(int index,String value)
    {
      device_config[index][0]=lowByte(value.toInt());
      device_config[index][1]=highByte(value.toInt());
    }
    
    void COMM::wifi_begin_update(int index,String value)
    {
      value.toCharArray(wifi_begin[index],wifi_begin_lenght);
    }
    
    String COMM::DCToStr(int index){
      
      return String(word(device_config[index][1],device_config[index][0]));
      
    }
    String COMM::DRToStr(int index){
      
      return String(word(device_register[index][1],device_register[index][0]));
      
    }

    String COMM::WBToStr(int index){
      
      return String();
      
    }






    
