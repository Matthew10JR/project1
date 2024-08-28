#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHTesp.h>
#include <ESP32Servo.h>
#include <Keypad.h>
#include <WiFiClientSecure.h> // Use this for secure connections

#define DHTPIN 4
#define PIRPIN 2
#define BUZZERPIN 16
#define SERVOPIN 18
#define SOUNDPIN 34
#define MQ2PIN 35
//#define LDRPIN 15
#define LEDPIN 5
#define I2C_SDA 21  // Replace with your SDA pin
#define I2C_SCL 22  // Replace with your SCL pin

const char* ssid = "Matthew";
const char* password = "B9111968";
const char* mqtt_server = "cb7241bddd9347ccb8b1403f41271f2b.s1.eu.hivemq.cloud";  // HiveMQ public broker
const char* mqtt_username = "hivemq.webclient.1724159031696";
const char* mqtt_password = "20egJl61p&:T,x.UPXRu";
const int mqtt_port = 8883;

WiFiClientSecure espClient;
PubSubClient client(espClient);

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHTesp dht;
Servo myservo;

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";
unsigned long lastMsg = 0;
int value = 0;

const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int tempThreshold = 37;
int humidityThreshold = 80;
int soundThreshold = 5000;
int gasThreshold = 2500;
//int ldrThreshold = 1000;
int motionDetected = 0;

String doorPassword = "1234";
String input = "";
bool doorLocked = false;

// Function declarations
void handleKeypadInput(char key);
void monitorSensors();
void monitorTemperatureHumidity();
void monitorMotion();
void monitorSound();
void monitorGas();
//void monitorLight();
void buzzAlert();
void buzzPattern(int delayTime, int repeat);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client";
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("garage/door");
      client.publish("ESP32Client","connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Example: Control the garage door via MQTT
  if (String(topic) == "garage/door") {
    if (messageTemp == "open") {
      myservo.write(180);
      lcd.print("Door opened via MQTT");
    } else if (messageTemp == "close") {
      myservo.write(0);
      lcd.print("Door closed via MQTT");
    }
  }
}

void setup() {
  //delay(500);
  Serial.begin(115200);
  setup_wifi();
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.begin(16, 2);
  lcd.backlight();
  dht.setup(DHTPIN, DHTesp::DHT22);
  myservo.attach(SERVOPIN);
  pinMode(PIRPIN, INPUT);
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(SOUNDPIN, INPUT);
  pinMode(MQ2PIN, INPUT);
  //pinMode(LDRPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  myservo.write(0);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void publishSensorData() {
  TempAndHumidity data = dht.getTempAndHumidity();
  String temperature = String(data.temperature);
  String humidity = String(data.humidity);
  
  // Publish temperature and humidity
  client.publish("home/garage/temperature", temperature.c_str());
  client.publish("home/garage/humidity", humidity.c_str());

  // Read and publish sound and gas sensor data
  int soundLevel = analogRead(SOUNDPIN);
  int gasLevel = analogRead(MQ2PIN);
  
  // Publish sound level
  client.publish("home/garage/sound", String(soundLevel).c_str());

  // Publish gas level
  client.publish("home/garage/gas", String(gasLevel).c_str());

  // Read and publish motion status
  int motionDetected = digitalRead(PIRPIN);
  String motionStatus = motionDetected ? "Motion Detected" : "No Motion";
  client.publish("home/garage/motion", motionStatus.c_str());

  // Debugging: Print motion status to Serial Monitor
  Serial.print("Motion Status: ");
  Serial.println(motionStatus);
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 1000)
  {
    char key = keypad.getKey();
  Serial.print("Key pressed: ");
  Serial.print(key);
  Serial.print("\n");
  if (key) {
    handleKeypadInput(key);
  }

  if (!doorLocked) {
    monitorSensors();
  }

  publishSensorData();

  delay(500);

  }
  
}

void handleKeypadInput(char key) {
  if (key == '#') {
    doorLocked = false;
    lcd.clear();
    lcd.print("Door closed");
    myservo.write(0);
    delay(2000);
    lcd.clear();
    return;
  }

  input += key;
  if (input.length() == 4) {
    if (input == doorPassword) {
      doorLocked = true;
      myservo.write(180);
      lcd.clear();
      lcd.print("Door opened");
      delay(3000);
      lcd.clear();
      input = "";
    } else {
      lcd.clear();
      lcd.print("Wrong password");
      delay(2000);
      lcd.clear();
      input = "";
    }
  }
}

void monitorSensors() {
  monitorTemperatureHumidity();
  monitorMotion();
  monitorSound();
  monitorGas();
  //monitorLight();
}

void monitorTemperatureHumidity() {
  TempAndHumidity data = dht.getTempAndHumidity();
  if (data.temperature > tempThreshold) {
    myservo.write(180);
    lcd.clear();
    lcd.print("Temp Alert :");
    lcd.setCursor(0, 1);
    lcd.print(data.temperature);
    buzzAlert();
    client.publish("home/garage/temperature", String(data.temperature).c_str());
  } else if (data.humidity > humidityThreshold) {
    myservo.write(180);
    lcd.clear();
    lcd.print("Humidity Alert :");
    lcd.setCursor(0, 1);
    lcd.print(data.humidity);
    buzzAlert();
    client.publish("home/garage/humidity", String(data.humidity).c_str());
  } else {
    lcd.clear();
    lcd.print("Temp : ");
    lcd.print(data.temperature);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humid: ");
    lcd.print(data.humidity);
    lcd.print(" %");

    client.publish("home/garage/temperature", String(data.temperature).c_str());
    client.publish("home/garage/humidity", String(data.humidity).c_str());
  }
}

unsigned long lastMotionTime = 0;
bool motionHandled = false;

void monitorMotion() {
  motionDetected = digitalRead(PIRPIN);
  
  if (motionDetected && !motionHandled) {
    Serial.println("Motion detected");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Motion Detected");
    lcd.setCursor(0, 1);
    lcd.print("Door opening");

    // Open the door
    myservo.write(180);
    lastMotionTime = millis(); // Record the time when motion was detected
    motionHandled = true; // Mark that motion has been handled
  }

  // Check if enough time has passed to close the door
  if (motionHandled && (millis() - lastMotionTime >= 10000)) {
    myservo.write(0); // Close the door
    Serial.println("Servo set to 0 degrees");
    lcd.clear();
    lcd.print("Door closed");
    motionHandled = false; // Reset motionHandled for next detection
  }
}

void monitorSound() {
  int soundLevel = analogRead(SOUNDPIN);
  if (soundLevel > soundThreshold) {
    myservo.write(180);
    lcd.clear();
    lcd.print("Sound Alert :");
    lcd.setCursor(0, 1);
    lcd.print(soundLevel);
    buzzAlert();
    client.publish("home/garage/sound", String(soundLevel).c_str());
  }
}

void monitorGas() {
  int gasLevel = analogRead(MQ2PIN);
  if (gasLevel > gasThreshold) {
    myservo.write(180);
    lcd.clear();
    lcd.print("Gas Alert :");
    lcd.setCursor(0, 1);
    lcd.print(gasLevel);
    buzzAlert();
    client.publish("home/garage/gas", String(gasLevel).c_str());
  }
}

//void monitorLight() {
//  int lightLevel = analogRead(LDRPIN);
//  if (lightLevel < ldrThreshold) {
//    lcd.clear();
//    lcd.print("Low Light Alert");
//    buzzAlert();
//    client.publish("home/garage/light", String(lightLevel).c_str());
//  }
//}

void buzzAlert() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZERPIN, HIGH);
    delay(100);
    digitalWrite(BUZZERPIN, LOW);
    delay(100);
  }
}

void buzzPattern(int delayTime, int repeat) {
  for (int i = 0; i < repeat; i++) {
    digitalWrite(BUZZERPIN, HIGH);
    delay(delayTime);
    digitalWrite(BUZZERPIN, LOW);
    delay(delayTime);
  }
}
