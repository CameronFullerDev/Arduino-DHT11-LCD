#include <LiquidCrystal.h>
#include <DHT.h>

// === Constants ===
constexpr int DHTPIN = 13;
constexpr int DHTTYPE = DHT11;
constexpr int SOIL_PIN = A0;

constexpr int PUMP_IN1 = 9;
constexpr int PUMP_IN2 = 10;
constexpr int PUMP_EN = 11;

constexpr int TEMP_HIGH_THRESHOLD = 24; // °C
constexpr int SOIL_DRY_THRESHOLD_PERCENT = 40; // Below this = dry

constexpr unsigned long LOOP_DELAY_MS = 2000;
constexpr unsigned long PUMP_ON_DURATION = 60UL * 1000;   // 1 minute
constexpr unsigned long PUMP_INTERVAL = 10UL * 60 * 1000; // 10 minutes

// === Soil Moisture Calibration ===
constexpr int SOIL_WET = 300;  // Adjust based on testing
constexpr int SOIL_DRY = 800;  // Adjust based on testing

// === Objects ===
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
DHT dht(DHTPIN, DHTTYPE);

// === State Variables ===
bool pumpIsRunning = false;
unsigned long lastPumpCheckTime = -PUMP_INTERVAL;  // Force first check
unsigned long pumpStartTime = 0;

void setup() {
  lcd.begin(16, 2);
  dht.begin();
  lcd.print("Starting...");

  pinMode(PUMP_IN1, OUTPUT);
  pinMode(PUMP_IN2, OUTPUT);
  pinMode(PUMP_EN, OUTPUT);

  stopPump();  // Ensure pump is off
  delay(2000);
  lcd.clear();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int rawMoisture = analogRead(SOIL_PIN);
  int moisturePercent = getMoisturePercent(rawMoisture);
  unsigned long currentTime = millis();

  if (isnan(humidity) || isnan(temperature)) {
    lcd.clear();
    lcd.print("Sensor error");
    stopPump();
    pumpIsRunning = false;
  } else {
    bool conditionsGood = (temperature > TEMP_HIGH_THRESHOLD) &&
                          (moisturePercent < SOIL_DRY_THRESHOLD_PERCENT);

    // === Stop pump if either time is up OR conditions have improved ===
    if (pumpIsRunning) {
      if ((currentTime - pumpStartTime >= PUMP_ON_DURATION) || !conditionsGood) {
        stopPump();
        pumpIsRunning = false;
      }
    }

    // === Start pump if enough time passed and conditions need it ===
    if (!pumpIsRunning && (currentTime - lastPumpCheckTime >= PUMP_INTERVAL)) {
      lastPumpCheckTime = currentTime;

      if (conditionsGood) {
        startPump();
        pumpStartTime = currentTime;
        pumpIsRunning = true;
      }
    }

    displaySensorData(temperature, humidity, moisturePercent, pumpIsRunning);
  }

  delay(LOOP_DELAY_MS);
}

void displaySensorData(float temperature, float humidity, int moisturePercent, bool pumpOn) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print((int)temperature); // Display temperature as whole number
  lcd.print("C H:");
  printFloat(lcd, humidity, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("M:");
  lcd.print(moisturePercent);
  lcd.print("% PUMP:");
  lcd.print(pumpOn ? "ON " : "OFF");
}

void printFloat(LiquidCrystal& display, float value, uint8_t decimals) {
  display.print(value, decimals);
}

int getMoisturePercent(int rawValue) {
  rawValue = constrain(rawValue, SOIL_WET, SOIL_DRY);
  return map(rawValue, SOIL_DRY, SOIL_WET, 0, 100); // Inverse scale: dry=0%
}

void startPump() {
  digitalWrite(PUMP_EN, HIGH);
  digitalWrite(PUMP_IN1, HIGH);
  digitalWrite(PUMP_IN2, LOW);
}

void stopPump() {
  digitalWrite(PUMP_EN, LOW);
  digitalWrite(PUMP_IN1, LOW);
  digitalWrite(PUMP_IN2, LOW);
}
