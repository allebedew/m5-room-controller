
#include <Arduino.h>
#include <M5Stack.h>

#include "ui.h"
#include "disp.h"

void ui_setup() {

}

int screen = 0;
int selected = -1;
bool state_changed = false;
unsigned long last_action = 0;

bool ui_state_was_changed() {
    if (state_changed) {
        state_changed = false;
        return true;
    }
    return false;
}

void beep(int tone) {
    switch (tone) {
        case 0: M5.Speaker.tone(1500, 50); break;
        case 1: M5.Speaker.tone(2000, 50); break;
    }
}

void select_button_pressed() {
    switch (screen) {
        case 0:
            switch (selected) {
                case -1: 
                    selected = 0; 
                    beep(1); 
                    break;
                case 0: 
                    mqtt_set_heater_toggle(); 
                    beep(0); 
                    break;
                case 1: 
                    if (mqtt_set_ac_toggle()) { beep(0); }
                    break;
                case 2: 
                    screen = 1; 
                    selected = 0; 
                    beep(1);
                    break;
            }
            break;
        case 1: {
            switch (selected) {
                case 0: {
                    screen = 0;
                    selected = -1;
                    beep(1);
                    break;
                }
                case 3: {
                    mqtt_set_ac_light(!mqtt_data()->ac_light);
                    beep(0);
                    break;
                }
                case 4: {
                    mqtt_set_ac_swing(!mqtt_data()->ac_swing);
                    beep(0);
                    break;
                }
            }
            break;
        }
    }
}

int last_brightness = 0;

void side_button_pressed(bool right) {
    switch (screen) {
        case 0:
            switch (selected) {
                case -1:
                    if (mqtt_set_target_inc(right)) { M5.Speaker.tone(1500, 50); }
                    break;
                default:
                    selected += right ? 1 : -1;
                    if (selected >= 3) { selected = -1; }
                    beep(1);
                    break;
            }
            break;
        case 1: 
            selected += right ? 1 : -1;
            if (selected < 0 || selected > 7) { selected = 0; }
            beep(1);
            break;
    }
}

void ui_loop() { // 240 x 320

  M5.update(); 
  
  if (M5.BtnB.wasPressed()) {
      select_button_pressed();
      state_changed = true;
      last_action = millis();
  }
  if (M5.BtnA.wasPressed()) {
      side_button_pressed(false);
      state_changed = true;
      last_action = millis();
  }
  if (M5.BtnC.wasPressed()) {
      side_button_pressed(true);
      state_changed = true;
      last_action = millis();
  }

  if ((millis() - last_action) > 30000 && (screen != 0 || selected != -1)) {
      screen = 0;
      selected = -1;
      state_changed = true;
      log_d("screen reset by timeout");
  }
  
    if (mqtt_data_was_changed() || ui_state_was_changed()) {
        switch (screen) {
            case 0: {
                bool can_control = mqtt_can_control_heater_terget() || mqtt_can_control_ac_terget();
                draw_main_screen(mqtt_data(), can_control, selected);
                break;
            }
            case 1:
                draw_menu_screem(mqtt_data(), selected);
                break;
        }
    }

  int brightness = 5;
  int light = mqtt_data()->room_light;
  if (light >= 20) {
      brightness = 255;
  } else if (light == NO_DATA || light >= 1) {
      brightness = 150;
  }
  if (brightness != last_brightness) {
      last_brightness = brightness;
      M5.Lcd.setBrightness(brightness);
      log_d("set brightness = %d", brightness);
  }
}
