#include <WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ThingSpeak.h>

WiFiClient client;

unsigned long myChannelNumber = 3428611;
const char * myWriteAPIKey = "FUQJBTILLXYCCIA8";

void sendToThingSpeak(int soil, float temp, bool pump)
{
  ThingSpeak.setField(1, soil);
  ThingSpeak.setField(2, temp);
  ThingSpeak.setField(3, pump ? 1 : 0);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (x == 200) {
    Serial.println("Upload Success");
  } else {
    Serial.print("Upload Failed. Error: ");
    Serial.println(x);
  }
}

// Konfigurasi OLED SSD1306
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 15                   // Pin DATA DHT22 dihubungkan ke GPIO 15 ESP32
#define DHTTYPE DHT22               
DHT dht(DHTPIN, DHTTYPE);

const int SOIL_MOISTURE_PIN = 34;
const int PUMP_RELAY_PIN = 19;              // Pin input Modul Relay dihubungkan ke GPIO 19 ESP32

int MOISTURE_THRESHOLD_LOW = 15;            // Ambang batas bawah (%) untuk menyalakan pompa
int MOISTURE_THRESHOLD_HIGH = 85;           // Ambang batas atas (%) untuk mematikan pompa
bool PUMP_STATUS = false;

char ssid[] = "Wokwi-GUEST";
char pass[] = "";


// Fungsi untuk membaca sensor, update OLED, dan kirim data ke Blynk
void updateSystem()
{
  // 1. Membaca dan mengonversi kelembaban tanah ke persen
  int soilMoisturePercentage = map(analogRead(SOIL_MOISTURE_PIN), 0, 4095, 0, 100);

  // 2. Membaca suhu dari DHT22
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 0.0;
  }

  // 3. Logika kontrol Relay Pompa berdasarkan kelembaban tanah
  if (soilMoisturePercentage < MOISTURE_THRESHOLD_LOW)
  {
    PUMP_STATUS = true;
    digitalWrite(PUMP_RELAY_PIN, HIGH);     // Relay Aktif -> Pompa NYALA
  }
  else if (soilMoisturePercentage > MOISTURE_THRESHOLD_HIGH)
  {
    PUMP_STATUS = false;
    digitalWrite(PUMP_RELAY_PIN, LOW);      // Relay Mati -> Pompa MATI
  }

  // 4. Print status ke Serial Monitor untuk debugging
  Serial.print("Soil Moisture: ");   Serial.print(soilMoisturePercentage); Serial.println("%");
  Serial.print("Temperature: ");     Serial.print(temperature);            Serial.println("°C");
  Serial.print("Pump Status: ");     Serial.println(PUMP_STATUS ? "ON" : "OFF");
  Serial.println("-------------");
  // 6. Tampilkan data ke Layar OLED SSD1306
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("--- MONITORING ---");

  // Tampilkan Kelembaban Tanah
  display.setCursor(0, 16);
  display.print("Soil Moist: ");
  display.print(soilMoisturePercentage);
  display.println(" %");

  // Tampilkan Suhu
  display.setCursor(0, 32);
  display.print("Temp      : ");
  display.print(temperature, 1); // 1 angka di belakang koma
  display.println(" C");

  // Tampilkan Status Pompa
  display.setCursor(0, 48);
  display.print("Pump      : ");
  display.println(PUMP_STATUS ? "ON" : "OFF");

  display.display(); // Terapkan perubahan ke layar
  sendToThingSpeak(
soilMoisturePercentage,
temperature,
PUMP_STATUS
);
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

Serial.println();
Serial.println("WiFi Connected");
ThingSpeak.begin(client);
  
  dht.begin();
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, LOW);

  // Inisialisasi OLED dengan alamat I2C 0x3C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Berhenti jika OLED gagal inisialisasi
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 24);
  display.println("Connecting...");
  display.display();

}

unsigned long lastUpdate = 0;

void loop()
{
  if (millis() - lastUpdate >= 16000)
  {
    lastUpdate = millis();
    updateSystem();
  }
}
