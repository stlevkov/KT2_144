#include <Wire.h>  // Needed for I2C communication
#include <TimeLib.h>   // Needed by CLock 
#include <DS1307RTC.h> // Clock library

int temp = 6; // define the digital temperature sensor interface
int val ; // define numeric variables val

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>             // Needed for communication with the Display

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

// Define D3, D4 for Rotary Encoder Button
#define outputA 3
#define outputB 4
#define encoderSwitch 2

int switchCounter = 0;
int counter = 0;
int aState;
int bState;
int aLastState;


// Menu variables
int pages[] = {0, 0, 0, 0, 0};  // Array menus holding Pages
int menus[] = {1, 0, 0, 0, 0};  // Array menus holding positions in the Menu List


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode (outputA, INPUT); // Define outputA to be INPUT from Rotary Encoder
  pinMode (outputB, INPUT); // Define outputB to be INPUT from Rotary Encoder
  pinMode (encoderSwitch, OUTPUT); // Define Rotary Encoder Click switch to be output
  digitalWrite(encoderSwitch, HIGH);

  // Reads the initial state of the outputA
  aLastState = digitalRead(outputA);

  // Use this initializer (uncomment) if you're using a 1.44" TFT
  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab

  // Set first background to black
  tft.fillScreen(ST7735_BLACK);


  pages[0] = 1;   // Home Page
  pages[1] = 0;  // Menu Page
  pages[2] = 0;  // Home Page + Temp ajust calibration
  pages[3] = 0;   // Home Page + Clock ajust
  pages[4] = 0;  // Home Page + Game 0 - 100 with Rotary Encoder

}

void loop() {
  // put your main code here, to run repeatedly:

  if (pages[0] == 1) {
    Serial.println("We are in Home Page");
    if (digitalRead(encoderSwitch) == LOW) {
      Serial.println("Button pressed!");
      pages[0] = 0;
      pages[1] = 1;  // We choose Menu
      pages[2] = 0;
      pages[3] = 0;
      pages[4] = 0;
      tft.fillScreen(ST7735_BLACK); // Clear the display
      drawHeader();
      drawMenuListEmpty();
      counter = 0; // Clear the counter for the Menu, so we can start the menu with 1st
      menus[0] = 1;
      menus[1] = 0;
      menus[2] = 0;
      menus[3] = 0;
      menus[4] = 0;
      drawMenuList(true, false, false, false, false);
    } else {
      // We draw Home page here:
      drawHomePage();
    }
  }
  if (pages[1] == 1) {
    Serial.println("Choosen Menu: " + (String) menus[0] + " " + (String) menus[1] + " " + (String) menus[2] + " " + (String) menus[3] + " " + (String) menus[4]);
    aState = digitalRead(outputA); // Reads the "current" state of the outputA
    bState = digitalRead(outputB);
    // If the previous and the current state of the outputA are different, that means a Pulse has occured
    if (aState != aLastState) {
      // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
      if (bState != aState) {
        counter ++;
      } else {
        counter --;
      }
      // Serial.println(counter);
      if (counter == 0) {
        menus[0] = 1;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuList(true, false, false, false, false);
      } else if (counter == 2) {
        menus[0] = 0;
        menus[1] = 1;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuList(false, true, false, false, false);
      } else if (counter == 4) {
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 1;
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuList(false, false, true, false, false);
      } else if (counter == 6) {
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 1;
        menus[4] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuList(false, false, false, true, false);
      } else if (counter == 8) {
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 1;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuList(false, false, false, false, true);
      }  else if (counter < 0) {
        menus[0] = 1;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
        counter = 0;
      } else if (counter > 8) {
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 1;
        counter = 8;
      }
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state

    // we try to get into the menu
    if (menus[0] == 1) {

    }  else if (menus[1] == 1) {

    } else if (menus[2] == 1) {

    } else if (menus[3] == 1) {

    } else if (menus[4] == 1) {
      // Serial.println("We are marked MENU 5 - RETURN TO HOME PAGE");
      if (digitalRead(encoderSwitch) == LOW) {
        pages[0] = 1;   // We choose HOME Page
        pages[1] = 0;
        pages[2] = 0;
        pages[3] = 0;
        pages[4] = 0;
        // ------ //
        counter = 0;
        menus[0] = 1;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
      }
      if (pages[0] == 1) {
        Serial.println("Return to Home Page.");
        drawSelectedMenu(false, false, false, false, true);
        delay(1000); // If user holds, will be redirected back to Menu from Home, because home checks if user press the button!
        tft.fillScreen(ST7735_BLACK); // Clear the display
      }
    }
  }
  if (pages[2] == 1) {
    Serial.println("We are in Temp Page");
    Serial.println("Drawing temp page");
  }
  if (pages[3] == 1) {
    Serial.println("We are in Clock ajust Page");
    Serial.println("Drawing clock ajust page");
  }
  if (pages[4] == 1) {
    Serial.println("We are in Game 0 - 100 with Rotary Encoder Page");
    Serial.println("Drawing Game 0 - 100 with Rotary Encoder page");
  }


}

void drawMenuListEmpty() {
  tft.setTextSize(1);
  tft.setTextColor(ST7735_CYAN);
  tft.setCursor(45, 25);
  tft.print("Temp");
  tft.setCursor(45, 45);
  tft.print("Clock");
  tft.setCursor(45, 65);
  tft.print("Game 1");
  tft.setCursor(45, 85);
  tft.print("Game 2");
  tft.setCursor(45, 105);
  tft.print("Back");
}

void drawSelectedMenu(bool menu1IsSelected, bool menu2IsSelected, bool menu3IsSelected, bool menu4IsSelected, bool menu5IsSelected) {
  if (menu1IsSelected) {
    tft.drawFastHLine(35, 20, 56, ST7735_GREEN);
    tft.drawFastVLine(35, 20, 15, ST7735_GREEN);
    tft.drawFastHLine(35, 35, 56, ST7735_GREEN);
    tft.drawFastVLine(90, 20, 15, ST7735_GREEN);
  } else if (menu2IsSelected) {
    tft.drawFastHLine(35, 40, 56, ST7735_GREEN);
    tft.drawFastVLine(35, 40, 15, ST7735_GREEN);
    tft.drawFastHLine(35, 55, 56, ST7735_GREEN);
    tft.drawFastVLine(90, 40, 15, ST7735_GREEN);
  } else if (menu3IsSelected) {
    tft.drawFastHLine(35, 60, 56, ST7735_GREEN);
    tft.drawFastVLine(35, 60, 15, ST7735_GREEN);
    tft.drawFastHLine(35, 75, 56, ST7735_GREEN);
    tft.drawFastVLine(90, 60, 15, ST7735_GREEN);
  } else if (menu4IsSelected) {
    tft.drawFastHLine(35, 80, 56, ST7735_GREEN);
    tft.drawFastVLine(35, 80, 15, ST7735_GREEN);
    tft.drawFastHLine(35, 95, 56, ST7735_GREEN);
    tft.drawFastVLine(90, 80, 15, ST7735_GREEN);
  } else if (menu5IsSelected) {
    tft.drawFastHLine(35, 100, 56, ST7735_GREEN);
    tft.drawFastVLine(35, 100, 15, ST7735_GREEN);
    tft.drawFastHLine(35, 115, 56, ST7735_GREEN);
    tft.drawFastVLine(90, 100, 15, ST7735_GREEN);
  }
}

void drawMenuList(bool menu1IsSelected, bool menu2IsSelected, bool menu3IsSelected, bool menu4IsSelected, bool menu5IsSelected) {
  if (menu1IsSelected) {
    tft.drawFastHLine(35, 20, 56, ST7735_CYAN);
    tft.drawFastVLine(35, 20, 15, ST7735_CYAN);
    tft.drawFastHLine(35, 35, 56, ST7735_CYAN);
    tft.drawFastVLine(90, 20, 15, ST7735_CYAN);
  } else if (menu2IsSelected) {
    tft.drawFastHLine(35, 40, 56, ST7735_CYAN);
    tft.drawFastVLine(35, 40, 15, ST7735_CYAN);
    tft.drawFastHLine(35, 55, 56, ST7735_CYAN);
    tft.drawFastVLine(90, 40, 15, ST7735_CYAN);
  } else if (menu3IsSelected) {
    tft.drawFastHLine(35, 60, 56, ST7735_CYAN);
    tft.drawFastVLine(35, 60, 15, ST7735_CYAN);
    tft.drawFastHLine(35, 75, 56, ST7735_CYAN);
    tft.drawFastVLine(90, 60, 15, ST7735_CYAN);
  } else if (menu4IsSelected) {
    tft.drawFastHLine(35, 80, 56, ST7735_CYAN);
    tft.drawFastVLine(35, 80, 15, ST7735_CYAN);
    tft.drawFastHLine(35, 95, 56, ST7735_CYAN);
    tft.drawFastVLine(90, 80, 15, ST7735_CYAN);
  } else if (menu5IsSelected) {
    tft.drawFastHLine(35, 100, 56, ST7735_CYAN);
    tft.drawFastVLine(35, 100, 15, ST7735_CYAN);
    tft.drawFastHLine(35, 115, 56, ST7735_CYAN);
    tft.drawFastVLine(90, 100, 15, ST7735_CYAN);
  }
}

void drawHeader() {
  // -----------------------BATTERY STATUS ICON----------------------------------
  tft.setTextSize(1);
  if (readVcc() < 3788) {                                    // 3788 ~ 3.69V on the Battery
    batteryStatusImage("", 105, 2,  5, true);            // we call the function with "" because if expired=true we set RED color instead
  } else {
    batteryStatusImage(ST7735_WHITE, 105, 2, map(readVcc(), 3788, 4311, 1, 16), false);
  }
  // ------------------------CHARGING ICON BATTERY PERCENTAGE---------------------------------
  if (!true) {  // TODO hook up wire to the Charging module

  } else {
    tft.setTextColor(ST7735_CYAN);
    if (readVcc() > 4320) {
      tft.setCursor(50, 3);
      tft.print("Charg.");
    } else {
      tft.setCursor(82, 3);
      tft.print(map(readVcc(), 3788, 4311, 0, 100));
      tft.print("%");
    }
  }
  // -----------------------DATE AND TIME ----------------------------------
  tft.setCursor(2, 3);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE);
  tft.print(getClock());  // Print Clock
  // Test print Clock on Serial Monitor TODO Delete it
}

void drawHomePage() {
  drawHeader();
  // --------------------------TERMOMETER VALUES-------------------------------
  temp = 24;                           // TODO Change with the value from the sensor
  // tft.fillScreen(ST7735_BLACK);
  tft.setCursor(40, 40);
  tft.setTextSize(7);
  printTermometerValues(temp);
  // -----------------------TERMOMETER STATUS ICON----------------------------------
  termometerStatusImage(ST7735_WHITE, ST7735_RED, 10, 10, map(temp, 1, 60, 1 , 29));
}

void printTermometerValues(int temp) {
  if (temp >= 20 && temp <= 22) {
    tft.setTextColor(ST7735_GREEN);
    tft.print(temp);
  } else if (temp > 24) {
    tft.setTextColor(ST7735_RED);
    tft.print(temp);
  } else if (temp < 20) {
    tft.setTextColor(ST7735_BLUE);
    tft.print(temp);
  } else if (temp > 22 && temp <= 24) {
    tft.setTextColor(ST7735_YELLOW);
    tft.print(temp);
  }
}

void termometerStatusImage(uint16_t color1, uint16_t color2, uint16_t x, uint16_t y, uint16_t fillUp) { // fillUp can be from 1 to 29

  tft.drawFastHLine(15, 40, 5, color1);
  tft.drawFastVLine(14, 41, 30, color1);
  tft.drawFastVLine(20, 41, 30, color1);
  tft.drawCircle(17, 80, 9, color1);
  tft.fillCircle(17, 80, 8, color2);

  // fill up
  for (int i = 0; i <= fillUp; i++) {
    tft.drawFastHLine(15, 70 - i, 5, color2);
  }

}

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
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  // return result; // Vcc in millivolts
  float v = result;
  return v;
}

void batteryStatusImage(uint16_t color1, uint16_t x, uint16_t y, uint16_t fillUp, boolean expired) { // fillUp can be from 1 to 16
  if (expired) {
    tft.drawFastHLine(x, 2, 20, ST7735_RED);                   // Draw top Corner line of the battery icon
    tft.drawFastHLine(x, y + 8, 20, ST7735_RED);               // Draw bottom Corner line of the battery icon
    tft.drawFastVLine(x, 2, 9, ST7735_RED);                    // Draw left Corner line of the battery icon
    tft.drawFastVLine(x + 20, 2, 9, ST7735_RED);               // Draw right Corner line of the battery icon
    tft.drawFastVLine(x - 1, y + 2, 5, ST7735_RED);            // Draw the left positive cathode of the battery #1
    tft.drawFastVLine(x - 2, y + 2, 5, ST7735_RED);            // Draw the left positive cathode of the battery #2
  } else {
    tft.drawFastHLine(x, 2, 20, color1);                       // Draw top Corner line of the battery icon
    tft.drawFastHLine(x, y + 8, 20, color1);                   // Draw bottom Corner line of the battery icon
    tft.drawFastVLine(x, 2, 9, color1);                        // Draw left Corner line of the battery icon
    tft.drawFastVLine(x + 20, 2, 9, color1);                   // Draw right Corner line of the battery icon
    tft.drawFastVLine(x - 1, y + 2, 5, color1);                // Draw the left positive cathode of the battery #1
    tft.drawFastVLine(x - 2, y + 2, 5, color1);                // Draw the left positive cathode of the battery #2

    // filling up
    for (int i = 0; i <= fillUp; i++) {
      tft.drawFastVLine((x + 18) - i, y + 2, 5, ST7735_GREEN);
    }
  }
}

String getClock() {
  String msg = "";
  tmElements_t tm;

  if (RTC.read(tm)) {
    msg += tm.Hour;
    msg += ":";
    msg += tm.Minute;
    /*
      Serial.print("Ok, Time = ");
      print2digits(tm.Hour);
      Serial.write(':');
      print2digits(tm.Minute);
      Serial.write(':');
      print2digits(tm.Second);
      Serial.print(", Date (D/M/Y) = ");
      Serial.print(tm.Day);
      Serial.write('/');
      Serial.print(tm.Month);
      Serial.write('/');
      Serial.print(tmYearToCalendar(tm.Year));
      Serial.println();
    */
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }
  return msg;

}

