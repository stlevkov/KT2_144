
  int temp = 6; // define the digital temperature sensor interface
int val ; // define numeric variables val


#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>


// Define 1.44" display pins
#define TFT_CS     10
#define TFT_RST    8  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to -1!
#define TFT_DC     9

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);


float p = 3.1415926;

// --------------------------------------------------------------------------------------------------------------------



void setup() {
  Serial.begin(9600);
  
  pinMode (temp, INPUT) ;// define digital temperature sensor output interface
  Serial.print("Hello! ST7735 TFT Test");

  // Use this initializer if you're using a 1.8" TFT
  //tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  // Use this initializer (uncomment) if you're using a 1.44" TFT
  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab

  // Use this initializer (uncomment) if you're using a 0.96" 180x60 TFT
  //tft.initR(INITR_MINI160x80);   // initialize a ST7735S chip, mini display

  Serial.println("Initialized");

  uint16_t time = millis();
  tft.fillScreen(ST7735_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

}

void loop() {

  
//  tft.fillRect(77, 20 , 50, 40, ST7735_BLACK);
  tft.setCursor(2, 4);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_MAGENTA);
  tft.print(readVcc() - 150); // 4266
  tft.setTextColor(ST7735_GREEN);
  tft.print("mV");
 
  // ---------------------------------------------------------
  temp = 10;
 
  tft.setCursor(40, 40);
  tft.setTextSize(7);

  // analogRead(temp)
   
  
  
  if (temp >= 20 && temp <= 22){
    tft.setTextColor(ST7735_GREEN);
    tft.print(temp);
  } else if (temp > 24){
    tft.setTextColor(ST7735_RED);
    tft.print(temp);
  } else if (temp < 20) {
    tft.setTextColor(ST7735_BLUE);
    tft.print(temp);
  } else if (temp > 22 && temp <= 24){
    tft.setTextColor(ST7735_YELLOW);
    tft.print(temp);
  }
  termometerStatusImage(ST7735_WHITE, ST7735_RED, 10, 10, map(temp, 1, 60, 1 , 29)); 
  

  // ---------------------------------------------------------
  tft.setTextSize(1);
  if (readVcc() < 3788) {                                    // 3788 ~ 3.69V on the Battery
      batteryStatusImage(ST7735_GREEN,105, 2,  5, true);
  } else{
      batteryStatusImage(ST7735_GREEN,105, 2, map(readVcc(), 4311, 3788, 16, 1), false);  
  }
  
  // ---------------------------------------------------------
  if (readVcc() > 4311) {
    tft.setTextColor(ST7735_BLUE);
    tft.setCursor(50, 4);
    tft.print("Charging");
  }

   // ---------------------------------------------------------

  
    
   
}

void termometerStatusImage(uint16_t color1, uint16_t color2, uint16_t x, uint16_t y, uint16_t fillUp){ // fillUp can be from 1 to 29
    
   tft.drawFastHLine(15, 40, 5, color1);  
   tft.drawFastVLine(14, 41, 30, color1);
   tft.drawFastVLine(20, 41, 30, color1);
   tft.drawCircle(17, 80, 9, color1);
   tft.fillCircle(17, 80, 8, color2);

   // fill up 
   for (int i = 0; i <= fillUp; i++){
      tft.drawFastHLine(15, 70 - i, 5,color2); 
   } 
  
}

void batteryStatusImage(uint16_t color1,uint16_t x, uint16_t y, uint16_t fillUp, boolean expired) { // fillUp can be from 1 to 16
    if(expired) {
       tft.drawFastHLine(x, 2, 20, ST7735_RED);  
       tft.drawFastHLine(x, y + 8, 20, ST7735_RED);
       tft.drawFastVLine(x, 2, 9, ST7735_RED);
       tft.drawFastVLine(x + 20, 2, 9, ST7735_RED);
       tft.drawFastVLine(x -1, y + 2, 5, ST7735_RED);
       tft.drawFastVLine(x -2, y + 2, 5, ST7735_RED);
    } else {
       tft.drawFastHLine(x, 2, 20, color1);  
       tft.drawFastHLine(x, y + 8, 20, color1);
       tft.drawFastVLine(x, 2, 9, color1);
       tft.drawFastVLine(x + 20, 2, 9, color1);
       tft.drawFastVLine(x -1, y + 2, 5, color1);
       tft.drawFastVLine(x -2, y + 2, 5, color1);
       
       // filling up
       for (int i = 0;i <= fillUp; i++){
         tft.drawFastVLine((x + 18) - i, y + 2, 5, color1); 
       }
    }
}

void batteryExpiredStatusImage(uint16_t color1,uint16_t x, uint16_t y) {   
   
}
/*
double GetTemperature(int v){
  double Temp;
  Temp = log(10000.0 / (1024.0 / v - 1)); 
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp))* Temp);
  Temp = Temp - 273.15; // Convert Kelvin to Celcius
  return Temp;
  
}
*/

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
 // return result; // Vcc in millivolts
  float v = result;
  return v;
}


