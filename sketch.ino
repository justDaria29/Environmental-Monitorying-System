#include <LiquidCrystal.h>
#include <TimeLib.h>

tmElements_t tm;
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns

int state1 = 0;
int state2 = 0;
int state3 = 0;
int state4 = 0;
int state5 = 0;

int b1_prev;
int b2_prev;
int b3_prev;
int b4_prev;
int b5_prev;

int b1; //data
int b2; //ora
int b3; //temperatura
int b4; //luminozitatea
int b5; //calitate aer

#define TEMP_PIN A0       // Temperature sensor
#define LIGHT_PIN A4      // Light sensor
#define AIR_QUALITY_PIN A5

int currentState = 0;  // 0 = welcome screen, 1-6 for each button function
unsigned long lastButtonPress = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long DEBOUNCE_TIME = 200; 
const unsigned long DISPLAY_UPDATE = 1000;

const int TIME_ZONE_OFFSET = 2;  // Romania is UTC+2

void setup()
{
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);

  Serial.begin (9600);
  Serial.println(__DATE__);
  Serial.println(__TIME__);
  if (getDate(__DATE__) && getTime(__TIME__)) {
    Serial.println("AVR Macro strings converted to tmElements.");
  }
  setTime(makeTime(tm));//set Ardino system clock to compiled time
    Serial.println("System millis clock referenced to tmElements.");
    Serial.println();

  lcd.begin(16, 2);
  lcd.print("Welcome!"); //Display a intro message
  lcd.setCursor(0, 1);   // set the cursor to column 0, line 1
  lcd.print("Choose operation"); //Display a intro message 
  currentState = 0;
}

void loop()
{
  unsigned long currentMillis = millis();
  
  b1 = digitalRead(13); //update pins
  b2 = digitalRead(12);
  b3 = digitalRead(11);
  b4 = digitalRead(10);
  b5 = digitalRead(9);
  
  if (currentMillis - lastButtonPress > DEBOUNCE_TIME) {
    if (b1 == LOW && b1_prev == HIGH) {
      currentState = 1;
      lastButtonPress = currentMillis;
      lastDisplayUpdate = 0; // Force immediate update when changing state
    }
    else if (b2 == LOW && b2_prev == HIGH) {
      currentState = 2;
      lastButtonPress = currentMillis;
      lastDisplayUpdate = 0;
    }
    else if (b3 == LOW && b3_prev == HIGH) {
      currentState = 3;
      lastButtonPress = currentMillis;
      lastDisplayUpdate = 0;
    }
    else if (b4 == LOW && b4_prev == HIGH) {
      currentState = 4;
      lastButtonPress = currentMillis;
      lastDisplayUpdate = 0;
    }
    else if (b5 == LOW && b5_prev == HIGH) {
      currentState = 5;
      lastButtonPress = currentMillis;
      lastDisplayUpdate = 0;
    }
  }

  b1_prev = b1; // update previous state
  b2_prev = b2;
  b3_prev = b3;
  b4_prev = b4;
  b5_prev = b5;

  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE) {
    lastDisplayUpdate = currentMillis;
    
    // Handle current state
    switch (currentState) {
      case 0:  // Welcome screen - do nothing until button press
        break;
      case 1:
        printDate();
        break;
      case 2:
        printTime();
        break;
      case 3:
        printTemperature();
        break;
      case 4:
        printLightIntensity();
        break;
      case 5:
        printAirQuality();
        break;
    }
  }
  
}

// double readTempInCelsius(int count, int pin) {
//   // citește temperatura de count ori de pe pinul analogic pin
//   double sumTemp = 0;
//   for (int i =0; i < count; i++) {
//   int val = analogRead(pin);
//   double temp = log(((10240000/val) - 10000));
//   temp = 1 / (0.002859148 + (0.000234125 + (0.0000000876741 * temp * temp ))* temp );
//   temp += 55;
//   temp = temp - 273.15; 
//   sumTemp = sumTemp + temp; // suma temperaturilor
//   }
//   return sumTemp / (float)count; // media returnată
// }

double readTempInCelsius(int count, int pin) {
  double sumTemp = 0;
  for (int i = 0; i < count; i++) {
    int rawValue = analogRead(pin);
    
    // Convert analog reading to temperature
    // This formula is calibrated for use with a 10k voltage divider
    float resistance = (1023.0 / rawValue) - 1.0;
    resistance = 10000.0 / resistance;  // 10k is the voltage divider resistor value
    
    float temperature = 1.0 / (log(resistance / 10000.0) / 3950.0 + 1.0 / (273.15 + 25.0));
    temperature -= 273.15;  // Convert Kelvin to Celsius
    
    sumTemp += temperature;
    delay(10);
  }
  return sumTemp / count;
}

void printTemperature() {
  static bool showDots = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperature");
  lcd.print(showDots ? "..." : "   ");
  
  float temp = readTempInCelsius(10, TEMP_PIN);
  
  lcd.setCursor(0, 1);
  if (temp == -999) {
    lcd.print("Error reading");
  } else {
    lcd.print(temp, 1); // Display with 1 decimal place
    lcd.print(" C");
  }
  
  showDots = !showDots;
}

float readLightIntensity(int count, int pin) {
  long sumIntensity = 0;
  for (int i = 0; i < count; i++) {
    int rawValue = analogRead(pin);
    // Invert the reading since higher values mean darker conditions
    int invertedValue = 1023 - rawValue;
    // Convert to percentage
    float percentage = (invertedValue / 1023.0) * 100.0;
    sumIntensity += percentage;
    delay(10);
  }
  return sumIntensity / (float)count;
}

void printLightIntensity() {
  static bool showDots = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Light Level");
  lcd.print(showDots ? "..." : "   ");
  
  float lightLevel = readLightIntensity(10, LIGHT_PIN);
  
  lcd.setCursor(0, 1);
  lcd.print(lightLevel, 1);
  lcd.print("% ");
  
  if (lightLevel >= 75) {
    lcd.print("Bright");
  } else if (lightLevel >= 40) {
    lcd.print("Medium");
  } else if (lightLevel >= 15) {
    lcd.print("Dim");
  } else {
    lcd.print("Dark");
  }
  
  showDots = !showDots;
}

float readAirQuality(int count, int pin) {
  float sumAirQuality = 0;
  
  // Take multiple readings to get a stable value
  for (int i = 0; i < count; i++) {
    int reading = analogRead(pin);
    // Convert 0-1023 range to 0-100% air quality scale
    float percentage = map(reading, 0, 1023, 0, 100);
    sumAirQuality += percentage;
    delay(10); // Short delay between readings
  }
  
  return sumAirQuality / count;
}

// Update the display function to show meaningful air quality information
void printAirQuality() {
  static bool showDots = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Air Quality");
  lcd.print(showDots ? "..." : "   ");
  
  float airQuality = readAirQuality(10, AIR_QUALITY_PIN);
  
  lcd.setCursor(0, 1);
  
  // Display both numerical value and qualitative assessment
  lcd.print(airQuality, 1);
  lcd.print("% ");
  
  // Add qualitative assessment
  if (airQuality >= 80) {
    lcd.print("Good");
  } else if (airQuality >= 60) {
    lcd.print("Moderate");
  } else if (airQuality >= 40) {
    lcd.print("Poor");
  } else {
    lcd.print("Bad");
  }
  
  showDots = !showDots;
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;
  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour + TIME_ZONE_OFFSET;  // Add timezone offset
  if (tm.Hour >= 24) {  // Handle day rollover if needed
    tm.Hour -= 24;
  }
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

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
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

void printTime() {
  static bool showDots = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time");
  lcd.print(showDots ? "..." : "   ");
  lcd.setCursor(0, 1);
  
  // Get current time components
  int h = hour();
  int m = minute();
  int s = second();
  
  // Format hours with leading zero if needed
  if (h < 10) {
    lcd.print("0");
  }
  lcd.print(h);
  
  // Print minutes and seconds using the existing printDigits function
  printDigits(m);
  printDigits(s);
  
  showDots = !showDots;
}

void printDate() {
  static bool showDots = false;  // For blinking indicator
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Date");
  lcd.print(showDots ? "..." : "   ");
  lcd.setCursor(0, 1);
  lcd.print(month());
  lcd.print("/");
  lcd.print(day());
  lcd.print("/");
  lcd.print(year());
  showDots = !showDots;  // Toggle dots for next update
}
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if(digits < 10)
    lcd.print("0");
  lcd.print(digits);
}

