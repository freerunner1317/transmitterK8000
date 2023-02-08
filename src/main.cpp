/*
   RadioLib SX127x Transmit with Interrupts Example

   This example transmits LoRa packets with one second delays
   between them. Each packet contains up to 256 bytes
   of data, in the form of:
    - Arduino String
    - null-terminated char array (C-string)
    - arbitrary binary data (byte array)

   Other modules from SX127x/RFM9x family can also be used.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx127xrfm9x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>

// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
SX1278 radio = new Module(10, 2, 9, 3);
#define PARSE_AMOUNT 5

int intData[PARSE_AMOUNT];     // массив численных значений после парсинга
boolean recievedFlag;
boolean getStarted;
byte index = 0;
String string_data = "";


const size_t lenMessage = 3;
byte byteArr[lenMessage] = {};

// or using RadioShield
// https://github.com/jgromes/RadioShield
//SX1278 radio = RadioShield.ModuleA;

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;
void setFlag(void);
void parsing();

void setup() {
  Serial.begin(115200);

  // initialize SX1278 with default settings
  Serial.print(F("Initializing ... "));
  // carrier frequency:                   434.0 MHz
  // bandwidth:                           125.0 kHz
  // spreading factor:                    9
  // coding rate:                         7
  // sync word:                           0x12
  // output power:                        17 dBm
  // current limit:                       100 mA
  // preamble length:                     8 symbols
  // amplifier gain:                      0 (automatic gain control)
  int state = radio.begin();

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
  if (radio.setBandwidth(500.0) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    Serial.println(F("Selected bandwidth is invalid for this module!"));
    while (true);
  }

  // set spreading factor to 10
  if (radio.setSpreadingFactor(10) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    Serial.println(F("Selected spreading factor is invalid for this module!"));
    while (true);
  }

  if (radio.setGain(1) == RADIOLIB_ERR_INVALID_GAIN) {
    Serial.println(F("Selected gain is invalid for this module!"));
    while (true);
  }
  if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION ){
    Serial.println(F("Selected CRC is invalid for this module!"));
    while (true);
  }
  if (radio.setSyncWord(0xDF) != RADIOLIB_ERR_NONE) {
    Serial.println(F("Unable to set sync word!"));
    while (true);
  }

  // set the function that will be called
  // when packet transmission is finished
  radio.setDio0Action(setFlag);

  // start transmitting the first packet
  Serial.print(F("[SX1278] Sending first packet ... "));

  // you can transmit C-string or Arduino string up to
  // 256 characters long
  transmissionState = radio.startTransmit("Hello World!");

  // you can also transmit byte array up to 256 bytes long
  /*
    byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                      0x89, 0xAB, 0xCD, 0xEF};
    state = radio.startTransmit(byteArr, 8);
  */
}

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;
void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }
  transmittedFlag = true;
}

void loop() {
  parsing();
  if ((transmittedFlag) && (recievedFlag)) {
    enableInterrupt = false;
    transmittedFlag = false;

    if (transmissionState == RADIOLIB_ERR_NONE) {
      //Serial.println(F("\ntr fin!"));
    } else {
      Serial.print(F("failed, code "));
      Serial.println(transmissionState);
    }
    //Serial.print(F("\nSend "));
    Serial.print("Trans:       ");
    // you can transmit C-string or Arduino string up to
    // 256 characters long
    for (byte i = 0; i < 3; i++) { // выводим элементы массива      
      char text[2] = "";
      sprintf(text, "%02X", byteArr[i]);
      Serial.print(text);
      //Serial.print((char)byteArr[i]);
    } Serial.println(); 

    //transmissionState = radio.startTransmit(string_data);
    
    
    transmissionState = radio.startTransmit(byteArr, lenMessage);
    
    recievedFlag = false;
    enableInterrupt = true;
  }
}

void parsing() {
  //index = 0;
  if (Serial.available() > 0) {
    byte incomingByte = Serial.read();        // обязательно ЧИТАЕМ входящий символ

    char text[2] = "";
    sprintf(text, "%02X", incomingByte);
    Serial.print(text);
    
    if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)     
      //string_data += (char)incomingByte;      
      byteArr[index] = incomingByte;
      index++;
    }
    // if (byteArr[1] == 0xFC){
    //   getStarted = false;                     // сброс
    //   recievedFlag = true;                    // флаг на принятие   
    //   string_data += (char)incomingByte;
    //   byteArr[1] = incomingByte;
    //   Serial.println("       Parsed");
    // }
    if ((incomingByte == 0xF3) && (index == 0)) {                // если это $
      getStarted = true;                      // поднимаем флаг, что можно парсить
      //string_data = "";
      //string_data += (char)incomingByte;
      index = 0;
      //byteArr[0] = incomingByte;
    }
    if (index >= 4) {                // если таки приняли ; - конец парсинга
      getStarted = false;                     // сброс
      recievedFlag = true;                    // флаг на принятие   
      //string_data += (char)incomingByte;
      //byteArr[index] = incomingByte;
      index = 0;
      Serial.println("       Parsed");
    }
  }
}

