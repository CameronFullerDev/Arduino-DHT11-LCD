#include <LiquidCrystal.h>
#include <DHT.h>

constexpr uint8_t DHTPIN = 2;
constexpr uint8_t DHTTYPE = DHT11;
constexpr unsigned long LOOP_DELAY_MS = 2000;

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  lcd.begin(16, 2);
  dht.begin();
  lcd.print("Starting...");
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    lcd.clear();
    lcd.print("Sensor error");
  } else {
    displaySensorData(temperature, humidity);
  }

  delay(LOOP_DELAY_MS);
}

void displaySensorData(float temperature, float humidity) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  printFloat(lcd, temperature, 1);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  printFloat(lcd, humidity, 0);
  lcd.print("%");
}

void printFloat(LiquidCrystal& display, float value, uint8_t decimals) {
  // Arduino print supports decimals arg on floats, so just forward it
  display.print(value, decimals);
}