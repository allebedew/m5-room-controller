
#include <Arduino.h>
#include <M5Stack.h>

#include "disp_helpers.h"
#include "disp.h"

#define LOW_COLOR M5.Lcd.color565(64, 64, 64)

TFT_eSprite sprite = TFT_eSprite(&M5.Lcd);

uint16_t voc_to_color(int voc) {
  if (voc == NO_DATA) { return DARKGREY; }
  if (voc >= 660) { return RED; }
  if (voc >= 220) { return YELLOW; }
  return WHITE;
}

uint16_t co2_to_color(int co2) {
  if (co2 == NO_DATA) { return DARKGREY; }
  if (co2 >= 1000) { return RED; }
  if (co2 >= 800) { return YELLOW; }
  return WHITE;
}

uint16_t hum_to_color(int hum) {
  if (hum == NO_DATA) { return DARKGREY; }
  if (hum > 60 || hum < 40) { return YELLOW; }
  return WHITE;
}

uint16_t out_temp_to_color(float temp) {
  if (temp == NO_DATA) { return DARKGREY; }
  if (temp >= 30) { return /*MAGENTA*/ M5.Lcd.color565(255, 0, 255); }
  if (temp >= 25) { return /*RED*/ M5.Lcd.color565(255, 0, 0); }
  if (temp >= 20) { return /*ORANGE*/ M5.Lcd.color565(255, 170, 0); }
  if (temp >= 10) { return /*YELLOW*/ M5.Lcd.color565(255, 255, 0); }
  if (temp >= 0) { return /*CYAN*/ M5.Lcd.color565(0, 255, 255); }
  return /*BLUE*/ M5.Lcd.color565(0, 0, 255);
}

uint16_t room_temp_to_color(float temp) {
  if (temp == NO_DATA) { return DARKGREY; }
  if (temp >= 28) { return /*MAGENTA*/ M5.Lcd.color565(255, 0, 255); }
  if (temp >= 27) { return /*RED*/ M5.Lcd.color565(255, 0, 127); }
  if (temp >= 26) { return /*RED*/ M5.Lcd.color565(255, 0, 0); }
  if (temp >= 25) { return /*RED*/ M5.Lcd.color565(255, 85, 0); }
  if (temp >= 24) { return /*ORANGE*/ M5.Lcd.color565(255, 170, 0); }
  if (temp >= 23) { return /*YELLOW*/ M5.Lcd.color565(255, 255, 0); }
  if (temp >= 22) { return /*CYAN*/ M5.Lcd.color565(0, 255, 255); }
  if (temp >= 21) { return /*CYAN*/ M5.Lcd.color565(0, 127, 255); }
  return /*BLUE*/ M5.Lcd.color565(0, 0, 255);
}

uint32_t temp_to_angle(float t) {
    t = min((float)28.0, max((float)20.0, t)) - (float)20.0;
    return 240 + t * 30;
}

void draw_temp_arc(float temp, float target_temp, uint32_t x, uint32_t y, bool can_control_terget, bool selected) {

  // ARC

  for (int i=20; i<=28; i++) {
    if (i < 28) {
      fillArc(sprite, x, y, temp_to_angle(i) + 1, 28, 80, 80, 10, LOW_COLOR);

      if (i < floorf(temp)) {
        fillArc(sprite, x, y, temp_to_angle(i) + 1, 28, 80, 80, 10, room_temp_to_color(temp));
      } else if (i == floorf(temp)) {
        uint32_t a = (temp - floor(temp)) * 28;
        fillArc(sprite, x, y, temp_to_angle(i) + 1, a, 80, 80, 10, room_temp_to_color(temp));
      }
    }

    // MARKERS

    sprite.setTextSize(1);
    sprite.setTextDatum(i < 24 ? CR_DATUM : i > 24 ? CL_DATUM : CC_DATUM);
    sprite.setTextColor(LOW_COLOR);
    sprite.drawNumber(i, arc_x(x, temp_to_angle(i), 85), arc_y(y, temp_to_angle(i), 90));
  }

  // ARROW
  if (can_control_terget) {
    draw_arc_arrow(sprite, x, y, temp_to_angle(target_temp), 90, 90, selected ? RED : LOW_COLOR, 10, 10, true);
  }
  // draw_arc_arrow(x, y, temp_to_angle(temp), 60, 60, RED, false);

  // TEMP TEXT
  
  char t1_s[5];
  char t2_s[5];
  if (temp == NO_DATA) {
    strcpy(t1_s, "--");
    strcpy(t2_s, ".-");
  } else {
    float t_fl = floorf(temp);
    float t_r = roundf((abs(temp) - abs(t_fl)) * 10);
    sprintf(t1_s, "%.0f", t_fl);
    sprintf(t2_s, ".%.0f", t_r);
  }

  sprite.setTextSize(5);
  int16_t w1 = sprite.textWidth(t1_s);
  sprite.setTextSize(3);
  int16_t w2 = sprite.textWidth(t2_s);
  int16_t dx = x-(w1+w2-4)/2;

  sprite.setTextDatum(BL_DATUM);
  sprite.setTextColor(room_temp_to_color(temp));
  sprite.setTextSize(5);
  sprite.drawString(t1_s, dx, y+11, 1);
  sprite.setTextSize(3);
  sprite.drawString(t2_s, dx+w1-4, y+10, 1);
}

void draw_titled_value( // 100x64
  const String &title, 
  const String &value, 
  uint16_t color, 
  uint32_t x, 
  uint32_t y, 
  uint8_t datum, 
  bool rect = false,
  bool selected = false
) {
  uint64_t h_color, v_color;
  if (rect) {
    if (selected) {
      sprite.fillRoundRect(x-50, y-7, 100, 64, 10, color);
      h_color = BLACK;
      v_color = BLACK;
    } else {
      sprite.drawRoundRect(x-50, y-7, 100, 64, 10, color);
      h_color = color;
      v_color = color;
    }
  } else {
    h_color = LOW_COLOR;
    v_color = color;
  }

  sprite.setTextDatum(datum);
  sprite.setTextSize(2);
  sprite.setTextColor(h_color);
  sprite.drawString(title, x, value.isEmpty() ? y+16 : y, 1);
  sprite.setTextSize(3);  
  sprite.setTextColor(v_color);
  sprite.drawString(value, x, y+25, 1);
} 

void display_begin() {
  sprite.setColorDepth(8);
  sprite.createSprite(320, 240);
}

const size_t ds_len = 10;
char ds[ds_len];

void data_str(const char *fmt, int v) {
  if (v == NO_DATA) {
    strcpy(ds, "--");
  } else {
    snprintf(ds, ds_len, fmt, v);  
  }
}

void data_str(const char *fmt, float v) {
  if (v == NO_DATA) {
    strcpy(ds, "--");
  } else {
    snprintf(ds, ds_len, fmt, v);  
  }
}

void draw_menu_item(int n, const String &title, const String &value, uint16_t color, bool selected) {
  const int h = 30;
  uint16_t t_color, v_color;
  if (selected) {
    sprite.fillRoundRect(10, n*h, 300, h, 8, LOW_COLOR);
    t_color = BLACK;
    v_color = BLACK;
  } else {
    t_color = LOW_COLOR;
    v_color = color;
  }
  sprite.setTextSize(2);
  sprite.setTextColor(t_color);
  sprite.setTextDatum(CL_DATUM);
  sprite.drawString(title, 20, n*h+h/2, 1);
  sprite.setTextColor(v_color);
  sprite.setTextDatum(CR_DATUM);
  sprite.drawString(value, 300, n*h+h/2, 1);
}

void draw_menu_screem(MQTT_Data* data, int selected) {
  sprite.fillSprite(BLACK);

  draw_menu_item(0, "Back", "", WHITE, selected == 0);

  draw_menu_item(1, "Curtains 1", "--%", WHITE, selected == 1);
  draw_menu_item(2, "Curtains 2", "--%", WHITE, selected == 2);

  draw_menu_item(3, "A/C Light", data->ac_light ? "ON" : "OFF", WHITE, selected == 3);
  draw_menu_item(4, "A/C Swing", data->ac_swing ? "ON" : "OFF", WHITE, selected == 4);

  data_str("%.1f", data->room_light);
  draw_menu_item(5, "Light", ds, WHITE, selected == 5);
  
  draw_menu_item(6, "Energy", "-- W", WHITE, selected == 6);
  draw_menu_item(7, "Water Supply", "--", WHITE, selected == 7);
  // draw_menu_item(8, "Boiler", "OFF", WHITE, selected == 8);
  // draw_menu_item(9, "PIR", "", WHITE, selected == 9);

  // CURTAINS -> CLOSE,20,40,60,80,OPEN
  // LIGHTS -> LIST OF LIGHTS, AMBIENT
  // A/C -> LIGHT, SWING
  // WATER -> SUPPLY, BOILER
  // PIR -> LIST PIRS
  // INFO -> ENERGY, LIGHT

  sprite.pushSprite(0, 0);
}

void draw_main_screen(MQTT_Data* data, bool can_control_terget, int selected) { // 240 x 320
  sprite.fillSprite(BLACK);

  float target = data->heater_enabled ? data->heater_target : (data->ac_mode > 0 ? data->ac_temp : NO_DATA);
  draw_temp_arc(data->room_temp, target, 160, 100, can_control_terget, selected == -1);

  data_str("%.1f", data->outside_temp);
  draw_titled_value("Outside", ds, out_temp_to_color(data->outside_temp), 0, 0, TL_DATUM);

  data_str("%d%%", data->room_humidity);
  draw_titled_value("Hum", ds, hum_to_color(data->room_humidity), 0, 65, TL_DATUM);

  data_str("%d", data->room_co2);
  draw_titled_value("CO2", ds, co2_to_color(data->room_co2), 320, 0, TR_DATUM);

  data_str("%d", data->room_voc);
  draw_titled_value("VOC", ds, voc_to_color(data->room_voc), 320, 65, TR_DATUM);

  uint16_t heater_color = LOW_COLOR;
  if (data->heater_enabled) {
    data_str("%.1f", data->heater_target);
    heater_color = data->heater_state ? RED : ORANGE;
  } else {
    strcpy(ds, "OFF");  
  }
  draw_titled_value("Heater", ds, heater_color, 50, 180, TC_DATUM, true, selected == 0);
  
  uint16_t ac_color = LOW_COLOR;
  if (data->ac_connected) {
    if (data->ac_mode == 0) {
      strcpy(ds, "OFF");
    } else {
      ac_color = data->ac_mode == 1 ? ORANGE : CYAN;
      data_str("%d", data->ac_temp);
    }
  } else {
    strcpy(ds, "N/A");
  }
  draw_titled_value("A/C", ds, ac_color, 50+10+100, 180, TC_DATUM, true, selected == 1);

  draw_titled_value("Menu", "", LOW_COLOR, 50+20+200, 180, TC_DATUM, true, selected == 2);

  sprite.pushSprite(0, 0);
}

void draw_message(const String &string) {
  sprite.fillSprite(BLACK);
  sprite.setTextSize(2);
  sprite.setTextColor(WHITE);
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString(string, 160, 120, 1);
  sprite.pushSprite(0, 0);
}
