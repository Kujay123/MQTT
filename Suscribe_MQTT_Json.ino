#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ====== Konfigurasi OLED ======
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====== Konfigurasi WiFi ======
const char* ssid     = "TECNO CAMON 40 Pro 5G";
const char* password = "santrifiwa";

// ====== Konfigurasi MQTT ======
const char* mqtt_server = "broker.hivemq.com";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "/azka";

WiFiClient espClient;
PubSubClient client(espClient);

String clientId;   // Client ID dibuat dari MAC address

// Variabel untuk menyimpan data terakhir
String suhuVal = "--";
String kelembapanVal = "--";

// ====== Koneksi WiFi ======
void setup_wifi() {
  Serial.println();
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi terhubung!");
}

// ====== Fungsi untuk Menampilkan ke OLED ======
void tampilkanKeOLED(String suhu, String kelembapan) {
  display.clearDisplay();
  
  // Header / Judul
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 0);
  display.println("MONITORING RUANGAN");
  
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE); // Garis pemisah

  // Tampilan Suhu
  display.setCursor(0, 18);
  display.print("Suhu :");
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print(suhu);
  display.setTextSize(1);
  display.print(" C");

  // Tampilan Kelembapan
  display.setCursor(70, 18);
  display.print("Kelembapan:");
  display.setTextSize(2);
  display.setCursor(70, 30);
  display.print(kelembapan);
  display.setTextSize(1);
  display.print(" %");

  // Footer status
  display.fillRect(0, 52, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK); // Teks hitam di atas background putih
  display.setCursor(4, 54);
  display.print("Status: Terhubung");

  display.display();
}

// ====== Callback saat pesan MQTT diterima ======
void callback(char* topic, byte* payload, unsigned int length) {
  String pesan = "";
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  Serial.print("Pesan diterima: ");
  Serial.println(pesan);

  // Parsing JSON sederhana
  int idxSuhu = pesan.indexOf("suhu");
  int idxKelembapan = pesan.indexOf("kelembapan");

  if (idxSuhu != -1 && idxKelembapan != -1) {
    int startSuhu = pesan.indexOf(":", idxSuhu) + 1;
    int endSuhu = pesan.indexOf(",", startSuhu);
    if (endSuhu == -1) endSuhu = pesan.indexOf("}", startSuhu);
    suhuVal = pesan.substring(startSuhu, endSuhu);
    suhuVal.trim();

    int startKelembapan = pesan.indexOf(":", idxKelembapan) + 1;
    int endKelembapan = pesan.indexOf("}", startKelembapan);
    kelembapanVal = pesan.substring(startKelembapan, endKelembapan);
    kelembapanVal.trim();

    // Tampilkan data terbaru ke OLED
    tampilkanKeOLED(suhuVal, kelembapanVal);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    if (client.connect(clientId.c_str())) {
      Serial.println("terhubung!");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi OLED (Alamat I2C umum: 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 alokasi gagal"));
    for(;;);
  }

  // Tampilan awal saat booting
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.println("Memulai ESP32...");
  display.display();
  delay(2000);

  setup_wifi();

  clientId = "ESP32Client-" + WiFi.macAddress();
  clientId.replace(":", "");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
