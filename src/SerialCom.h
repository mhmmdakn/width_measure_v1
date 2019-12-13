#ifndef SerialCom_h
#define SerialCom_h

#include <Arduino.h>

//////////Serial Protocol////////////
#define FUNC 0            // 1 byte
#define START_ADDRESS 1   // 2 byte
#define NUMBER_OF_VALUE 3 // 1 byte
#define DATA 4            // 1 byte
////////////////////////////////////

//////////Serial Function///////////
#define READ 1
#define WRITE 2
#define RESPONSE 3
#define UPDATE 4
#define SAVE 5
#define OK 6
#define MOD 7
#define ERROR 8
////////////////////////////////////

//////////Error Code////////////////
#define READ_ERROR 1
#define WRITE_ERROR 2
#define RESPONSE_ERROR 3
#define UPDATE_ERROR 4
#define TIME_OUT_ERROR 5
////////////////////////////////////




///////Register Start Address///////
#define WIFI_BEGIN 100
#define WIFI_CONFIG 200
#define DEVICE_CONFIG 300
#define DEVICE_REGISTER 400
#define ERROR_REGISTER 500
////////////////////////////////////

///////Register END Address///////
#define WIFI_BEGIN_END 199
#define WIFI_CONFIG_END 299
#define DEVICE_CONFIG_END 399
#define ERROR_REGISTER_END 599
////////////////////////////////////


#define TIME_OUT 100
#define MESSAGE_LENGTH 4 
#define PACKAGE_LENGTH 6



#define wifi_begin_lenght 16
#define wifi_config_lenght 4
#define device_config_lenght 2
#define device_register_lenght 2


  typedef enum{
  DATA_TYPE_8=0,
  DATA_TYPE_16,
  DATA_TYPE_32
  }DataTypeDef;
 enum{
  sta_ssid,
  sta_password,
  ap_ssid,
  ap_password,  
  wifi_begin_size 
  };///WIFI_BEGIN
  
  enum{
  sta_ip,
  sta_dns,
  sta_gateway,
  subnet,
  ap_ip,
  ap_gateway,
  modbus_port,
  wifi_config_size    
  };///WIFI_CONFIG
 
  
  enum{   
  distance_val1,
  offsetof,  
  encoder_rev_pulse,
  circle_dim,
  step_rev_pulse,
  min_pulse,
  r1_m,
  r1_s,
  r2_m,
  r2_s,
  r3_m,
  r3_s,
  sleep_t,
  sampling_range,   
  number_of_samples_for_fabric,
  tolerance,
  factory_settings,
  device_config_size
  };///DEVICE_CONFIG
  
  enum{ 
  end_of_work,
  cwidth_val,    
  mwidth_val,
  number_of_samples,
  fabric_ref,
  measured, 
  fault,  
  run_stop,
  wifi_ok,
  ap_sta_mod,
  updt,  
  device_register_size
  };///DEVICE_REGISTER
class COMM{
  public:
      bool serialEventEN=0;
      byte stRxData[127]={0};
      byte stTxData[6]={0};     

      
      byte wifi_begin[wifi_begin_size][wifi_begin_lenght];
      byte wifi_config[wifi_config_size][wifi_config_lenght];
      byte device_config[device_config_size][device_config_lenght];
      byte device_register[device_register_size][device_register_lenght];

      void device_register_update(byte* byteArray);
      void device_config_update(int index,String value);
      void wifi_begin_update(int index,String value);
      void st_update(byte point);
      void register_int();
      bool success();
      bool save();
      bool read(int start_address,byte number_of_val,byte* byteArray);
      bool write(int start_address,byte number_of_val,byte* byteArray,DataTypeDef dataType);
      bool wifi_begin_check();
      bool wifi_config_check();
      bool device_config_check();
      bool device_register_check();
      String DCToStr(int index);
      String DRToStr(int index);
      String WCToStr(int index);
      String WBToStr(int index);
};
      
      
#endif
