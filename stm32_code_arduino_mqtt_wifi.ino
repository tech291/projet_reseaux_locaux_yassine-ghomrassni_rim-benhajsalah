
#include <SPI.h>
#include <WiFiST.h>
#include <Wire.h>
#include <HTS221Sensor.h>
#include <LPS22HBSensor.h>
#include <PubSubClient.h>

// ========= CONFIGURATION WIFI ============
char ssid[] = "Redmi 12";
char pass[] = "rimrimrimredmi";

// ============ CONFIGURATION MQTT ============
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* client_id = "STM32_IOT_yassin_rym_real";

// Topics MQTT
const char* topic_temp = "/iot/yassin_rym/temperature";
const char* topic_hum = "/iot/yassin_rym/humidite";
const char* topic_press = "/iot/yassin_rym/pression";

// Pin definitions for USART1 (ST-Link VCP)
#define TX_PIN PB6
#define RX_PIN PB7


HardwareSerial SerialVCP(RX_PIN, TX_PIN);

// SPI3 pins & WiFi module pins

SPIClass SPI_3(PC12, PC11, PC10);
WiFiClass WiFi(&SPI_3, PE0, PE1, PE8, PB13);


TwoWire dev_i2c(PB11, PB10);  


HTS221Sensor *HumTemp;
LPS22HBSensor *PressTemp;

// MQTT Client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastMqttAttempt = 0;
unsigned long lastSensorRead = 0;

// ============ MQTT RECONNECT ============
void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  // Essayer de se reconnecter toutes les 5 secondes
  if (millis() - lastMqttAttempt < 5000) {
    return;
  }
  lastMqttAttempt = millis();
  
  SerialVCP.print("Connexion MQTT...");
  SerialVCP.flush();
  
  if (mqttClient.connect(client_id)) {
    SerialVCP.println(" OK!");
    SerialVCP.flush();
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    SerialVCP.print(" Echec (rc=");
    SerialVCP.print(mqttClient.state());
    SerialVCP.println(")");
    SerialVCP.flush();
  }
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PB14, OUTPUT);  // Green LED
  

  SerialVCP.begin(115200);
  delay(2000);
  
  SerialVCP.println("\n========================================");
  SerialVCP.println("WiFi + MQTT + Sensors - B-L475E-IOT01A");
  SerialVCP.println("========================================\n");
  SerialVCP.flush();
  
  // Fast blink to confirm startup
  for(int i = 0; i < 3; i++) {
    digitalWrite(PB14, HIGH);
    delay(100);
    digitalWrite(PB14, LOW);
    delay(100);
  }
  
  // INITIALIZE I2C AND SENSORS 
  SerialVCP.println("=== Initialisation I2C et Capteurs ===");
  SerialVCP.flush();
  dev_i2c.begin();
  delay(100);
  
  
  SerialVCP.println("Scan I2C...");
  SerialVCP.flush();
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++) {
    dev_i2c.beginTransmission(address);
    error = dev_i2c.endTransmission();
    
    if (error == 0) {
      SerialVCP.print("  Trouve: 0x");
      if (address < 16) SerialVCP.print("0");
      SerialVCP.print(address, HEX);
      
      if (address == 0x5F) SerialVCP.print(" (HTS221)");
      if (address == 0x5D) SerialVCP.print(" (LPS22HB)");
      
      SerialVCP.println();
      SerialVCP.flush();
      nDevices++;
    }
  }
  
  SerialVCP.print("  Total: ");
  SerialVCP.print(nDevices);
  SerialVCP.println(" peripherique(s)\n");
  SerialVCP.flush();
  
  // Initialize HTS221 (Temperature and Humidity)
  HumTemp = new HTS221Sensor(&dev_i2c);
  if (HumTemp->begin() != 0) {
    SerialVCP.println("  ERREUR: HTS221 init failed!");
  }
  delay(100);
  HumTemp->Enable();
  delay(200);
  
 
  PressTemp = new LPS22HBSensor(&dev_i2c);
  if (PressTemp->begin() != 0) {
    SerialVCP.println("  ERREUR: LPS22HB init failed!");
  }
  delay(100);
  PressTemp->Enable();
  delay(200);
  
  SerialVCP.end();
  delay(100);
  SerialVCP.begin(115200);
  delay(1000);
  
  SerialVCP.println("✓ HTS221 (Temp/Hum) OK");
  SerialVCP.println("✓ LPS22HB (Pression) OK");
  SerialVCP.flush();
  
  // Verify sensors
  uint8_t id_hts = 0;
  uint8_t id_lps = 0;
  HumTemp->ReadID(&id_hts);
  PressTemp->ReadID(&id_lps);
  
  SerialVCP.print("  HTS221 ID: 0x");
  SerialVCP.println(id_hts, HEX);
  SerialVCP.print("  LPS22HB ID: 0x");
  SerialVCP.println(id_lps, HEX);
  SerialVCP.println();
  SerialVCP.flush();
  
  // INITIALIZE WIFI
  SerialVCP.println("=== Initialisation WiFi ===");
  SerialVCP.flush();
  
  if (WiFi.status() == WL_NO_SHIELD) {
    SerialVCP.println("ERREUR: Module WiFi non trouve!");
    SerialVCP.flush();
    while (true) {
      digitalWrite(PB14, HIGH);
      delay(100);
      digitalWrite(PB14, LOW);
      delay(100);
    }
  }
  
  SerialVCP.println("✓ Module WiFi detecte");
  SerialVCP.flush();
  
  String fv = WiFi.firmwareVersion();
  SerialVCP.print("  Firmware: ");
  SerialVCP.println(fv);
  SerialVCP.flush();
  
  // Connect to WiFi
  SerialVCP.print("\nConnexion a: ");
  SerialVCP.println(ssid);
  SerialVCP.flush();
  
  status = WiFi.begin(ssid, pass);
  
  int attempts = 0;
  while (status != WL_CONNECTED && attempts < 30) {
    delay(1000);
    status = WiFi.status();
    SerialVCP.print(".");
    SerialVCP.flush();
    attempts++;
    digitalWrite(PB14, !digitalRead(PB14));
  }
  
  SerialVCP.println();
  SerialVCP.flush();
  
  if (status == WL_CONNECTED) {
    SerialVCP.println("\n========================================");
    SerialVCP.println("✓ WiFi Connecte!");
    SerialVCP.println("========================================");
    SerialVCP.flush();
    
    SerialVCP.print("SSID: ");
    SerialVCP.println(WiFi.SSID());
    
    IPAddress ip = WiFi.localIP();
    SerialVCP.print("IP: ");
    SerialVCP.print(ip[0]); SerialVCP.print(".");
    SerialVCP.print(ip[1]); SerialVCP.print(".");
    SerialVCP.print(ip[2]); SerialVCP.print(".");
    SerialVCP.println(ip[3]);
    
    long rssi = WiFi.RSSI();
    SerialVCP.print("Signal: ");
    SerialVCP.print(rssi);
    SerialVCP.println(" dBm\n");
    SerialVCP.flush();
    
    digitalWrite(PB14, HIGH);
    
    // CONFIGURATION MQTT
    SerialVCP.println("=== Configuration MQTT ===");
    SerialVCP.flush();
    
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setKeepAlive(60);
    
    SerialVCP.print("Broker: ");
    SerialVCP.println(mqtt_server);
    SerialVCP.print("Port: ");
    SerialVCP.println(mqtt_port);
    SerialVCP.println();
    SerialVCP.flush();
    
  } else {
    SerialVCP.println("WiFi connection failed!");
    SerialVCP.println("Continuing with sensors only\n");
    SerialVCP.flush();
  }
  
  SerialVCP.println("========================================");
  SerialVCP.println("  SYSTEME PRET - Debut des mesures");
  SerialVCP.println("========================================\n");
  SerialVCP.flush();
}

void loop() {
  unsigned long currentTime = millis();
  
  // LED heartbeat
  if (currentTime % 1000 < 500) {
    digitalWrite(PB14, HIGH);
  } else {
    digitalWrite(PB14, LOW);
  }
  
  // Manage MQTT connection
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      reconnectMQTT();
    } else {
      mqttClient.loop();
    }
  }
  
  if (currentTime - lastSensorRead >= 5000) {
    lastSensorRead = currentTime;
    
    // Read sensors
    float temperature = 0;
    float humidity = 0;
    float pressure = 0;
    
    uint8_t tempStatus = HumTemp->GetTemperature(&temperature);
    uint8_t humStatus = HumTemp->GetHumidity(&humidity);
    uint8_t pressStatus = PressTemp->GetPressure(&pressure);
    
    // Display readings
    SerialVCP.println("┌─────────────────────────────────┐");
    SerialVCP.println("│      MESURES CAPTEURS REELS     │");
    SerialVCP.println("├─────────────────────────────────┤");
    
    SerialVCP.print("│ Temperature: ");
    SerialVCP.print(temperature, 2);
    SerialVCP.print(" °C");
    if (tempStatus != 0) SerialVCP.print(" [ERR]");
    SerialVCP.println("      │");
    SerialVCP.flush();
    
    SerialVCP.print("│ Humidite   : ");
    SerialVCP.print(humidity, 2);
    SerialVCP.print(" %");
    if (humStatus != 0) SerialVCP.print(" [ERR]");
    SerialVCP.println("       │");
    SerialVCP.flush();
    
    SerialVCP.print("│ Pression   : ");
    SerialVCP.print(pressure, 2);
    SerialVCP.print(" hPa");
    if (pressStatus != 0) SerialVCP.print(" [ERR]");
    SerialVCP.println("  │");
    SerialVCP.flush();
    
    SerialVCP.println("└─────────────────────────────────┘");
    
    // WiFi status
    SerialVCP.print("WiFi: ");
    if (WiFi.status() == WL_CONNECTED) {
      SerialVCP.println("Connecte");
    } else {
      SerialVCP.println("Deconnecte");
    }
    SerialVCP.flush();
    
    // MQTT Publishing
    if (mqttClient.connected()) {
      SerialVCP.println("\n--- Publication MQTT ---");
      
      // Convert to strings
      char temp_str[10];
      char hum_str[10];
      char press_str[10];
      
      dtostrf(temperature, 4, 2, temp_str);
      dtostrf(humidity, 4, 2, hum_str);
      dtostrf(pressure, 6, 2, press_str);
      
      // Publish
      bool temp_ok = mqttClient.publish(topic_temp, temp_str);
      bool hum_ok = mqttClient.publish(topic_hum, hum_str);
      bool press_ok = mqttClient.publish(topic_press, press_str);
      
      if (temp_ok && hum_ok && press_ok) {
        SerialVCP.println(" Toutes les valeurs publiees!");
        SerialVCP.print("  Temp: "); SerialVCP.println(temp_str);
        SerialVCP.print("  Hum : "); SerialVCP.println(hum_str);
        SerialVCP.print("  Press: "); SerialVCP.println(press_str);
      } else {
        SerialVCP.println("✗ Echec publication MQTT");
      }
      
      SerialVCP.println("------------------------");
    } else {
      SerialVCP.println("\nMQTT: Non connecte");
    }
    
    SerialVCP.println();
    SerialVCP.flush();
  }
  
  delay(100);
}