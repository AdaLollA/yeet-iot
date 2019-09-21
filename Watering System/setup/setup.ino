#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <PolledTimeout.h>

const char *ssid = "WhereIsAlex";
const char *pswd = "yeeyeeboys";
const char *mqtt_server = "iotgateway";

const String TEMPERATURE_TOPIC = "arduino1/temperature"; // this is the [root topic]
const String BRIGHTNESS_TOPIC = "arduino1/brightness";
const String HUMIDITY_TOPIC = "arduino1/humidity";
const String WATER_PUMP_TOPIC = "arduino1/water_pump";

#define TIME_BETWEEN_MESSAGES 15 * 60 * 1000

#define BOARD_MODE_NETWORK 0
#define BOARD_MODE_TEMPERATURE 1
#define BOARD_MODE_BRIGHTNESS 2
#define BOARD_MODE_HUMIDITY 3
#define BOARD_MODE_PUMP 4

//// Temperature
#define SDA_PIN 4
#define SCL_PIN 5
const int16_t I2C_MASTER = 0x42;
const int16_t I2C_SLAVE = 0x48;

//// Brightness
#define PHOTORESISTOR_PIN 16

//// Humidity
const byte HUMIDITY_PIN = A0;

//// Pump
#define PUMP_HIGH 0
#define PUMP_LOW 2
#define TOGGLE 14

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, callback, espClient);
long lastMsg = 0;
int state = BOARD_MODE_NETWORK;
int lastRun = 0;
int pumpStart = 0;
int mqttPump = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pswd);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  Wire.begin(SDA_PIN, SCL_PIN, I2C_MASTER);

  // Brightness
  pinMode(PHOTORESISTOR_PIN, INPUT);

  // Pump
  pinMode(PUMP_HIGH, OUTPUT);
  pinMode(PUMP_LOW, OUTPUT);
  pinMode(TOGGLE, INPUT);

  digitalWrite(PUMP_LOW, LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pump:");
  String val = "";
  for (int i = 0; i < length; i++) {
    val += (char)payload[i];
  }
  Serial.println(val.toInt());

  mqttPump = val.toInt();
  pumpStart = millis();
}

String macToStr(const uint8_t *mac)
{
  String result;
  for (int i = 0; i < 6; ++i)
  {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

String composeClientID()
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String clientId;
  clientId += "esp-";
  clientId += macToStr(mac);
  return clientId;
}


void subscribeToBroker(String topic) {
  client.subscribe(topic.c_str());
  Serial.print("Subscribed to: ");
  Serial.println(topic);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    String clientId = composeClientID();
    clientId += "-";
    clientId += String(micros() & 0xff, 16); // to randomise. sort of

    // Attempt to connect
    if (client.connect(clientId.c_str(), "iot", "iotgateway"))
    {
      Serial.println("connected");
      subscribeToBroker(WATER_PUMP_TOPIC);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.print(" wifi=");
      Serial.print(WiFi.status());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

double calculateC(int pre, int post)
{
  return post == 0 ? pre : pre + (post / 254.0);
}

void temperatureLoop()
{
  Serial.println("pinging...");
  Wire.requestFrom(I2C_SLAVE, 2); // request 6 bytes from slave device #8

  int pre = 0;
  int post = 0;

  if (Wire.available())
  { // slave may send less than requested
    int c = Wire.read(); // receive a byte as character

    if (pre == 0)
    {
      pre = c;
    }
    else
    {
      post = c;
    }

    double value = calculateC(pre, post);
    Serial.print("Temperature: ");
    Serial.println(value); // print the character


    client.publish((char*) TEMPERATURE_TOPIC.c_str() , (char*) String(value).c_str(), true);
  }
}

void brightnessLoop()
{
  Serial.print("Brightness: ");
  String payload = "";

  if (digitalRead(PHOTORESISTOR_PIN) == 0)
  {
    Serial.println("It's bright!");
    payload += "HIGH";
  }

  else
  {
    Serial.println("It's dark.");
    payload += "LOW";
  }

  client.publish((char*) BRIGHTNESS_TOPIC.c_str() , (char*) payload.c_str(), true);
}

void humidityLoop()
{
  int value = analogRead(HUMIDITY_PIN);
  float humidity = (1023.0 - value) / 1023.0;

  Serial.print("Humidity: ");
  Serial.println(humidity);

  client.publish((char*) HUMIDITY_TOPIC.c_str() , (char*) String(humidity).c_str(), true);
}

void pumpLoop()
{
  digitalWrite(PUMP_HIGH, digitalRead(TOGGLE) || mqttPump);
}

void loop()
{
  switch (state)
  {
    case BOARD_MODE_NETWORK:
      // confirm still connected to mqtt server
      if (!client.connected())
      {
        reconnect();
      }

      state = BOARD_MODE_TEMPERATURE;
      break;

    case BOARD_MODE_TEMPERATURE:
      temperatureLoop();
      state = BOARD_MODE_BRIGHTNESS;
      break;

    case BOARD_MODE_BRIGHTNESS:
      brightnessLoop();
      state = BOARD_MODE_HUMIDITY;
      break;

    case BOARD_MODE_HUMIDITY:
      humidityLoop();
      lastRun = millis();
      state = BOARD_MODE_PUMP;
      break;

    case BOARD_MODE_PUMP:
      client.loop();
      pumpLoop();
      break;
  }

  if(state == BOARD_MODE_PUMP && pumpStart + (1000 * mqttPump) < millis()) {
    mqttPump = 0;
  }

  if(state == BOARD_MODE_PUMP && lastRun + TIME_BETWEEN_MESSAGES < millis()) {
    state = BOARD_MODE_NETWORK;
  }
}