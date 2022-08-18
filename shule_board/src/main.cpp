#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <LibPrintf.h>
#include <MicroNMEA.h>
#include <TimeLib.h>

// #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
//#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
// #define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
// #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
// #define _TASK_PRIORITY          // Support for layered scheduling priority
// #define _TASK_MICRO_RES         // Support for microsecond resolution
// #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 and ESP32 ONLY)
// #define _TASK_DEBUG             // Make all methods and variables public for debug purposes
// #define _TASK_INLINE            // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
// #define _TASK_TIMEOUT           // Support for overall task timeout
// #define _TASK_OO_CALLBACKS      // Support for dynamic callback method binding
#include <TaskScheduler.h>
// Scheduler
Scheduler ts;

SoftwareSerial gps(7, 0); // RX, TX

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

static char nmeaBuffer[85];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

static char iobuf[1024];

static void SyncGPS() {
  if (nmea.getYear() > 2000) {
    sprintf(iobuf, "GPS time: %d %d %d %d:%d\r\n", nmea.getYear(), nmea.getMonth(), nmea.getDay(), nmea.getHour(), nmea.getMinute());
    Serial.print(iobuf);
    setTime(nmea.getMinute(), nmea.getHour(), nmea.getSecond(), nmea.getDay(), nmea.getMonth(), nmea.getYear());
  }
}
#define GPS_SYNC_PERIOD 30 * TASK_SECOND
Task tGPS(GPS_SYNC_PERIOD, TASK_FOREVER, &SyncGPS, &ts, true );

static void BackupTick() {
  adjustTime(1);
}
Task tTick(TASK_SECOND, TASK_FOREVER, &BackupTick, &ts, true );

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
static void ChangeLEDs() {
  if (timeStatus() != timeSet) {
    colorWipe(strip.Color(255, 0, 0), 25); // Red
    colorWipe(strip.Color(0, 0, 255), 25); // Blue
    return;
  }
  static int which = 0;
  switch (which++) {
    case 0:
      colorWipe(strip.Color(255, 0, 0), 50); // Red
      break;
    case 1:
      colorWipe(strip.Color(0, 255, 0), 50); // Green
      break;
    case 2:
      colorWipe(strip.Color(0, 0, 255), 50); // Blue
    default:
      which = 0;
  }
}
Task tLEDs(TASK_SECOND * 10, TASK_FOREVER, &ChangeLEDs, &ts, true );

void setup() {
  Serial.begin(9600);
  gps.begin(9600);

  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
}

uint32_t Wheel(byte WheelPos);

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void loop() {
  int i = 0;
  while (i++ < 100 && gps.available()) {
    nmea.process(gps.read());
  }
  ts.execute();
  #if 0
  static int counter = 0;

  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  //colorWipe(strip.Color(0, 255, 0), 50); // Green
  //colorWipe(strip.Color(0, 0, 255), 50); // Blue
//colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW

  while (gps.available()) {
    char c = gps.read();
      if (nmea.process(c) && nmea.getYear() > 2020) {
        sprintf(iobuf, "GPS time: %d %d %d %d:%d\r\n", nmea.getYear(), nmea.getMonth(), nmea.getDay(), nmea.getHour(), nmea.getMinute());
        Serial.print(iobuf);
      }

  }
  #endif
}
