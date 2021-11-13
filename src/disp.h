
#include "mqtt.h"

void display_begin();

void draw_message(const String &string);
void draw_main_screen(MQTT_Data* data, bool can_control_terget, int selected);
void draw_menu_screem(MQTT_Data* data, int selected);