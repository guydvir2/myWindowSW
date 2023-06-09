#ifndef MAX_TOPIC_SIZE
#define MAX_TOPIC_SIZE 40
#endif

#ifndef UNDEF_INPUT
#define UNDEF_INPUT 255
#endif

#define UNDEF_POSITION 211

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
    bool newMSG = false;
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
struct winPosition
{
    bool is_rotating = false;
    float current_position = 0.0; // now position 0-100
    float request_position = 0.0; // cmd to position
    float last_position = 0.0;    // last saved position
    float init_position = 0;
    unsigned long start_clk = 0;
};
struct motorProperties
{
    const uint8_t MAX_OPEN_POSITION = 100;
    const uint8_t MIN_OPEN_POSITION = 0;
    float UP_DURATION = 60.0;             // set by user for each window (seconds)
    float DOWN_DURATION = 60.0;           // set by user for each window (seconds)
    float STALL_SEC = 0.0;             // time untill motor actually moves (for now same up & down)
    float EXTRA_TIME_END = 0.0; // give extra time to make sure when it come to 0 or 100
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