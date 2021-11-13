
#include <Arduino.h>
#include <WiFi.h>
#include <M5Stack.h>

#include "disp.h"
#include "creds.h"
#include "ui.h"

void log_ram() {
  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());
}

void setup(void) {
  M5.begin(true, false, true, false);
  M5.Speaker.setVolume(1);

  ui_setup();
  mqtt_setup();
  display_begin();

  WiFi.setHostname(MQTT_ID);
  WiFi.begin(SSID, WIFI_PASS);
  draw_message("Connecting to WiFi...");
  WiFi.waitForConnectResult();

  M5.Speaker.tone(1000, 50);
  delay(50);
  M5.Speaker.tone(1000, 50);
}

void loop() { // 240 x 320
  
  if (!WiFi.isConnected()) {
    draw_message("WiFi failure, restarting...");
    delay(10000);
    ESP.restart();
  }
    
  if (!mqtt_is_connected()) {
    draw_message("Connecting to MQTT...");
    if (!mqtt_connect()) {
      draw_message("Reconnecting to MQTT...");
      delay(10000);
      return;
    }
  }

  ui_loop();
  mqtt_loop();

  delay(10);
}