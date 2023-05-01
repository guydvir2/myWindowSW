// #pragma once

struct Win_act_telem
{
    uint8_t state = 255;  /* Up/Down/ Off */
    uint8_t reason = 255; /* What triggered the button */
    bool lockdown_state = false;
};
struct Win_props
{
    const char *name;
    uint8_t id;
    uint8_t inpins[2];
    uint8_t outpins[2];
    uint8_t inpins2[2];

    bool extSW = false;
    bool lockdown = false;
    bool virtCMD = false;
};

enum REASONS : const uint8_t
{
    TIMEOUT,
    BUTTON,
    BUTTON2,
    LCKDOWN,
    MQTT
};
enum WIN_STATES : const uint8_t
{
    STOP,
    UP,
    DOWN,
    ERR
};