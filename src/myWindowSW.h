/* Written by Guy Dvir 30/08/2022
Its purpose is to control 3 state windows switch.
Features:
1) Auto off - to prevent holding relaty Up or down for infinite time
2) Lockdown - when signaled, shuts down window, and disables switch until lockdown is disabled.
3) Dual Sw - in case of need 2 switches to control the window.
4) Telemetry - psot change of state.
*/
#ifndef mywindowsw_h
#define mywindowsw_h

#include <Arduino.h>
#include "myW.h"
class RockerSW
{
#define PRESSED LOW
#define DEBOUNCE_MS 50

private:
    unsigned long _down_ms[2] = {0, 0};
    unsigned long _down_rel_ms[2] = {0, 0};
    bool _is_triggered[2] = {false, false};

    uint8_t _state[2] = {false, false};
    uint8_t _last_state[2] = {false, false};
    uint8_t _pins[2] = {UNDEF_INPUT, UNDEF_INPUT};

public:
    RockerSW();
    uint8_t get_SWstate();
    uint8_t get_pins(uint8_t i);
    void set_input(uint8_t upPin, uint8_t downPin, uint8_t active_dir = INPUT_PULLUP);

private:
    uint8_t _readPin(uint8_t i);
};

class WinSW
{
#define RELAY_ON HIGH
private:
    RockerSW *RockerSW_V[2]{};

    uint8_t _id = 0;
    static uint8_t _next_id; /* Instance counter */
    bool _uselockdown = false;
    bool _lockdownState = false;
    bool _motor_rotating = false;

    // Precentage calc parameters
    float WIN_UP_DURATION = 90.0;             // set by user for each window (seconds)
    float WIN_DOWN_DURATION = 90.0;           // set by user for each window (seconds)
    float _current_postion = 0.0;             // position 0-100
    float _requested_position = 0.0;          // cmd to position
    float _last_position = 0.0;               // last saved position
    float _motor_stall_sec = 0.0;             // time untill motor actually moves (for now same up & down)
    float _end_movement_extra_time_sec = 0.0; // give extra time to make sure when it come to 0 or 100
    float _start_position = 0;
    unsigned long _start_clk = 0;

public:
    bool virtCMD = false;
    bool useDebug = false;
    bool useExtSW = false;

    char ver[14] = "WinSW_v0.58";
    char name[MAX_TOPIC_SIZE];
    uint8_t outpins[2];

    Win_act_telem MSG;

public:
    WinSW(bool use_debug = false);
    bool loop();

    void init_lockdown();
    void release_lockdown();

    void set_id(uint8_t i);
    void set_name(const char *_name);
    void set_input(uint8_t upin, uint8_t dpin);
    void set_extras(bool useLockdown = true);
    void set_ext_input(uint8_t upi = UNDEF_INPUT, uint8_t dpin = UNDEF_INPUT);
    void set_output(uint8_t outup_pin = UNDEF_INPUT, uint8_t outdown_pin = UNDEF_INPUT);

    void set_Win_position(float position); /* Set Open Position 0-100*/
    void set_motor_properties(float to_to_up = 100, float time_to_down = 100, float stick_time = 0.5, float end_move_time = 0.5);

    void set_WINstate(uint8_t state, uint8_t reason); /* External Callback UP/DOWN/OFF */

    uint8_t get_id();
    uint8_t get_winState();   /* off, up, down */
    float get_Win_position(); /* value 0-100 */

    void clear_newMSG();
    void get_Win_props(Win_props &win_props);
    void print_preferences();

private:
    void _winUP();
    void _allOff();
    void _readSW();
    void _winDOWN();
    void _switch_window_state(uint8_t state, uint8_t i);

    void _stop_if_position();
    void _start_timing_movement();
    void _calc_current_position();
    void _validate_position_value(float &value);
};
#endif