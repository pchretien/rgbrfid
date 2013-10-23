// IO pin definitions
#define RFID_CONTROL_PIN 2
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN 11
#define READY_LED_PIN 13
#define RX_PIN 8
#define TX_PIN 1

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
  NONE,
  RED,
  GREEN,
  BLUE,
  YELLOW,
  BLACK,
  WHITE,
  RANDOM,
  EMPTY
};

void postMessage(QMessage msg);
QMessage getMessage();

