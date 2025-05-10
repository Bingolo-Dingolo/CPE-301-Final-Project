#include <LiquidCrystal.h> // LCD library
#include <DHT11.h> // Temp and Humidity Module library
#include <RTClib.h> // Real Time Clock library
#include <Stepper.h> // Stepper library

#define RESET_BTN 12
#define START_STOP_BTN 2
#define FAN_CTRL_PIN 41
#define WATER_SENSOR_PIN 37
#define DHT_PIN 8
#define DHTTYPE DHT11
#define BACK_BTN 25
#define FORWARD_BTN 39

#define GREEN_LED 6   // IDLE state
#define BLUE_LED 3    // RUNNING state
#define YELLOW_LED 4  // DISABLED state
#define RED_LED 5     // ERROR state

// LCD setup
LiquidCrystal lcd(43, 45, 50, 51, 52, 53);  // RS, E, D4, D5, D6, D7

// Stepper Motor setup
#define STEPS 100
Stepper stepper(STEPS, 27, 29, 31, 33);

// DHT sensor setup
DHT11 dht(DHT_PIN);

// RTC setup
RTC_DS3231 rtc;

// State Enum
enum SystemState { DISABLED, IDLE, RUNNING, ERROR };
SystemState currentState = DISABLED;

// Variables
bool systemEnabled = false;
int stepPosition = 0;
const int TEMP_THRESHOLD_HIGH = 22;
const int TEMP_THRESHOLD_LOW = 20;
bool fanOn = false;
unsigned long lastUpdateTime = 0;

// ISR Flags
volatile bool startStopPressed = false;

// ISR Functions
void handleStartStop(){
  startStopPressed = true;
}

// RTC Function
void logEvent(String message){
  DateTime now = rtc.now();
  Serial.print("[");
  Serial.print(now.timestamp());
  Serial.print("] ");
  Serial.println(message);
}

// LCD function 
void updateLCD(float temp, float humidity){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.print(" %");
}

void updateLEDs(SystemState state){
  digitalWrite(GREEN_LED, state == IDLE);
  digitalWrite(BLUE_LED, state == RUNNING);
  digitalWrite(YELLOW_LED, state == DISABLED);
  digitalWrite(RED_LED, state == ERROR);
}

void changeState(SystemState newState){
  currentState = newState;
  updateLEDs(currentState);
  logEvent("State changed to: " + String(newState));
}

void moveStepper(int direction){
  if (direction > 0) {
    stepper.step(25);
    stepPosition++;
  } 
  else{
    stepper.step(-25);
    stepPosition--;
  }
  logEvent("Stepper moved to position: " + String(stepPosition));
}

void setup(){
  Serial.begin(9600);
  lcd.begin(16, 2);
  rtc.begin();

  // components
  pinMode(FAN_CTRL_PIN, OUTPUT);
  pinMode(WATER_SENSOR_PIN, INPUT);
  pinMode(RESET_BTN, INPUT_PULLUP);
  pinMode(START_STOP_BTN, INPUT_PULLUP);
  pinMode(BACK_BTN, INPUT_PULLUP);
  pinMode(FORWARD_BTN, INPUT_PULLUP);

  // LEDs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // required a specific pin
  attachInterrupt(digitalPinToInterrupt(START_STOP_BTN), handleStartStop, FALLING);
  changeState(DISABLED);
}

void loop(){
  if (startStopPressed){
    startStopPressed = false;
    if (currentState == DISABLED){
      changeState(IDLE);
    } 
    else{
      digitalWrite(FAN_CTRL_PIN, LOW);
      fanOn = false;
      changeState(DISABLED);
    }
  }

  if (currentState == DISABLED){
    return; // Nothing ever happens while in disabled state
  }

  // reset button
  if (digitalRead(RESET_BTN) == LOW && currentState == ERROR){
    if (digitalRead(WATER_SENSOR_PIN) == HIGH){
      changeState(IDLE);
    }
  }

  // stepper control
  if (digitalRead(FORWARD_BTN) == LOW) moveStepper(1);
  if (digitalRead(BACK_BTN) == LOW) moveStepper(-1);

  // LCD and sensors update every 5 seconds, I think
  if (millis() - lastUpdateTime > 5000 || lastUpdateTime == 0){
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    updateLCD(temp, hum);
    lastUpdateTime = millis();

    if (digitalRead(WATER_SENSOR_PIN) == LOW){
      changeState(ERROR);
      return;
    }

    if (currentState == IDLE && temp > TEMP_THRESHOLD_HIGH){
      digitalWrite(FAN_CTRL_PIN, HIGH);
      fanOn = true;
      changeState(RUNNING);
    }

    if (currentState == RUNNING && temp < TEMP_THRESHOLD_LOW){
      digitalWrite(FAN_CTRL_PIN, LOW);
      fanOn = false;
      changeState(IDLE);
    }
  }

  if (currentState == ERROR){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WATER LEVEL LOW");
  }
}
