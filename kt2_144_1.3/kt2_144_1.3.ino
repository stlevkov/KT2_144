#include <Wire.h>                   // Needed for I2C communication
#include <TimeLib.h>                // Needed by CLock 
#include <DS1307RTC.h>              // Clock library

int temp = 0;                       // Holds the Temp variable for reading from DHT Sensor
int tempMin = 0;                    // Holds the minimum recorded temp
int tempMax = 0;                    // Holds the maximum recorded temp
int lastTempMin = 0;                // Needed for refreshing the Temp Min value
int lastTempMax = 0;                // Needed for refreshing the Temp Max value
int hum = 0;                        // Holds the hum var
int lastTempState = 0;              // define the last state of the temp measurement
int lasthumState = 0;               // define the last state of the hum measurement
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

// Color definitions for using tft library HEXs instead of ST7735_COLORNAME for example
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// Define your project info here to show on about page
#define FIRMWARE_VERSION "1.2"
#define COUNTRY "Bulgaria"
#define CODE_LOCATION "GitHub"
#define CODE_ORIGIN "/stlevkov"
#define CODE_REPO "/KT2_144"

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13
#define TFT_MOSI 11

float p = 3.1415926;                    // define pi first 7 numbers

// Define D3, D4 for Rotary Encoder Button and the switch
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
const int vccCalibration = 130;         // Use this with minus offset ( - 130);

// Used by Clock Adjustion Page for selecting the current position and colored it by defined color in the methods bellow
int selectedClockIndex = 0;             // Posibble values for Clock Adjust Page: 0 - hours, 1 - minutes, 2 - day, 3 - month, 4 - year

//Define Days for Clock
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
tmElements_t tm;                        // Define the RTC Class use

int hourLastState = 0;                  // Needed for refreshing the Hour value
int minutesLastState = 0;               // Needed for refreshing the Minutes value
int secondsLastState = 0;               // Needed for refreshing the Seconds value

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
  tft.fillScreen(BLACK);


  // This info is for Boot up, showing the last firmware compile date & time - there is 3 sec delay in here
  printFirmwareInfo();

  // Pages
  pages[0] = 1;  // Home
  pages[1] = 0;  // Menu
  pages[2] = 0;  // Temp ajust calibration
  pages[3] = 0;  // Clock ajust
  pages[4] = 0;  // Battery info
  pages[5] = 0;  // About

  // Menus
  menus[0] = 1;  // Temp
  menus[1] = 0;  // Clock
  menus[2] = 0;  // Batt
  menus[3] = 0;  // About
  menus[4] = 0;  // Back
  // ---------------- +++++   DHT22     +++++----------------- //
  dht.begin();

  // ---------------- +++++  INIT TEMP MIN MAX     +++++----------------- //
  tempMin = getDhtData("temp");
  tempMax = tempMin;
}

void loop() {
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
      tft.fillScreen(BLACK); // Clear the display
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
        menus[0] = 1; //TEMP
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, true, false, false, false, false);
      } else if (counter == 2) {     //  WE MARK MENU 2 CLOCK
        menus[0] = 0;
        menus[1] = 1; //CLOCK
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, false, true, false, false, false);
      } else if (counter == 4) {    // WE MARK MENU 3 BATT
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 1; //BATT
        menus[3] = 0;
        menus[4] = 0;
        tft.fillScreen(BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, false, false, true, false, false);
      } else if (counter == 6) {     // WE MARK MENU 4 ABOUT
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 1; //ABOUT
        menus[4] = 0;
        tft.fillScreen(BLACK); // Clear the display
        drawHeader();
        drawMenuListEmpty();
        drawMenuListButtons(false, false, false, false, true, false);
      } else if (counter == 8) {      // WE MARK MENU 5 BACK
        menus[0] = 0;
        menus[1] = 0;
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 1; //BACK
        tft.fillScreen(BLACK); // Clear the display
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
        tft.fillScreen(BLACK); // Clear the display
        drawHeader();
        tft.setCursor(15, 20);
        tft.setTextSize(1);
        tft.setTextColor(YELLOW);
        tft.print("DATE & TIME PAGE");
        tft.setCursor(15, 30);
        tft.setTextSize(1);
        tft.setTextColor(BLUE);
        tft.print("Rotate to adjust");
        tft.setCursor(20, 100);
        tft.setTextColor(WHITE);
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
        tft.setTextColor(YELLOW);
        tft.print(hours);

        tft.setTextColor(WHITE);
        tft.setCursor(70, 50);
        tft.print(minutes);

        tft.setTextColor(WHITE);
        tft.setCursor(55, 50);
        tft.print(":");

      }

    } else if (menus[2] == 1) {
      // Serial.println("We are marked MENU 3 - GO TO BATTERY PAGE");
      if (digitalRead(encoderSwitch) == LOW) {
        pages[0] = 0;
        pages[1] = 0;
        pages[2] = 0;
        pages[3] = 0;
        pages[4] = 1;   // We choose Battery Page
        // ------ //
        counter = 0;
        menus[0] = 1;
        menus[1] = 0;  // We choose Batt Menu
        menus[2] = 0;
        menus[3] = 0;
        menus[4] = 0;
      }
      if (pages[4] == 1) {
        Serial.println("Go to Batt Page.");
        drawMenuListButtons(true, false, false, true, false, false); // Draw selected Menu 2 for GREEN border
        delay(1000); // If user holds, will be redirected back to Menu from Home, because home checks if user press the button!
        tft.fillScreen(BLACK); // Clear the display
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
        drawMenuListButtons(true, false, false, false, true, false); // Draw selected Menu 2 for GREEN border
        delay(1000); // If user holds, will be redirected back to Menu from Home, because home checks if user press the button!
        tft.fillScreen(BLACK); // Clear the display
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
        tft.fillScreen(BLACK); // Clear the display

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

            tft.fillScreen(BLACK); // Clear the display
            drawHeader();
            tft.setCursor(15, 20);
            tft.setTextSize(1);
            tft.setTextColor(YELLOW);
            tft.print("DATE & TIME PAGE");
            tft.setCursor(15, 30);
            tft.setTextSize(1);
            tft.setTextColor(BLUE);
            tft.print("Rotate to adjust");
            tft.setCursor(20, 100);
            tft.setTextColor(WHITE);
            tft.print("Click to save.");

            tft.setCursor(20, 50);
            tft.setTextSize(3);
            tft.setTextColor(YELLOW);
            tft.print(hours);

            tft.setTextColor(WHITE);
            tft.setCursor(70, 50);
            tft.print(minutes);

            tft.setTextColor(WHITE);
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

            tft.fillScreen(BLACK); // Clear the display
            drawHeader();
            tft.setCursor(15, 20);
            tft.setTextSize(1);
            tft.setTextColor(YELLOW);
            tft.print("DATE & TIME PAGE");
            tft.setCursor(15, 30);
            tft.setTextSize(1);
            tft.setTextColor(BLUE);
            tft.print("Rotate to adjust");
            tft.setCursor(20, 100);
            tft.setTextColor(WHITE);
            tft.print("Click to save.");

            tft.setCursor(20, 50);
            tft.setTextSize(3);
            tft.setTextColor(YELLOW);
            tft.print(hours);

            tft.setTextColor(WHITE);
            tft.setCursor(70, 50);
            tft.print(minutes);

            tft.setTextColor(WHITE);
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
        tft.fillScreen(BLACK); // Clear the display
        drawHeader();
        tft.setCursor(15, 20);
        tft.setTextSize(1);
        tft.setTextColor(YELLOW);
        tft.print("DATE & TIME PAGE");
        tft.setCursor(15, 30);
        tft.setTextSize(1);
        tft.setTextColor(BLUE);
        tft.print("Rotate to adjust");
        tft.setCursor(20, 100);
        tft.setTextColor(WHITE);
        tft.print("Click to save.");

        tft.setCursor(20, 50);
        tft.setTextSize(3);
        tft.setTextColor(WHITE);              // Here   Colored color for choosen is Yellow
        tft.print(hours);

        tft.setTextColor(YELLOW);            // Here    Colored color for choosen is Yellow
        tft.setCursor(70, 50);
        tft.print(minutes);

        tft.setTextColor(WHITE);
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

            tft.fillScreen(BLACK); // Clear the display
            drawHeader();
            tft.setCursor(15, 20);
            tft.setTextSize(1);
            tft.setTextColor(YELLOW);
            tft.print("DATE & TIME PAGE");
            tft.setCursor(15, 30);
            tft.setTextSize(1);
            tft.setTextColor(BLUE);
            tft.print("Rotate to adjust");
            tft.setCursor(20, 100);
            tft.setTextColor(WHITE);
            tft.print("Click to save.");

            tft.setCursor(20, 50);
            tft.setTextSize(3);
            tft.setTextColor(WHITE);
            tft.print(hours);

            tft.setTextColor(YELLOW);
            tft.setCursor(70, 50);
            tft.print(minutes);

            tft.setTextColor(WHITE);
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

            tft.fillScreen(BLACK); // Clear the display
            drawHeader();
            tft.setCursor(15, 20);
            tft.setTextSize(1);
            tft.setTextColor(YELLOW);
            tft.print("DATE & TIME PAGE");
            tft.setCursor(15, 30);
            tft.setTextSize(1);
            tft.setTextColor(BLUE);
            tft.print("Rotate to adjust");
            tft.setCursor(20, 100);
            tft.setTextColor(WHITE);
            tft.print("Click to save.");

            tft.setCursor(20, 50);
            tft.setTextSize(3);
            tft.setTextColor(WHITE);
            tft.print(hours);

            tft.setTextColor(YELLOW);
            tft.setCursor(70, 50);
            tft.print(minutes);

            tft.setTextColor(WHITE);
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
        configureTime(hours, minutes, seconds, 23, 12);
        Serial.println("selectedClockIndex changed!");

        // Change colors of the numbers - Color minutes
        tft.fillScreen(BLACK); // Clear the display
        drawHeader();

        tft.setCursor(20, 50);
        tft.setTextSize(3);
        tft.setTextColor(WHITE);              // Here   Colored color for choosen is Yellow
        tft.print(hours);

        tft.setTextColor(WHITE);            // Here    Colored color for choosen is Yellow
        tft.setCursor(70, 50);
        tft.print(minutes);

        tft.setTextColor(WHITE);
        tft.setCursor(55, 50);
        tft.print(":");

        // Display text - New Values Saved
        tft.setTextSize(1);
        tft.setCursor(15, 100);

        tft.setTextColor(GREEN);
        tft.print("New value Saved.");
        delay(3000); // Wait before show Menu, because if user little holds the button activate the first menu automaticly
        pages[0] = 1;   // We return to Home page
        pages[1] = 0;
        pages[2] = 0;
        pages[3] = 0;
        pages[4] = 0;
        pages[5] = 0;
        tft.fillScreen(BLACK); // Clear the display
        drawHomePage();
      }
    }
    aLastState = aState; // Updates the previous state of the outputA with the current state
  }

  // -------------------------------------- Battery PAGE ------------------------------------ //
  if (pages[4] == 1) {
    Serial.println("We are in Batt page");
    drawHeader();
    tft.setTextSize(1);
    tft.setCursor(30, 20);
    tft.setTextColor(GREEN);
    tft.print("Battery INFO");
    tft.drawCircle(64, 77, 45, MAGENTA);

    // Define local variables, because the static ones are used by the drawHeader() method
    tft.setTextSize(2);
    tft.setTextColor(CYAN);
    tft.setCursor(50, 45);
    tft.print(map(vcc, 2961, 4214, 0, 100));
    tft.print("%");
    vccLastState = vcc;
    tft.setTextColor(RED);
    tft.setCursor(30, 69);
    tft.print(vcc);
    tft.print("mV");
    vcc = readVcc() - vccCalibration;
    if (vcc != vccLastState) {
      tft.fillRect(45, 45, 40, 15, BLACK);  // Clear the Percentage
      tft.fillRect(30, 69, 70, 15, BLACK);  // Clear the milliVolts
      vccLastState = vcc;
    }



    // TODO - Change with digitalRead charging pin
    if (true) {
      tft.setTextColor(GREEN);
      tft.setTextSize(1);
      tft.setCursor(40, 95);
      tft.print("Charging");
    }



    if (digitalRead(encoderSwitch) == LOW) {  // We go back
      pages[0] = 0;
      pages[1] = 1;                          // We return to Menu page
      pages[2] = 0;
      pages[3] = 0;
      pages[4] = 0;
      pages[5] = 0;

      // ------------- //
      tft.fillScreen(BLACK); // Clear the display
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
    tft.setTextColor(BLUE);
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
      tft.fillScreen(BLACK); // Clear the display
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
    tft.fillScreen(BLACK); // Clear the display
    drawHeader();
    tft.setCursor(40, 40);
    tft.setTextSize(7);
    printTermometerValues(tempCalibration);
    tft.setCursor(15, 20);
    tft.setTextSize(1);
    tft.setTextColor(GREEN);
    tft.print("Temp Calibration");
    tft.setCursor(20, 100);
    tft.setTextColor(WHITE);
    tft.print("Click to save.");
    lastTempCalibrationState = tempCalibration;
  }


  if (digitalRead(encoderSwitch) == LOW) {
    Serial.println("Button pressed!");
    Serial.println("Saving the Calibration Value of the Temperature sensor!");
    Serial.println(" - In the beggining of the loop the temp value will be refreshed.");
    tft.fillScreen(BLACK); // Clear the display
    drawHeader();
    tft.setCursor(40, 40);
    tft.setTextSize(7);
    temp = temp + tempCalibration;
    printTermometerValues(temp);
    tft.setTextSize(1);
    tft.setCursor(15, 100);

    tft.setTextColor(GREEN);
    tft.print("New value Saved.");
    delay(1000); // Wait before show Menu, because if user little holds the button activate the first menu automaticly
    pages[0] = 1;   // We return to Home page
    pages[1] = 0;
    pages[2] = 0;
    pages[3] = 0;
    pages[4] = 0;
    pages[5] = 0;
    tft.fillScreen(BLACK); // Clear the display
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
  tft.print("Batt");
  tft.setCursor(45, 85);
  tft.print("About");
  tft.setCursor(45, 105);
  tft.print("Back");
}

// ver. 1.2 25778 bytes | ver. 1.3 25868 bytes, but less rows of code
void drawMenuListButtons(bool isPresed, bool menu1IsSelected, bool menu2IsSelected, bool menu3IsSelected, bool menu4IsSelected, bool menu5IsSelected) {
  uint16_t color = isPresed ? GREEN : CYAN;
  uint16_t h = menu1IsSelected  ? 20 : menu2IsSelected  ? 40 : menu3IsSelected ? 60 : menu4IsSelected ? 80 : menu5IsSelected ? 100 : 0;
  tft.drawRoundRect(35, h, 48, 15, 4, color);
}

void drawHeader() {
  // -----------------------BATTERY STATUS ICON----------------------------------

  vcc = readVcc() - vccCalibration;  // Calibrate the reading

  if (vcc != vccLastState) {
    tft.fillRect(100, 0, 88, 11, BLACK);               // clear header, voltage is changed
    vccLastState = vcc;
  }

  tft.setTextSize(1);
  // If we put Regulated PSU - 5V 1A, the current used from the device is 0.115/0.116A
  // Basicaly the readVcc function did not return equivalent values and has no constant change regarding the provided voltage.
  // readVcc uses a integreated vin of arduino to check the voltage supplied.
  // However we correct that voltage with - 0.130V (130 milliVolts) to get the right results

  // Min - 2.8V  (but for serial monitor needed at least 3.3V)
  // Max - 4.2V on Battery

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
    batteryStatusImage(105, 2, 21, 9, map(vcc, 2961, 4214, 1, 16), true);
  } else if (vcc >= 2961) {
    if (vcc <= 4214) {
      // ------------------------CHARGING ICON BATTERY PERCENTAGE---------------------------------
      batteryStatusImage(105, 2, 21, 9, map(vcc, 2961, 4214, 1, 16), false);
    } else if (vcc > 4214) {
      // TODO - Add Charging logic
      tft.setTextColor(CYAN);
      tft.setCursor(95, 3);
      tft.print("USB");
    }
  }


  // -----------------------DATE AND TIME ----------------------------------
  tft.setCursor(2, 3);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print(getClock());  // Print Clock

}

void drawHomePage() {
  // check if temp has different value

  if (temp != lastTempState) {
    tft.fillRect(0, 35, 128, 55, BLACK);      // Clear the Temp Value and Temp Image Rectangle Area
    lastTempState = temp;
    if (tempMin > temp) {
      tempMin = temp;
    }
    if (tempMax < temp) {
      tempMax = temp;
    }
  }
  drawHeader();
  // --------------------------TERMOMETER VALUES-------------------------------
  // TODO Change with the value from the sensor
  // tft.fillScreen(BLACK);
  tft.setCursor(40, 37);
  tft.setTextSize(7);
  printTermometerValues(temp);
  printHumidityValues(hum);
  printTermometerMinAndMaxValues();
  printSeasonText(30, 20);                      // Color is determinated by the season

  // -----------------------TERMOMETER STATUS ICON----------------------------------
  if (temp >= 22 && temp <= 24) {
    termometerStatusImage(WHITE, GREEN, 10, 10, map(temp, 1, 60, 1 , 29));
  } else if (temp > 24) {
    termometerStatusImage(WHITE, RED, 10, 10, map(temp, 1, 60, 1 , 29));
  } else if (temp < 22) {
    termometerStatusImage(WHITE, BLUE, 10, 10, map(temp, 1, 60, 1 , 29));
  }

}

void printSeasonText(int x, int y) {
  tft.setCursor(x, y);
  tft.setTextSize(1);
  if (RTC.read(tm)) {
    if (tm.Month == 12 || tm.Month == 1 || tm.Month == 2) {
      tft.setTextColor(CYAN);
      tft.print("* WINTER *");
    } else if (tm.Month == 3 || tm.Month == 4 || tm.Month == 5){
      tft.setTextColor(GREEN);
      tft.print("SPRING");
    } else if (tm.Month == 6 || tm.Month == 7 || tm.Month == 8){
      tft.setTextColor(YELLOW);
      tft.print("SUMMER");
    } else if (tm.Month == 9 || tm.Month == 10 || tm.Month == 11){
      tft.setTextColor(MAGENTA);
      tft.print("AUTUMN");
    }
  }
}

void printTermometerMinAndMaxValues() {
  if (lastTempMin != tempMin){
      tft.fillRect(102, 100, 17, 10, BLACK);
      lastTempMin = tempMin;
  }

  if (lastTempMax != tempMax){
    tft.fillRect(102, 115, 17, 10, BLACK);
    lastTempMax = tempMax;
  }
  
  tft.setCursor(75, 100);
  tft.setTextSize(1);
  tft.setTextColor(GREEN);
  tft.print("Tmin ");
  tft.setTextColor(MAGENTA);
  tft.print(tempMin);
  tft.setCursor(75, 115);
  tft.setTextColor(GREEN);
  tft.print("Tmax ");
  tft.setTextColor(RED);
  tft.print(tempMax);
}

void printHumidityValues(int hum) {
  if (hum > 99) { // Prevent display the 100th value, we dont need this space on the screen
    hum = 99;
  }
  // Refresh with black rectangle if value is changed
  if (lasthumState != hum) {
    tft.fillRect(10, 100, 35, 23, BLACK);
    lasthumState = hum;
  }
  tft.setCursor(10, 100);
  tft.setTextColor(CYAN);
  tft.setTextSize(3);
  tft.print(hum);

  // Draw "%" percent symbol
  tft.drawCircle(50, 104, 3, CYAN);
  tft.drawCircle(60, 116, 3, CYAN);
  tft.drawLine(50, 118, 60, 102, CYAN);
}

void printTermometerValues(int temp) {
  if (temp >= 22 && temp <= 24) {
    tft.setTextColor(GREEN);
    tft.print(temp);
  } else if (temp > 24) {
    tft.setTextColor(RED);
    tft.print(temp);
  } else if (temp < 22) {
    tft.setTextColor(BLUE);
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

// Optimized scince ver. 1.2 = 80 bytes free
void batteryStatusImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillUp, boolean expired) {
  uint16_t color;
  if (expired) {
    color = RED;
  } else {
    color = WHITE;
    for (int i = 0; i <= fillUp; i++) {                  // Filling up
      tft.drawFastVLine((x + 18) - i, y + 2, h - 4, GREEN);  // can be from 1 to 16
    }
  }
  tft.drawRect(x, y, w, h, color);
  tft.drawFastVLine(x - 1, y + 2, h - 4, color);                 // Draw the left positive cathode of the battery #1
  tft.drawFastVLine(x - 2, y + 2, h - 4, color);                 // Draw the left positive cathode of the battery #2
}

String getClock() {
  String msg = "";
  tmElements_t tm;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  int days = 0;
  int months = 0;


  if (RTC.read(tm)) {

    // ----------- GET TIME ---------- //
    hours = tm.Hour;
    if (hours < 10) {
      msg += 0;
    }
    minutes = tm.Minute;
    if (hours != hourLastState || minutes != minutesLastState) {
      tft.fillRect(0, 0, 34, 13, BLACK);
      hourLastState = hours;
      minutesLastState = minutes;
    }
    msg += hours;
    msg += ":";
    if (minutes < 10) {
      msg += 0;
    }
    msg += minutes;
    msg += ":";

    seconds = tm.Second;
    if (seconds < 10) {
      msg += 0;
    }
    if (seconds != secondsLastState) {
      tft.fillRect(36, 0, 15, 11, BLACK);
      secondsLastState = seconds;
    }
    msg += seconds;

    // ----------- GET DATE ----------- //
    days = tm.Day;
    months = tm.Month;

    msg += " ";
    msg += days;
    msg += "/";
    msg += monthName[months - 1];

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


void configureTime(int hours, int mins, int secs, int days, int months) {   // Configure RTCDS1307 Clock module by using #include <TimeLib.h>   // Needed by CLock ; #include <DS1307RTC.h> // Clock library
  tm.Hour = hours;
  tm.Minute = mins;
  tm.Second = secs;
  tm.Day = days;
  tm.Month = months;
  RTC.write(tm);
}


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
  tft.setTextColor(GREEN);
  tft.setCursor(5, 30);
  tft.print("KT2_144 Open Source");
  delay(1000);
  tft.setCursor(5, 50);
  tft.print("Firmware date:");
  tft.setCursor(5, 60);
  tft.print(__DATE__);
  tft.setCursor(5, 80);
  tft.print(__TIME__);
  delay(3000);
  tft.fillScreen(BLACK);
}
