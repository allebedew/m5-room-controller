
#include <PubSubClient.h>
#include <WiFi.h>

#include "creds.h"
#include "mqtt.h"

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

const size_t topic_len = 100;
char topic[100];
const size_t msg_len = 25;
char msg[25];

MQTT_Data* data = (MQTT_Data*) malloc(sizeof(MQTT_Data));

bool changed = false;

void format_topic(const char* device, const char* control, const char* meta, bool setter);
bool compare_topic(const char* device, const char* control, const char* topic);

bool mqtt_data_was_changed() {
  if (changed) {
    changed = false;
    return true;
  }
  return false;
}

MQTT_Data* mqtt_data() {
    return data;
}

bool mqtt_is_connected() {
  return mqtt.connected();
}

bool mqtt_connect() {
  if (!WiFi.isConnected() || mqtt.connected()) { return true; }
  log_d("MQTT disconnented, connecting...");

  if (mqtt.connect(MQTT_ID, MQTT_LOGIN, MQTT_PASS)) {
    log_d("MQTT connected!");

    format_topic("weather", "Temperature", "", false);
    mqtt.subscribe(topic);

    format_topic("sensor_living", "Temperature", "", false);
    mqtt.subscribe(topic);
    format_topic("sensor_living", "CO2", "", false);
    mqtt.subscribe(topic);
    format_topic("sensor_living", "Air Quality (VOC)", "", false);
    mqtt.subscribe(topic);
    format_topic("sensor_living", "Humidity", "", false);
    mqtt.subscribe(topic);
    format_topic("sensor_living", "Illuminance", "", false);
    mqtt.subscribe(topic);

    format_topic("_radiator_living", "Enabled", "", false);
    mqtt.subscribe(topic);
    format_topic("_radiator_living", "State", "", false);
    mqtt.subscribe(topic);
    format_topic("_radiator_living", "TargetTemp", "", false);
    mqtt.subscribe(topic);

    format_topic("_radiator_living", "Enabled", "", false);
    mqtt.subscribe(topic);
    format_topic("_radiator_living", "State", "", false);
    mqtt.subscribe(topic);
    format_topic("_radiator_living", "TargetTemp", "", false);
    mqtt.subscribe(topic);

    format_topic("air_esp_living", "Connected", "", false);
    mqtt.subscribe(topic);
    format_topic("air_esp_living", "Light", "", false);
    mqtt.subscribe(topic);
    format_topic("air_esp_living", "Swing", "", false);
    mqtt.subscribe(topic);
    format_topic("air_esp_living", "Temp", "", false);
    mqtt.subscribe(topic);
    format_topic("air_esp_living", "X-HM-Mode", "", false);
    mqtt.subscribe(topic);

    return true;
  }
  return false;
}

const size_t s_len = 10;
char s[s_len];

void mqtt_message_handler(char* rcv_topic, byte* payload, unsigned int length) {
  strncpy(msg, (char*)payload, length);
  msg[length] = '\0';

  if (compare_topic("weather", "Temperature", rcv_topic)) {
    float v = atof(msg);
    snprintf(s, s_len, "%.2f", v);
    data->outside_temp = v;

  // ================

  } else if (compare_topic("sensor_living", "Temperature", rcv_topic)) {
    float v = atof(msg);
    snprintf(s, s_len, "%.2f", v);
    data->room_temp = v;

  } else if (compare_topic("sensor_living", "CO2", rcv_topic)) {
    int v = atoi(msg);
    data->room_co2 = v;

  } else if (compare_topic("sensor_living", "Air Quality (VOC)", rcv_topic)) {
    int v = atoi(msg);
    data->room_voc = v;

  } else if (compare_topic("sensor_living", "Humidity", rcv_topic)) {
    int v = atoi(msg);
    data->room_humidity = v;

  } else if (compare_topic("sensor_living", "Illuminance", rcv_topic)) {
    int v = atof(msg);
    data->room_light = v;

  // ================

  } else if (compare_topic("_radiator_living", "Enabled", rcv_topic)) {
    int v = atoi(msg);
    data->heater_enabled = v == 1;

  } else if (compare_topic("_radiator_living", "State", rcv_topic)) {
    int v = atoi(msg);
    data->heater_state = v == 1;

  } else if (compare_topic("_radiator_living", "TargetTemp", rcv_topic)) {
    float v = atof(msg);
    data->heater_target = v;

  // ================

  } else if (compare_topic("air_esp_living", "Connected", rcv_topic)) {
    int v = atoi(msg);
    data->ac_connected = v == 1;

  } else if (compare_topic("air_esp_living", "Swing", rcv_topic)) {
    int v = atoi(msg);
    data->ac_swing = v == 1;

  } else if (compare_topic("air_esp_living", "Light", rcv_topic)) {
    int v = atoi(msg);
    data->ac_light = v == 1;

  } else if (compare_topic("air_esp_living", "X-HM-Mode", rcv_topic)) {
    int v = atoi(msg);
    data->ac_mode = v;

  } else if (compare_topic("air_esp_living", "Temp", rcv_topic)) {
    int v = atoi(msg);
    data->ac_temp = v;

  }

  if (!compare_topic("sensor_living", "Illuminance", rcv_topic)) {
    log_d("MQTT > %s -- %s", rcv_topic, msg);
    changed = true;
  }
}

// === SERVICE ===

void format_topic(const char* device, const char* control, const char* meta, bool setter) {
  if (*control) {
    if (*meta) {
      snprintf(topic, topic_len, "/devices/%s/controls/%s/meta/%s", device, control, meta);
    } else {
      if (setter) {
        snprintf(topic, topic_len, "/devices/%s/controls/%s/on", device, control);
      } else {
        snprintf(topic, topic_len, "/devices/%s/controls/%s", device, control);
      }
    }
  } else {
    if (*meta) {
      snprintf(topic, topic_len, "/devices/%s/meta/%s", device, meta);
    } else {
      snprintf(topic, topic_len, "/devices/%s", device);
    }
  }
}

bool compare_topic(const char* device, const char* control, const char* rcv_topic) {
  format_topic(device, control, "", false);
  return strcmp(topic, rcv_topic) == 0;
}

// === ACTIONS ===

bool mqtt_can_control_heater_terget() {
  return data->heater_target != NO_DATA && data->heater_enabled;
}

bool mqtt_can_control_ac_terget() {
  return data->ac_temp != NO_DATA && data->ac_mode > 0;
}

bool mqtt_set_target_inc(bool up) {
  if (mqtt_can_control_heater_terget()) {
    float temp = data->heater_target + (up ? 0.5 : -0.5);
    if (temp >= 20 || temp <= 28) { 
      mqtt_set_heater_target(temp);
      return true;
    }
  }
  if (mqtt_can_control_ac_terget()) {
    int temp = data->ac_temp + (up ? 1 : -1);
    if (temp >= 20 || temp <= 28) {
      mqtt_set_ac_temp(temp);
      return true;
    }
  }
  return changed;
}

void mqtt_set_heater_toggle() {
  mqtt_set_heater_enabled(!data->heater_enabled);
  
  if (data->ac_mode > 0) {
    mqtt_set_ac_mode(0);
  }
}

bool mqtt_set_ac_toggle() {
  if (!data->ac_connected || data->ac_mode == NO_DATA) { return false; }
  
  int mode = data->ac_mode + 1;
  if (mode >= 3) { mode = 0; };
  mqtt_set_ac_mode(mode);

  if (data->heater_enabled) {
    mqtt_set_heater_enabled(false);
  }

  return true;
}

// === REAL ACTIONS ===

void mqtt_set_heater_target(float temp) {
  log_d("%f", temp);
  format_topic("_radiator_living", "TargetTemp", "", true);
  sprintf(msg, "%.1f", temp);
  mqtt.publish(topic, msg);
}

void mqtt_set_heater_enabled(bool enabled) {
  log_d("%d", enabled);
  format_topic("_radiator_living", "Enabled", "", true);
  mqtt.publish(topic, enabled ? "1" : "0");
}

void mqtt_set_ac_light(bool on) {
  log_d("%d", on);
  format_topic("air_esp_living", "Light", "", true);
  mqtt.publish(topic, on ? "1" : "0");
}

void mqtt_set_ac_swing(bool on) {
  log_d("%d", on);
  format_topic("air_esp_living", "Swing", "", true);
  mqtt.publish(topic, on ? "1" : "0");
}

void mqtt_set_ac_mode(int mode) {
  log_d("%d", mode);
  format_topic("air_esp_living", "X-HM-Mode", "", true);
  sprintf(msg, "%d", mode);
  mqtt.publish(topic, msg);
}

void mqtt_set_ac_temp(int temp) {
  log_d("%d", temp);
  format_topic("air_esp_living", "Temp", "", true);
  sprintf(msg, "%d", temp);
  mqtt.publish(topic, msg);
}

// === LOOP ===

void mqtt_setup() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqtt_message_handler);

  data->outside_temp = NO_DATA;

  data->room_temp = NO_DATA;
  data->room_humidity = NO_DATA;
  data->room_co2 = NO_DATA;
  data->room_voc = NO_DATA;
  data->room_light = NO_DATA;

  data->heater_target = NO_DATA;
  data->heater_enabled = false;
  data->heater_state = false;
  
  data->ac_connected = false;
  data->ac_light = false;
  data->ac_swing = false;
  data->ac_temp = NO_DATA;
  data->ac_mode = NO_DATA;
}

void mqtt_loop() {
  mqtt.loop();
}
