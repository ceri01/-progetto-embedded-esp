#include <ESP8266WiFi.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>
#include <StreamUtils.h>

#include "Secrets.h"
#include "Config.h"

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

HammingStream<7, 4> eccSerial(Serial);

Scheduler runner;
Task receiveDataTask(COMMUNICATION_INTERVAL, TASK_FOREVER, &receiveData);

void receiveData() {
	mqttClient.poll();
	StaticJsonDocument<128> doc;
	deserializeJson(doc, eccSerial);

	mqttSend("north", doc["north"]);
	mqttSend("south", doc["south"]);
	mqttSend("east", doc["east"]);
	mqttSend("west", doc["west"]);
	mqttSend("wind", doc["wind"]);
}

void mqttSend(const char *field, uint8_t value) {
	mqttClient.beginMessage((String(MQTT_TOPIC) + String(field)).c_str());
	mqttClient.print(value);
	mqttClient.endMessage();
}

void setup() {
	Serial.begin(BAUD_RATE, SERIAL_7N1);
	delay(1000);

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}

	if (!mqttClient.connect(MQTT_HOST, MQTT_PORT)) {
		ESP.reset();
	}

	runner.init();
	runner.addTask(receiveDataTask);
	receiveDataTask.enable();
}

void loop() {
	runner.execute();
}