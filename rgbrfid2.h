// IO pin definitions
#define RFID_CONTROL_PIN 9
#define RED_PIN 3
#define GREEN_PIN 5
#define BLUE_PIN 6
#define READY_LED_PIN 7
#define RX_PIN 8
#define TX_PIN 1
#define PIEZO_PIN 10

// Serial protocol control bytes
#define SERIAL_OPCODE 10
#define SERIAL_ENDCODE 13
#define KEY_LENGTH 10
#define ACTIVATE_RFID_WAIT 5 // milliseconds

// Queue metrics
#define QUEUE_LENGTH 16

#define SLOW_FADE 10
#define FAST_FADE 1
#define MAX_NO_DISK_COUNT 200

enum QMessage
{
  NONE,//0
  RED,
  GREEN,//2
  BLUE,
  YELLOW,//4
  BLACK,
  WHITE,//6
  RANDOM,
  EMPTY//8
};

void play(QMessage msg);
void postMessage(QMessage msg);
QMessage getMessage();
void rememberMessage(QMessage msg);

