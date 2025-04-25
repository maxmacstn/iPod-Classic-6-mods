#include "BluetoothA2DPSource.h"
#include "AudioTools.h"

#include "AudioTools/AudioLibs/A2DPStream.h"

I2SStream i2sStream;   // Access I2S as stream
A2DPStream a2dpStream; // access A2DP as stream

// I2S Config
static const int i2s_num = 0; // i2s port number

#define I2S_PIN_BCK         25
#define I2S_PIN_WS          17
#define I2S_PIN_DATA        16
#define LED_PIN 23
#define ADC_INT 26
#define ADC_MD_0 32
#define ADC_MD_1 33
#define ADC_MD_2 13
#define ADC_MD_3 27
#define TX_IPOD 19
#define RX_IPOD 18

// Stream Setting
#define I2S_SAMPLE_RATE 44100
#define I2S_BIT_RATE I2S_BITS_PER_SAMPLE_16BIT
#define I2S_PORT I2S_NUM_0

//Bluetooth Class of Device filter
#define BLUETOOTH_COD_MASK 0xFFF000
#define BLUETOOTH_COD_AUDIO 0x240000

//Timing
#define BLUETOOTH_SEARCH_TIME 15      //Search for bluetooth headphones for 15s
#define BLUETOOTH_CONNECTION_TIME 80  //The device must be able to connect to headphone within 60s, otherwise sleep.
#define AUDIO_INACTIVE_SLEEP_TIME 5*60    //If the audio is inactive for 5 minutes, then sleep.

StreamCopy copier(a2dpStream, i2sStream); // copies sound to out

// Task & Queues
QueueHandle_t pt_event_queue;
TaskHandle_t ipod_control_task_handle = NULL;
volatile bool isAudioActive = false;
unsigned long bluetoothSearchStartTime ;

//Prototypes
void bluetoothButtonHandler(uint8_t id, bool isReleased);
void setupBluetooth();
void notifyiPodBluetoothConnected();
void checkDeepSleep(bool isAudioActive);

uint8_t buffer[2048];
int16_t *samples = (int16_t*) buffer;

hw_timer_t *connectionMonitorTimer = NULL;

void IRAM_ATTR checkConnection(){
  static bool LEDState = 1;
  bool isBluetoothConnected = a2dpStream.source().is_connected();
  Serial.printf("Bluetooth is %s\n", isBluetoothConnected?"connected":"not connected");

  if (!isBluetoothConnected && millis() - bluetoothSearchStartTime > BLUETOOTH_CONNECTION_TIME*1000){
    //Could not connect in time, sleep the ESP.
    Serial.println("Could not connect to headphones/speakers, sleep.");
    esp_deep_sleep_start();
  }

  //Set LED
  if(isBluetoothConnected){
    digitalWrite(LED_PIN, HIGH);
  }else{
    digitalWrite(LED_PIN, LEDState);
    LEDState = !LEDState;
  }
}


//Bluetooth
struct BluetoothDeviceFound
{
  String ssid;
  esp_bd_addr_t address;
  int rssi;
};

BluetoothDeviceFound foundDevices[10];
int totalFound = 0;
int minRssi = -999;
esp_bd_addr_t minRssiMac;



// Arduino Setup
void setup(void)
{
  pinMode(LED_PIN, OUTPUT);

  for (int i = 0 ; i < 20; i++){
    digitalWrite(LED_PIN, i%2);
    delay(1000);
  }

  Serial.begin(115200);
  Serial2.begin(19200,SERIAL_8N1,RX_IPOD, TX_IPOD);
  // AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);


  pinMode(ADC_MD_0,OUTPUT);
  pinMode(ADC_MD_1,OUTPUT);
  pinMode(ADC_MD_2,OUTPUT);
  pinMode(ADC_MD_3,OUTPUT);

  digitalWrite(ADC_MD_0,LOW);
  digitalWrite(ADC_MD_1,LOW);
  digitalWrite(ADC_MD_2,LOW);
  digitalWrite(ADC_MD_3,LOW);


  // start i2s input with default configuration
  setupBluetooth();
  Serial.println("starting I2S...");
  // a2dpStream.addNotifyAudioChange(i2sStream); // i2s is using the info from a2dp


  i2sStream.addNotifyAudioChange(a2dpStream);

  I2SConfig i2sConfig = i2sStream.defaultConfig(RX_MODE);
  i2sConfig.pin_data = I2S_PIN_DATA;
  i2sConfig.pin_bck = I2S_PIN_BCK;
  i2sConfig.pin_ws = I2S_PIN_WS;
  i2sConfig.pin_mck = 0;
  i2sConfig.is_master = true;
  i2sConfig.use_apll = false;
  i2sConfig.auto_clear =true;
  i2sConfig.sample_rate = 44100;
  i2sConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  // i2sConfig.buffer_count = 16;
  // i2sConfig.buffer_size = 32;
  i2sConfig.channels = 2;
  i2sConfig.i2s_format = I2S_STD_FORMAT;

  i2sStream.begin(i2sConfig);

  Serial.println("System Ready..");

  notifyiPodBluetoothConnected();


}

// Arduino loop - copy data
void loop()
{

   size_t read = i2sStream.readBytes(buffer, 2048);
   size_t samples_read = read/sizeof(int16_t);

   
   int32_t avg =0;
   int min = INT_MAX;
   int max = INT_MIN;
   for (int i = 0 ;i < samples_read; i++){
    if (buffer[i] < min)
      min = samples[i];
    if (buffer[i] > max)
      max = samples[i];    
   }
   int diff =  max-min;
  //  Serial.printf("diff = %d\n", diff);

   if (diff < 200){
    memset(buffer, 0, sizeof buffer);
    isAudioActive = false;
  }else{
    isAudioActive = true;
  }
  
  a2dpStream.write(buffer, read);

  // copier.copy();
  checkDeepSleep(isAudioActive);

}

void checkDeepSleep(bool isAudioActive){
  static bool lastState = true;
  static unsigned long audioInactiveStartTime = 0;


  if(lastState != isAudioActive){
    // Serial.printf("Audio Active: %d\n", isAudioActive);
    if (!isAudioActive){
      audioInactiveStartTime = millis();
    }
  }

  if (!isAudioActive && millis() - audioInactiveStartTime > AUDIO_INACTIVE_SLEEP_TIME *1000){
    Serial.println("Start Deepsleep");
    esp_deep_sleep_start();
  }

  lastState = isAudioActive;
}

void notifyiPodBluetoothConnected(){
  for(int i = 0; i <3; i++){
    Serial2.write((uint8_t*)"\xFFU\x06\x02\x00\x00\x00\x00\x02\xF6\xFFU\x03\x02\x00\x00\xFB", 17);
    delay(500);
    Serial2.write((uint8_t*)"\xFFU\x06\x02\x00\x00\x00\x00\x01\xF7\xFFU\x03\x02\x00\x00\xFB", 17);
    delay(500);


  }


}

void onBluetoothConnectionChanged(esp_a2d_connection_state_t state, void *obj){
  Serial.printf("Connection State = %d\n", state);
}

bool isValid(const char* ssid, esp_bd_addr_t address, int rssi, uint32_t cod){
  BluetoothDeviceFound device;
  device.ssid = ssid;
  device.rssi = rssi;
  memcpy(device.address,address,6);

  //Filter by bluetooth CoD
  if((cod & BLUETOOTH_COD_MASK) != BLUETOOTH_COD_AUDIO)
    return false;


  Serial.printf("Found: %s, rssi %d, address %x %x %x %x %x %x, cod %x\n", device.ssid, device.rssi, device.address[5], device.address[4], device.address[3],  device.address[2], device.address[1], device.address[0], cod);


  // Exceed search period, connect if the device has the strongest RSSI.
  if(millis() - bluetoothSearchStartTime > BLUETOOTH_SEARCH_TIME *1000){
    if (memcmp(address,minRssiMac,6) == 0){
      Serial.printf("Connecting to %s\n", ssid);
      return true;

    }
    return false;
  }

  //Add device and record min rssi found.
  if (totalFound < 10){

    //If added, skip.
    for (int i = 0; i < totalFound; i++){
      if (memcmp(address,foundDevices[i].address,6) == 0){
        return false;
        }
    }

    //Add device to array.
    foundDevices[totalFound] =  device;
    if (device.rssi > minRssi){
      minRssi = device.rssi;
      memcpy(minRssiMac,address,6);
    }
    totalFound++;
  }


  
  return false;
}

void setupBluetooth(){
  // start bluetooth
  Serial.println("starting A2DP...");
  auto cfgA2DP = a2dpStream.defaultConfig(TX_MODE);
  // cfgA2DP.name = "EDIFIER W320TN";
  // cfgA2DP.name = "Echo Dot-2N5";
  cfgA2DP.name = "";
  cfgA2DP.auto_reconnect = false;
  a2dpStream.source().set_auto_reconnect(false);
  a2dpStream.source().set_ssid_callback(isValid);
  a2dpStream.source().set_avrc_passthru_command_callback(bluetoothButtonHandler);
  a2dpStream.source().set_local_name("iPod Classic");
  a2dpStream.source().start();

  // set intial volume
  a2dpStream.setVolume(0.5);

  Serial.println("Searching Bluetooth");
  bluetoothSearchStartTime = millis();

  //Set timer for connection monitor
  connectionMonitorTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(connectionMonitorTimer, &checkConnection, true);
  timerAlarmWrite(connectionMonitorTimer, 1000000, true);
  timerAlarmEnable(connectionMonitorTimer); 

  a2dpStream.begin(cfgA2DP);



}


void bluetoothButtonHandler(uint8_t id, bool isReleased) {
  // key codes: https://github.com/espressif/esp-idf/blob/a82e6e63d98bb051d4c59cb3d440c537ab9f74b0/components/bt/host/bluedroid/api/include/api/esp_avrc_api.h#L44-L102
  // key states: 0 = Pressed, 1 = Released
  Serial.printf("Button ID: %d Action: %d\n", id, isReleased);
  if(!isReleased) return;

  switch(id) {
    case ESP_AVRC_PT_CMD_VOL_UP:
      Serial2.write((uint8_t*)"\xFFU\x03\x02\x00\x02\xF9\xFFU\x03\x02\x00\x00\xFB", 14);
      break;
    case ESP_AVRC_PT_CMD_VOL_DOWN:
      Serial2.write((uint8_t*)"\xFFU\x03\x02\x00\x04\xF7\xFFU\x03\x02\x00\x00\xFB", 14);
      break;
    case ESP_AVRC_PT_CMD_FORWARD:
    case ESP_AVRC_PT_CMD_FAST_FORWARD:
      Serial2.write((uint8_t*)"\xFFU\x03\x02\x00\b\xF3\xFFU\x03\x02\x00\x00\xFB", 14);
      break;
    case ESP_AVRC_PT_CMD_BACKWARD:
    case ESP_AVRC_PT_CMD_REWIND:
      Serial2.write((uint8_t*)"\xFFU\x03\x02\x00\x10\xEB\xFFU\x03\x02\x00\x00\xFB", 14);
      break;
    case ESP_AVRC_PT_CMD_STOP:
      Serial2.write((uint8_t*)"\xFFU\x03\x02\x00\x80{\xFFU\x03\x02\x00\x00\xFB", 14);
      break;
    case ESP_AVRC_PT_CMD_PLAY:
      Serial2.write((uint8_t*)"\xFFU\x04\x02\x00\x00\x01\xF9\xFFU\x03\x02\x00\x00\xFB", 15);
      break;
    case ESP_AVRC_PT_CMD_PAUSE:
      Serial2.write((uint8_t*)"\xFFU\x04\x02\x00\x00\x02\xF8\xFFU\x03\x02\x00\x00\xFB", 15);
      break;
    case ESP_AVRC_PT_CMD_MUTE:
      Serial2.write((uint8_t*)"\xFFU\x04\x02\x00\x00\x04\xF6\xFFU\x03\x02\x00\x00\xFB", 15);
      break;
    case ESP_AVRC_PT_CMD_ROOT_MENU:
      Serial2.write((uint8_t*)"\xFFU\x05\x02\x00\x00\x00@\xB9\xFFU\x03\x02\x00\x00\xFB", 16);
      break;
    case ESP_AVRC_PT_CMD_SELECT:
      Serial2.write((uint8_t*)"\xFFU\x05\x02\x00\x00\x00\x80y\xFFU\x03\x02\x00\x00\xFB", 16);
      break;
    case ESP_AVRC_PT_CMD_UP:
      Serial2.write((uint8_t*)"\xFFU\x06\x02\x00\x00\x00\x00\x01\xF7\xFFU\x03\x02\x00\x00\xFB", 17);
      break;
    case ESP_AVRC_PT_CMD_DOWN:
      Serial2.write((uint8_t*)"\xFFU\x06\x02\x00\x00\x00\x00\x02\xF6\xFFU\x03\x02\x00\x00\xFB", 17);
      break;
    default:
      break;
  }

  Serial2.flush();
}