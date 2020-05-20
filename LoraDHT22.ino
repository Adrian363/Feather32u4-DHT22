




/* Quand on flash re-flash une carte, il ne faut pas oublier de réhinialiser le compteur de frames sur TTN sinon les données ne sont pas envoyées.*/

/*#define VBATPIN A9
   
float measuredvbat = analogRead(VBATPIN);
measuredvbat *= 2;    // we divided by 2, so multiply back
measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
measuredvbat /= 1024; // convert to voltage
Serial.print("VBat: " ); Serial.println(measuredvbat);

https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/power-management



*/

#include <RH_RF95.h>
#include "DHT.h"
#include <TinyLoRa.h>
#include <SPI.h>
#include "LowPower.h"

#define DHTPIN 12
#define DHTTYPE DHT22   // define type of sensor

DHT dht(DHTPIN, DHTTYPE); //define properties of sensor

RH_RF95 radio;

// The things Network configuration 


// Network Session Key (MSB)
uint8_t NwkSkey[16] = { 0x77, 0x8E, 0x00, 0xA1, 0x58, 0xB2, 0xC0, 0x06, 0x87, 0x92, 0xD1, 0x5F, 0x3C, 0x1C, 0x7E, 0xA3 };

// Application Session Key (MSB)
uint8_t AppSkey[16] = { 0x3D, 0x25, 0x95, 0xE9, 0x64, 0x26, 0x9F, 0xC1, 0x46, 0xD7, 0xEF, 0x8C, 0x6C, 0x6B, 0xE9, 0x99 };

// Device Address (MSB)
uint8_t DevAddr[4] = { 0x26, 0x02, 0x1F, 0xCB };

// Pinout for Adafruit Feather 32u4 LoRa
TinyLoRa lora = TinyLoRa(7, 8, 4);


float hum;
float temp;

// Define LoRa Data type and size, max 4 octect in our case
// temp and hum *100, even if the value are 100*100, only 16 bits are necessary for each, so 2 bytes, or 4 for the 2 values

unsigned char loraData[12];
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  /* While(! Serial) pas utile car on attend le début du serial , or lorsque l'objet fonctionne sur batterie, on ouvre pas de sérial*/  
 /* while (! Serial); */
  dht.begin(); //initiate the communication with sensor

  
   // Initialize pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  
   // Initialize LoRa
  Serial.print("Starting LoRa...");
  
  // define multi-channel sending
  lora.setChannel(MULTI);
  // set datarate, Spreading factor 7 and Bandwidth 125
  lora.setDatarate(SF7BW125);
  if(!lora.begin())
  {
    Serial.println("Failed");
    Serial.println("Check your radio");
    while(true);
  }
   // Optional set transmit power. If not set default is +17 dBm.
  // Valid options are: -80, 1 to 17, 20 (dBm).
  // For safe operation in 20dBm: your antenna must be 3:1 VWSR or better
  // and respect the 1% duty cycle.

  // lora.setPower(17);

  Serial.println("OK");
   
}

void loop() {

  delay(4000); // Delais minimum de lecture du capteur de température de 2 secondes

  Serial.println("Serial OK");
  // put your main code here, to run repeatedly:
  hum = dht.readHumidity();  //save the value of humidity to hum variable
  temp = dht.readTemperature(); // save the value of temperature to temp variable             
  
  Serial.print("Humidity is: ");                
  Serial.print(hum);
  Serial.print("% | ");
  Serial.print("Temperature is: ");
  Serial.print(temp);
  Serial.println(" degrees of Celsius");

  int16_t tempInt = round(temp * 100);
  int16_t humidInt = round(hum * 100);

  float lat=50.225893;
  float lon=-66.356457;
  int32_t latitude= lat*1000000;
  int32_t longitude= lon*1000000;

  loraData[0]=highByte(tempInt);
  loraData[1]=lowByte(tempInt);
  loraData[2]=highByte(humidInt);
  loraData[3]=lowByte(humidInt);

  loraData[4] = latitude;
  loraData[5] = latitude >> 8;
  loraData[6] = latitude >> 16;
  loraData[7] = latitude >> 24;


  loraData[8] = longitude;
  loraData[9] = longitude >> 8;
  loraData[10] = longitude >> 16;
  loraData[11] = longitude >> 24;
  
   Serial.println("Sending LoRa Data...");
  lora.sendData(loraData, sizeof(loraData), lora.frameCounter);
   Serial.print("Frame Counter: ");Serial.println(lora.frameCounter);
  lora.frameCounter++; /*Update le nombre de trames reçu sur TTN.*/
 
  //ATmega32U4 Shut ADC and BOD

 
  Serial.println(SLEEP_8S);
  USBDevice.detach();

  // Disable USB clock 
    USBCON |= _BV(FRZCLK);
    // Disable USB PLL
    PLLCSR &= ~_BV(PLLE); 
    // Disable USB
    USBCON &= ~_BV(USBE); 

    int timesleep=1800;
    radio.sleep();
  for (int i=0; i<(timesleep/8); i++){
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  
  USBDevice.attach(); 
  
  
}
