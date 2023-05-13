#ifndef MAX_TOPIC_SIZE
#define MAX_TOPIC_SIZE 40
#endif

#ifndef UNDEF_INPUT
#define UNDEF_INPUT 255
#endif

#define UNDEF_POSITION 211
#define MAX_OPEN_POSITION 100
#define MIN_OPEN_POSITION 0

enum state : const uint8_t
{
    STATE_OFF,
    STATE_1,
    STATE_2,
    STATE_NOCHG,
    STATE_ERR
};

struct Win_act_telem
{
    uint8_t state = 255;  /* Up/Down/ Off */
    uint8_t reason = 255; /* What triggered the button */
    float position = 0;   /* Windows position on a scal 0-100 */
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