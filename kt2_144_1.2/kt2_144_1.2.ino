#include <Wire.h>                   // Needed for I2C communication
#include <TimeLib.h>                // Needed by CLock 
#include <DS1307RTC.h>              // Clock library

int temp = 0;                       // Holds the Temp variable for reading from DHT Sensor
int hum = 0;                       // Holds the hum var
int lastTempState = 0;              // define the last state of the temp measurement
int lasthumState = 0;              // define the last state of the hum measurement
int tempCalibration = 0;            // define the calibration offset between (+ and -)
int lastTempCalibrationState = 1;   // define calibration state for the calibration of the Temp value

#include <Adafruit_GFX.h>           // Core graphics library
#include <Adafruit_ST7735.h>        // Hardware-specific library
#include <SPI.h>                    // Needed for communication with the Display

#include "DHT.h"                    // DHT22 sensor library

#define DHTPIN 6                    // what digital pin we're connected to
#define DHTTYPE DHT22               // DHT 22  (AM2302), AM2321

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

// --------------- ST7735 chip --------------- //
// Define 1.44" display pins
#define TFT_CS     10
#define TFT_RST    8               // you can also connect this to the Arduino reset
#define TFT_DC     9

// Color definitions for using tft library HEXs instead of ST7735_BLUE for example
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF


#define FIRMWARE_VERSION "1.2"
#define COUNTRY "Bulgaria"
#define CODE_LOCATION "GitHub"
#define CODE_ORIGIN "/stlevkov"
#define CODE_REPO "/KT2_144"

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

float p = 3.1415926;    // define pi first 7 numbers

// Define D3, D4 for Rotary Encoder Button
#define outputA 3
#define outputB 4
#define encoderSwitch 2

int switchCounter = 0;
int counter = 0;
int hoursCounter = 0;                   // Needed by Clock Adjustion Page for getting EVEN 0,2,4 untill 24 for hours
int minutesCounter = 0;                 // Needed by Clock Adjustion Page for getting EVEN 0,2,4 untill 60 for minutes

int aState;                             // Rotary Encoder start position
int bState;                             // Rotary Encoder end position
int aLastState;                         // Saving last state of the encoder

int vcc = 0;                            // Reading the voltage internally in milliVolts
int vccLastState = 0;                   // Saving the last state of the reading for clearing the display when the voltage change

// Used by Clock Adjustion Page for selecting the current position and colored it by defined color in the methods bellow
int selectedClockIndex = 0;             // Posibble values for Clock Adjust Page: 0 - hours, 1 - minutes, 2 - day, 3 - month, 4 - year

//Define Days for Clock
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
tmElements_t tm;                        // The importent thing here is this Class for RTC lib

int hourLastState = 0;
int minutesLastState = 0;

int hours;
int minutes;
int seconds;

// Menu variables
int pages[] = {0, 0, 0, 0, 0};          // Array menus holding Pages
int menus[] = {1, 0, 0, 0, 0};          // Array menus holding positions in the Menu List


void setup() {
  // Boot up:
  Serial.begin(115200);
  pinMode (outputA, INPUT);             // Define outputA to be INPUT from Rotary Encoder
  pinMode (outputB, INPUT);             // Define outputB to be INPUT from Rotary Encoder
  pinMode (encoderSwitch, OUTPUT);      // Define Rotary Encoder Click switch to be output
  digitalWrite(encoderSwitch, HIGH);

  // Reads the initial state of the outputA
  aLastState = digitalRead(outputA);

  // Use this initializer (uncomment) if you're using a 1.44" TFT
  tft.initR(INITR_144GREENTAB);         // initialize a ST7735S chip, black tab

  // Set first background to black
  tft.fillScreen(ST7735_BLACK);


  // This info is for Boot up, showing the last firmware compile date & time - there is 3 sec delay in here
  printFirmwareInfo();

  pages[0] = 1;  // Home Page
  pages[1] = 0;  // Menu Page
  pages[2] = 0;  // Home Page + Temp ajust calibration
  pages[3] = 0;  // Home Page + Clock ajust
  pages[4] = 0;  // Home Page + hum conf
  pages[5] = 0;  // Home Page + About
  // -------------------------------------- +++++   DHT22     +++++------------------------------------ //
  // Serial.println("DHT22 begin()!");
  dht.begin();
  // -------------------------------------- +++++   DHT22     +++++------------------------------------ //
}

void loop() {
  // TODO ALWAYS GET THE NEW VALUES FOR - TEMP + CALIBRATION


  // -------------------------------------- HOME PAGE ------------------------------------ //
  if (pages[0] == 1) {

    Serial.println("We are in Home Page");
    temp = getDhtData("temp") + tempCalibration;
    hum = getDhtData("hum");

    if (digitalRead(encoderSwitch) == LOW) {
      Serial.println("Button pressed!");
      pages[0] = 0;
      pages[1] = 1;  // We choose Menu
      pages[2] = 0;
      pages[3] = 0;
      pages[4] = 0;
      pages[5] = 0;
      tft.fillScreen(ST7735_BLACK); // Clear the display
      drawHeader();
      drawMenuListEmpty();
      counter = 0;                           // Clear the counter for the Menu, so we can start the menu with 1st
      menus[0] = 1;
      menus[1] = 0;
      menus[2] = 0;
      menus[3] = 0;
      menus[4] = 0;
      drawMenuListButtons( false, true, false, false, false, false);
      delay(1000); // Wait before show Menu, because if user little holds the button activate the first menu automaticly
    } else {
      // We draw Home page here:
      drawHomePage();
    }
  }

  // -------------------------------------- MENU PAGE ------------------------------------ //
  if (pages[1] == 1) {
    // CHOOSEN IS MENU PAGE WE TRY TO GET THE ROTARY ENCODER INFORMATION
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
      //  The counter is from 0, 2, 4, 6, 8
      if (counter == 0) {  // WE MARK MENU 1 - TEMP
        menus[0] = 1;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, true, false, false, false, false);
      } else if (counter == 2) {     //  WE MARK MENU 2 CLOCK
        menus[0] = 0;
        menus[1] = 1;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, false, true, false, false, false);
      } else if (counter == 4) {    // WE MARK MENU 3 hum
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 1;
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, false, false, true, false, false);
      } else if (counter == 6) {     // WE MARK MENU 4 About
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 1;
        menus[4] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, false, false, false, true, false);
      } else if (counter == 8) {      // WE MARK MENU 5 BACK
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 1;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, false, false, false, false, true);
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
      // Serial.println("We are marked MENU 1 - GO TO TEMP CALIBRATION PAGE");
      if (digitalRead(encoderSwitch) == LOW) {
        pages[0] = 0;
        pages[1] = 0;
        pages[2] = 1;  // We choose TEMP Calibration Page
        pages[3] = 0;
        pages[4] = 0;
        pages[5] = 0;
        // ------ //
        counter = 0;
        menus[0] = 1;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
      }
      if (pages[2] == 1) {
        Serial.println("Go to Temp Calibration Page.");
        drawMenuListButtons(true , true, false, false, false, false); // Draw selected Menu 1 for GREEN border
        delay(1000); // If user holds, will be redirected back to Menu from Home, because home checks if user press the button!
        counter = 0;
        temp = getDhtData("temp");
        tempCalibration = 0;
        lastTempCalibrationState = 2;
        drawTempPage();
      }

    }  else if (menus[1] == 1) {
      // Serial.println("We are marked MENU 2 - GO TO CLOCK PAGE");
      if (digitalRead(encoderSwitch) == LOW) {
        pages[0] = 0;
        pages[1] = 0;
        pages[2] = 0;
        pages[3] = 1;   // We choose CLOCK Calibration Page
        pages[4] = 0;
        pages[5] = 0;
        // ------ //
        counter = 0;
        menus[0] = 1;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
      }
      if (pages[3] == 1) {
        Serial.println("Go to CLOCK Calibration Page.");
        drawMenuListButtons(true, false, true, false, false, false); // Draw selected Menu 2 for GREEN border
        delay(1000); // If user holds, will be redirected back to Menu from Home, because home checks if user press the button!
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        tft.setCursor(15, 20);
        tft.setTextSize(1);
        tft.setTextColor(ST7735_YELLOW);
        tft.print("DATE & TIME PAGE");
        tft.setCursor(15, 30);
        tft.setTextSize(1);
        tft.setTextColor(ST7735_BLUE);
        tft.print("Rotate to adjust");
        tft.setCursor(20, 100);
        tft.setTextColor(ST7735_WHITE);
        tft.print("Click to save.");

        tmElements_t tm;                         // Initialize Time before the loop in the page
        RTC.read(tm);                            // Because when adjusting - the time keep changing

        hours = tm.Hour;                         // Prepare the global variables for adjusting
        minutes = tm.Minute;                     // Prepare the global variables for adjusting

        hoursCounter = hours;                    // When Adjust - starts from where the current time is
        minutesCounter = minutes;                // When Adjust - starts from where the current time is
        selectedClockIndex = 0;                  // We select the hours to adjust first when reaching the clock page
        // Display first init before the first Rotation of the button to show the clock
        tft.setCursor(20, 50);
        tft.setTextSize(3);
        tft.setTextColor(ST7735_YELLOW);
        tft.print(hours);

        tft.setTextColor(ST7735_WHITE);
        tft.setCursor(70, 50);
        tft.print(minutes);

        tft.setTextColor(ST7735_WHITE);
        tft.setCursor(55, 50);
        tft.print(":");

      }

    } else if (menus[2] == 1) {
      // Serial.println("We are marked MENU 3 - GO TO hum PAGE");
      if (digitalRead(encoderSwitch) == LOW) {
        pages[0] = 0;
        pages[1] = 0;
        pages[2] = 0;
        pages[3] = 0;
        pages[4] = 1;   // We choose hum Page
        // ------ //
        counter = 0;
        menus[0] = 1;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
      }
      if (pages[4] == 1) {
        Serial.println("Go to hum Page.");
        drawMenuListButtons(true, false, false, true, false, false); // Draw selected Menu 2 for GREEN border
        delay(1000); // If user holds, will be redirected back to Menu from Home, because home checks if user press the button!
        tft.fillScreen(ST7735_BLACK); // Clear the display
      }

    } else if (menus[3] == 1) {
      // Serial.println("We are marked MENU 4 - GO TO About PAGE;
      if (digitalRead(encoderSwitch) == LOW) {
        pages[0] = 0;
        pages[1] = 0;
        pages[2] = 0;
        pages[3] = 0;
        pages[4] = 0;
        pages[5] = 0;
        pages[5] = 1;  // We choose About Page
        // ------ //
        counter = 0;
        menus[0] = 1;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
      }
      if (pages[5] == 1) {
        Serial.println("Go to About Page.");
        drawMenuListButtons(true,false, false, false, true, false); // Draw selected Menu 2 for GREEN border
        delay(1000); // If user holds, will be redirected back to Menu from Home, because home checks if user press the button!
        tft.fillScreen(ST7735_BLACK); // Clear the display
      }

    } else if (menus[4] == 1) {
      // Serial.println("We are marked MENU 5 - RETURN TO HOME PAGE");
      if (digitalRead(encoderSwitch) == LOW) {
        pages[0] = 1;   // We choose HOME Page
        pages[1] = 0;
        pages[2] = 0;
        pages[3] = 0;
        pages[4] = 0;
        pages[5] = 0;
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
        drawMenuListButtons(true, false, false, false, false, true);
        delay(1000); // If user holds, will be redirected back to Menu from Home, because home checks if user press the button!
        tft.fillScreen(ST7735_BLACK); // Clear the display

      }
    }
  }

  // -------------------------------------- TEMP PAGE ------------------------------------ //
  if (pages[2] == 1) {
    // Serial.println("We are in Temp Page");
    // -----------------------ROTARY ENCODER LOGIC----------------------------------
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
      //  The counter is from 0, 2, 4, 6, 8
      if (counter == 0) {  // offset is equals to 0
        tempCalibration = 0;
      } else if (counter == 2) {
        tempCalibration = 1;
      } else if (counter == 4) {
        tempCalibration = 2;
      } else if (counter == 6) {
        tempCalibration = 3;
      } else if (counter == 8) {
        tempCalibration = 4;
      } else if (counter > 8) {
        counter = 8;
      } else if (counter < 0) {
        if (counter == -2) {
          tempCalibration = -1;
        } else if (counter == -4) {
          tempCalibration = -2;
        } else if (counter == -6) {
          tempCalibration = -3;
        } else if (counter == -8) {
          tempCalibration = -4;
        } else if (counter < -8) {
          counter = -8;
        }
      }
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state
    drawTempPage();
  }

  // -------------------------------------- CLOCK PAGE ------------------------------------ //
  if (pages[3] == 1) {
    Serial.println("We are in Clock ajust Page");
    Serial.print("selectedClockIndex: ");
    Serial.print(selectedClockIndex);
    Serial.println("");
    // -----------------------ROTARY ENCODER LOGIC----------------------------------
    aState = digitalRead(outputA); // Reads the "current" state of the outputA
    bState = digitalRead(outputB);

    // We first read the current index - See possible values defined before setup()
    if (selectedClockIndex == 0) {    // We adjust hours
      // If the previous and the current state of the outputA are different, that means a Pulse has occured

      if (aState != aLastState) {
        // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
        if (bState != aState) {
          if (hoursCounter >= 23) {
            hoursCounter = -1;
          } else {
            counter ++;
          }
          if (((counter & 1) == 0)) {
            hoursCounter ++;
            hours = hoursCounter;

            tft.fillScreen(ST7735_BLACK); // Clear the display
            drawHeader();
            tft.setCursor(15, 20);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_YELLOW);
            tft.print("DATE & TIME PAGE");
            tft.setCursor(15, 30);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_BLUE);
            tft.print("Rotate to adjust");
            tft.setCursor(20, 100);
            tft.setTextColor(ST7735_WHITE);
            tft.print("Click to save.");

            tft.setCursor(20, 50);
            tft.setTextSize(3);
            tft.setTextColor(ST7735_YELLOW);
            tft.print(hours);

            tft.setTextColor(ST7735_WHITE);
            tft.setCursor(70, 50);
            tft.print(minutes);

            tft.setTextColor(ST7735_WHITE);
            tft.setCursor(55, 50);
            tft.print(":");
          }
        } else {
          if (hoursCounter <= 0) {
            hoursCounter = 24;
          } else {
            counter --;
          }
          if (((counter & 1) == 0)) {
            hoursCounter --;
            hours = hoursCounter;

            tft.fillScreen(ST7735_BLACK); // Clear the display
            drawHeader();
            tft.setCursor(15, 20);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_YELLOW);
            tft.print("DATE & TIME PAGE");
            tft.setCursor(15, 30);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_BLUE);
            tft.print("Rotate to adjust");
            tft.setCursor(20, 100);
            tft.setTextColor(ST7735_WHITE);
            tft.print("Click to save.");

            tft.setCursor(20, 50);
            tft.setTextSize(3);
            tft.setTextColor(ST7735_YELLOW);
            tft.print(hours);

            tft.setTextColor(ST7735_WHITE);
            tft.setCursor(70, 50);
            tft.print(minutes);

            tft.setTextColor(ST7735_WHITE);
            tft.setCursor(55, 50);
            tft.print(":");

          }

        }
      }
      Serial.println(counter);
      Serial.println(hoursCounter);
      // Click to save the current position
      if (digitalRead(encoderSwitch) == LOW) {
        counter = 0;
        hoursCounter = 0;
        selectedClockIndex ++;                                      // going to adjust minutes
        Serial.println("selectedClockIndex changed!");

        // Change colors of the numbers - Color minutes
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();
        tft.setCursor(15, 20);
        tft.setTextSize(1);
        tft.setTextColor(ST7735_YELLOW);
        tft.print("DATE & TIME PAGE");
        tft.setCursor(15, 30);
        tft.setTextSize(1);
        tft.setTextColor(ST7735_BLUE);
        tft.print("Rotate to adjust");
        tft.setCursor(20, 100);
        tft.setTextColor(ST7735_WHITE);
        tft.print("Click to save.");

        tft.setCursor(20, 50);
        tft.setTextSize(3);
        tft.setTextColor(ST7735_WHITE);              // Here   Colored color for choosen is Yellow
        tft.print(hours);

        tft.setTextColor(ST7735_YELLOW);            // Here    Colored color for choosen is Yellow
        tft.setCursor(70, 50);
        tft.print(minutes);

        tft.setTextColor(ST7735_WHITE);
        tft.setCursor(55, 50);
        tft.print(":");
        delay(1000);
      }

    } else if (selectedClockIndex == 1) {  // We adjust minutes


      // If the previous and the current state of the outputA are different, that means a Pulse has occured
      if (aState != aLastState) {
        // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
        if (bState != aState) {
          if (minutesCounter >= 59) {
            minutesCounter = -1;
          } else {
            counter ++;
          }
          if (((counter & 1) == 0)) {
            minutesCounter ++;
            minutes = minutesCounter;

            tft.fillScreen(ST7735_BLACK); // Clear the display
            drawHeader();
            tft.setCursor(15, 20);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_YELLOW);
            tft.print("DATE & TIME PAGE");
            tft.setCursor(15, 30);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_BLUE);
            tft.print("Rotate to adjust");
            tft.setCursor(20, 100);
            tft.setTextColor(ST7735_WHITE);
            tft.print("Click to save.");

            tft.setCursor(20, 50);
            tft.setTextSize(3);
            tft.setTextColor(ST7735_WHITE);
            tft.print(hours);

            tft.setTextColor(ST7735_YELLOW);
            tft.setCursor(70, 50);
            tft.print(minutes);

            tft.setTextColor(ST7735_WHITE);
            tft.setCursor(55, 50);
            tft.print(":");
          }
        } else {
          if (minutesCounter <= 0) {
            minutesCounter = 60;
          } else {
            counter --;
          }
          if (((counter & 1) == 0)) {
            minutesCounter --;
            minutes = minutesCounter;

            tft.fillScreen(ST7735_BLACK); // Clear the display
            drawHeader();
            tft.setCursor(15, 20);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_YELLOW);
            tft.print("DATE & TIME PAGE");
            tft.setCursor(15, 30);
            tft.setTextSize(1);
            tft.setTextColor(ST7735_BLUE);
            tft.print("Rotate to adjust");
            tft.setCursor(20, 100);
            tft.setTextColor(ST7735_WHITE);
            tft.print("Click to save.");

            tft.setCursor(20, 50);
            tft.setTextSize(3);
            tft.setTextColor(ST7735_WHITE);
            tft.print(hours);

            tft.setTextColor(ST7735_YELLOW);
            tft.setCursor(70, 50);
            tft.print(minutes);

            tft.setTextColor(ST7735_WHITE);
            tft.setCursor(55, 50);
            tft.print(":");
          }
        }
      }
      Serial.println(counter);
      Serial.println(minutesCounter);
      // Click to save the current position
      if (digitalRead(encoderSwitch) == LOW) {
        //    selectedClockIndex ++;                                      // going to adjust days
        // TODO but for now we only adjust time!!!
        configureTime(hours, minutes, seconds);
        Serial.println("selectedClockIndex changed!");

        // Change colors of the numbers - Color minutes
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHeader();

        tft.setCursor(20, 50);
        tft.setTextSize(3);
        tft.setTextColor(ST7735_WHITE);              // Here   Colored color for choosen is Yellow
        tft.print(hours);

        tft.setTextColor(ST7735_WHITE);            // Here    Colored color for choosen is Yellow
        tft.setCursor(70, 50);
        tft.print(minutes);

        tft.setTextColor(ST7735_WHITE);
        tft.setCursor(55, 50);
        tft.print(":");

        // Display text - New Values Saved
        tft.setTextSize(1);
        tft.setCursor(15, 100);

        tft.setTextColor(ST7735_GREEN);
        tft.print("New value Saved.");
        delay(3000); // Wait before show Menu, because if user little holds the button activate the first menu automaticly
        pages[0] = 1;   // We return to Home page
        pages[1] = 0;
        pages[2] = 0;
        pages[3] = 0;
        pages[4] = 0;
        pages[5] = 0;
        tft.fillScreen(ST7735_BLACK); // Clear the display
        drawHomePage();
      }
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state
  }

  // -------------------------------------- Humidity PAGE ------------------------------------ //
  if (pages[4] == 1) {
    Serial.println("We are in hum page");
    tft.setTextSize(1);
    tft.setCursor(15, 50);
    tft.setTextColor(ST7735_BLUE);
    tft.print("Conf Humidity");
    drawHeader();
    if (digitalRead(encoderSwitch) == LOW) {  // We go back
      pages[0] = 0;
      pages[1] = 1;                          // We return to Menu page
      pages[2] = 0;
      pages[3] = 0;
      pages[4] = 0;
      pages[5] = 0;

      // ------------- //
      tft.fillScreen(ST7735_BLACK); // Clear the display
      drawHeader();
      drawMenuListEmpty();
      counter = 4;                           // Fix the counter for the Menu, so we can start the menu with Last choosen menu
      menus[0] = 0;
      menus[1] = 0;
      menus[2] = 1;
      menus[3] = 0;
      menus[4] = 0;
      drawMenuListButtons(false, false, false, true, false, false);
      delay(1000); // Wait before show Menu, because if user little holds the button activate the first menu automatic
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state
  }


  // -------------------------------------- About PAGE ------------------------------------ //
  if (pages[5] == 1) {
    drawHeader();
    Serial.println("We are in About page");
    tft.setTextSize(1);
    tft.setCursor(15, 15);
    tft.setTextColor(ST7735_BLUE);
    tft.print("Firmware Version");
    tft.setCursor(40, 30);
    tft.setTextSize(2);
    tft.setTextColor(GREEN);
    tft.print(FIRMWARE_VERSION);
    tft.drawFastHLine(25, 47, 70, RED);

    tft.setTextSize(1);
    tft.setCursor(5, 55);
    tft.setTextColor(WHITE);
    tft.print("Country  ");
    tft.setTextColor(GREEN);
    tft.print(COUNTRY);

    tft.setCursor(5, 67);
    tft.setTextColor(WHITE);
    tft.print("Source   ");
    tft.setTextColor(GREEN);
    tft.print(CODE_LOCATION);

    tft.setCursor(5, 79);
    tft.setTextColor(WHITE);
    tft.print("Owner    ");
    tft.setTextColor(GREEN);
    tft.print(CODE_ORIGIN);

    tft.setCursor(5, 91);
    tft.setTextColor(WHITE);
    tft.print("Repo     ");
    tft.setTextColor(GREEN);
    tft.print(CODE_REPO);

    tft.setCursor(5, 103);
    tft.setTextColor(WHITE);
    tft.print("Date     ");
    tft.setTextColor(GREEN);
    tft.print(__DATE__);


    tft.setCursor(5, 115);
    tft.setTextColor(WHITE);
    tft.print("Time     ");
    tft.setTextColor(GREEN);
    tft.print(__TIME__);

    if (digitalRead(encoderSwitch) == LOW) {  // We go back
      pages[0] = 0;
      pages[1] = 1;                          // We return to Menu page
      pages[2] = 0;
      pages[3] = 0;
      pages[4] = 0;
      pages[5] = 0;

      // ------------- //
      tft.fillScreen(ST7735_BLACK); // Clear the display
      drawHeader();
      drawMenuListEmpty();
      counter = 6;                           // Fix the counter for the Menu, so we can start the menu with Last choosen menu
      menus[0] = 0;
      menus[1] = 0;
      menus[2] = 0;
      menus[3] = 1;
      menus[4] = 0;
      drawMenuListButtons(false, false, false, false, true, false);
      delay(1000); // Wait before show Menu, because if user little holds the button activate the first menu automatic
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state
  }


}

void drawTempPage() {
  // --------------------------TERMOMETER CALIBRATION VALUES-------------------------------

  Serial.print("Temp Calibration: ");
  Serial.print(tempCalibration);
  Serial.print(" , counter: ");
  Serial.print(counter);
  Serial.println("");

  if (lastTempCalibrationState != tempCalibration) {                // ON ROTATION! we check for the last temo calibration, if not will start always from 0, instead of -4 for example
    tft.fillScreen(ST7735_BLACK); // Clear the display
    drawHeader();
    tft.setCursor(40, 40);
    tft.setTextSize(7);
    printTermometerValues(tempCalibration);
    tft.setCursor(15, 20);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_GREEN);
    tft.print("Temp Calibration");
    tft.setCursor(20, 100);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Click to save.");
    lastTempCalibrationState = tempCalibration;
  }


  if (digitalRead(encoderSwitch) == LOW) {
    Serial.println("Button pressed!");
    Serial.println("Saving the Calibration Value of the Temperature sensor!");
    Serial.println(" - In the beggining of the loop the temp value will be refreshed.");
    tft.fillScreen(ST7735_BLACK); // Clear the display
    drawHeader();
    tft.setCursor(40, 40);
    tft.setTextSize(7);
    temp = temp + tempCalibration;
    printTermometerValues(temp);
    tft.setTextSize(1);
    tft.setCursor(15, 100);

    tft.setTextColor(ST7735_GREEN);
    tft.print("New value Saved.");
    delay(1000); // Wait before show Menu, because if user little holds the button activate the first menu automaticly
    pages[0] = 1;   // We return to Home page
    pages[1] = 0;
    pages[2] = 0;
    pages[3] = 0;
    pages[4] = 0;
    pages[5] = 0;
    tft.fillScreen(ST7735_BLACK); // Clear the display
    drawHomePage();
  }
}

void drawMenuListEmpty() {
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(45, 25);
  tft.print("Temp");
  tft.setCursor(45, 45);
  tft.print("Clock");
  tft.setCursor(45, 65);
  tft.print("Hum");
  tft.setCursor(45, 85);
  tft.print("About");
  tft.setCursor(45, 105);
  tft.print("Back");
}

void drawMenuListButtons(bool isPresed, bool menu1IsSelected, bool menu2IsSelected, bool menu3IsSelected, bool menu4IsSelected, bool menu5IsSelected) {
  uint16_t color1;
  if (isPresed) {
    color1 = GREEN;
  } else {
    color1 = CYAN;
  }

  if (menu1IsSelected) {
    tft.drawRect(35, 20, 56, 15, color1);
  } else if (menu2IsSelected) {
    tft.drawRect(35, 40, 56, 15, color1);
  } else if (menu3IsSelected) {
    tft.drawRect(35, 60, 56, 15, color1);
  } else if (menu4IsSelected) {
    tft.drawRect(35, 80, 56, 15, color1);
  } else if (menu5IsSelected) {
    tft.drawRect(35, 100, 56, 15, color1);
  }
}

void drawHeader() {
  // -----------------------BATTERY STATUS ICON----------------------------------

  vcc = readVcc() - 130;  // Calibrate the reading

  if (vcc != vccLastState) {
    tft.fillRect(40, 0, 88, 13, BLACK);               // clear header, voltage is changed
    vccLastState = vcc;
  }

  tft.setTextSize(1);
  // If we put Regulated PSU - 5V 1A, the current used from the device is 0.115/0.116A
  // Basicaly the readVcc function did not return equivalent values and has no constant change regarding the provided voltage.
  // readVcc uses a integreated vin of arduino to check the voltage supplied.
  // However we correct that voltage with - 0.130V (130 milliVolts) to get the right results

  // min - 2.8V  (but for serial monitor needed at least 3.3)
  // max - 5V

  // Checked several settings:
  // Supplied     Actual + Calibration
  // 2800 V          2748 V
  // 3000 V          2961 V
  // 3300 V          3269 V
  // 3700 V          3684 V
  // 4000 V          4007 V
  // 4200 V          4214 V
  // 4500 V          4539 V
  // 4700 V          4741 V
  // 5000 V          5079 V

  if (vcc < 2961) {
    batteryStatusImage("", 105, 2,  5, true);            // we call the function with "" because if expired=true we set RED color instead
  } else if (vcc >= 2961) {
    if (vcc <= 4214) {
      tft.setCursor(40, 3);
      tft.setTextColor(ST7735_GREEN);
      tft.print(vcc);
      tft.print("mV");
      // ------------------------CHARGING ICON BATTERY PERCENTAGE---------------------------------
      tft.setTextColor(ST7735_WHITE);
      tft.setCursor(82, 3);
      tft.print(map(vcc, 2961, 4214, 0, 100));
      tft.print("%");

      batteryStatusImage(ST7735_WHITE, 105, 2, map(vcc, 2961, 4214, 1, 16), false);
    } else if (vcc > 4214) {
      tft.setCursor(40, 3);
      tft.setTextColor(ST7735_YELLOW);
      tft.print(vcc);
      tft.print("mV");

      tft.setTextColor(ST7735_CYAN);
      tft.setCursor(95, 3);
      tft.print("USB");

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
  // check if temp has different value
  
  if (temp != lastTempState) {
    tft.fillRect(0, 35, 128, 55, BLACK);      // Clear the Temp Value and Temp Image Rectangle Area
    lastTempState = temp;
  }
  drawHeader();
  // --------------------------TERMOMETER VALUES-------------------------------
  // TODO Change with the value from the sensor
  // tft.fillScreen(ST7735_BLACK);
  tft.setCursor(40, 37);
  tft.setTextSize(7);
  printTermometerValues(temp);
  printHumidityValues(hum);
  // -----------------------TERMOMETER STATUS ICON----------------------------------
  if (temp >= 22 && temp <= 24) {
    termometerStatusImage(WHITE, GREEN, 10, 10, map(temp, 1, 60, 1 , 29));
  } else if (temp > 24) {
    termometerStatusImage(WHITE, RED, 10, 10, map(temp, 1, 60, 1 , 29));
  } else if (temp < 22) {
    termometerStatusImage(WHITE, BLUE, 10, 10, map(temp, 1, 60, 1 , 29));
  }

  // -------------------- Humidity STATUS ICONS -------------------------------
  HumidityStatusImage(BLUE, CYAN, 10, 98);
  HumidityStatusImage(BLUE, CYAN, 5, 108);

}

// Warning - This method is using 3% of Storage Space
void HumidityStatusImage(uint16_t colorBorder, uint16_t colorFillUp, uint16_t x, uint16_t y) {
  tft.setCursor(1, 100);
  tft.drawPixel(x , y + 3, colorBorder);
  tft.drawPixel(x - 1, y + 4, colorBorder);
  tft.drawPixel(x - 2, y + 5, colorBorder);
  tft.drawPixel(x - 2, y + 6, colorBorder);
  tft.drawPixel(x - 2, y + 7, colorBorder);
  tft.drawPixel(x - 1, y + 8, colorBorder);
  tft.drawPixel(x, y + 9, colorFillUp);
  tft.drawPixel(x + 1, y + 10, colorBorder);
  tft.drawPixel(x + 2, y + 10, colorBorder);
  tft.drawPixel(x + 2, y + 9, colorBorder);
  tft.drawPixel(x + 3, y + 8, colorBorder);
  tft.drawPixel(x + 4, y + 7, colorBorder);
  tft.drawPixel(x + 4, y + 6, colorBorder);
  tft.drawPixel(x + 4, y + 6, colorBorder);
  tft.drawPixel(x, y + 8, colorFillUp);
  tft.drawPixel(x + 1, y + 9, colorFillUp);
  tft.drawPixel(x + 1, y + 8, colorFillUp);
  tft.drawPixel(x + 1, y + 7, colorFillUp);
  tft.drawPixel(x + 2, y + 7, colorFillUp);
  tft.drawPixel(x, y + 7, colorFillUp);
  tft.drawPixel(x - 1, y + 7, colorFillUp);
  tft.drawPixel(x - 1, y + 6, colorFillUp);
  tft.drawPixel(x, y + 6, colorFillUp);
  tft.drawPixel(x + 1, y + 6, colorFillUp);
  tft.drawPixel(x + 2, y + 6, colorFillUp);
  tft.drawPixel(x + 3, y + 5, colorFillUp);
  tft.drawPixel(x + 2, y + 5, colorFillUp);
  tft.drawPixel(x + 1, y + 5, colorFillUp);
  tft.drawPixel(x, y + 5, colorFillUp);
  tft.drawPixel(x - 1, y + 5, colorFillUp);
  tft.drawPixel(x + 2, y + 4, colorFillUp);
  tft.drawPixel(x + 1, y + 4, colorFillUp);
  tft.drawPixel(x, y + 4, colorFillUp);
  tft.drawPixel(x + 1, y + 3, colorFillUp);

}

void printHumidityValues(int hum) {
  if (hum > 99) { // Prevent display the 100th value, we dont need this space on the screen
    hum = 99;
  }
  if (lasthumState != hum) {
    tft.fillRect(18, 100, 35, 23, BLACK);
    lasthumState = hum;
  }

  tft.setCursor(18, 100);
  tft.setTextColor(CYAN);
  tft.setTextSize(3);
  tft.print(hum);

  // Draw % symbol
  tft.drawCircle(58, 104, 3, CYAN);
  tft.drawCircle(68, 116, 3, CYAN);
  tft.drawLine(58, 118, 68, 102, CYAN);
}

void printTermometerValues(int temp) {
  if (temp >= 22 && temp <= 24) {
    tft.setTextColor(ST7735_GREEN);
    tft.print(temp);
  } else if (temp > 24) {
    tft.setTextColor(ST7735_RED);
    tft.print(temp);
  } else if (temp < 22) {
    tft.setTextColor(ST7735_BLUE);
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
  while (bit_is_set(ADCSRA, ADSC)); // measunbring

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
  int hours = 0;
  int minutes = 0;

  if (RTC.read(tm)) {
    hours = tm.Hour;
    minutes = tm.Minute;
    if (hours != hourLastState || minutes != minutesLastState) {
      tft.fillRect(0, 0, 34, 13, BLACK);
      hourLastState = hours;
      minutesLastState = minutes;
    }

    msg += hours;
    msg += ":";
    msg += minutes;
  } else {
    if (RTC.chipPresent()) {
      // TODO - display error on the screen
      /*
        Serial.println("The DS1307 is stopped.  Please run the SetTime");
        Serial.println("example to initialize the time and begin running.");
        Serial.println();
      */
    } else {
      // TODO - display error on the screen
      /*
        Serial.println("DS1307 CLOCK read error!  Please check the circuitry.");
        Serial.println();
      */
    }
    // delay(9000);
  }
  return msg;
}

void configureTime(int Hour, int Min, int Sec) {   // Configure RTCDS1307 Clock module by using #include <TimeLib.h>   // Needed by CLock ; #include <DS1307RTC.h> // Clock library
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  RTC.write(tm);
}

// TODO - getDate is not used, uncomment when will be displayed on the screen
/*
  bool getDate(const char *str)
  {
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;

  return true;
  }
*/

int getDhtData(String option) {
  // Wait a few seconds between measurements.
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

  // Read temperature as Fahrenheit (isFahrenheit = true)
  //  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  // if (isnan(h) || isnan(t) || isnan(f)) {
  // Serial.println("Failed to read from DHT sensor!");  //TODO - display error message on the screen, not system out println
  //   return;
  //  }
  //  if (isnan(t) || isnan(h)) {  // Check for errors
  //    return;
  //  }

  // Compute heat index in Fahrenheit (the default)
  //  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  // float hic = dht.computeHeatIndex(t, h, false);
  /*
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");
  */
  if (option == "temp") {
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    return t;
  } else if (option == "hum") {
    float h = dht.readHumidity();
    return h;
  }
  delay(250);
}

void printFirmwareInfo() {
  tft.setTextSize(1);
  tft.setTextColor(ST7735_GREEN);
  tft.setCursor(5, 30);
  tft.print("SKL Electronics");
  delay(1000);
  tft.setCursor(5, 50);
  tft.print("Firmware date:");
  tft.setCursor(5, 60);
  tft.print(__DATE__);
  tft.setCursor(5, 80);
  tft.print(__TIME__);
  delay(3000);
  tft.fillScreen(ST7735_BLACK);
}
