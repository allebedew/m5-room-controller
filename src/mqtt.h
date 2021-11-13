
#define NO_DATA -1000

struct MQTT_Data {
    float outside_temp;

    float room_temp;
    int room_humidity;
    int room_co2;
    int room_voc;
    float room_light;

    float heater_target;
    bool heater_enabled;
    bool heater_state;

    bool ac_connected;
    bool ac_light;
    bool ac_swing;
    int ac_mode;
    int ac_temp;

    int curt1_position;
    int curt1_target;
    int curt1_state;

    int curt2_position;
    int curt2_target;
    int curt2_state;
};

void mqtt_setup();
void mqtt_loop();

bool mqtt_is_connected();
bool mqtt_connect();
bool mqtt_data_was_changed();
MQTT_Data* mqtt_data();

bool mqtt_can_control_heater_terget();
bool mqtt_can_control_ac_terget();
bool mqtt_set_target_inc(bool up);
void mqtt_set_heater_toggle();
bool mqtt_set_ac_toggle();

void mqtt_set_heater_target(float temp);
void mqtt_set_heater_enabled(bool on);

void mqtt_set_ac_light(bool on);
void mqtt_set_ac_swing(bool on);
void mqtt_set_ac_mode(int mode);
void mqtt_set_ac_temp(int temp);
