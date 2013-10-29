#include "rgbrfid2.h"
#include <SoftwareSerial.h>

#define DEBUG true
SoftwareSerial pinSerial(RX_PIN, TX_PIN);

boolean sound = false;
QMessage lastThreeMessages[3] = {NONE, NONE, NONE};

// Current and target colors
unsigned char colorPins[3] = {RED_PIN, GREEN_PIN, BLUE_PIN};
unsigned char currentColor[3] = {0,0,0};
unsigned char targetColor[3] = {255,255,255};

QMessage lastMessage = NONE;
unsigned char queueLength = 0;
QMessage* messageQueue = NULL;

int noDiskCount = 0;

// Initialization
void setup()
{
  // Serial communication for Debug
  if(DEBUG)
    Serial.begin(115200);

  Debug("", "Setup ...");
  
  // The digital pin #2 used to enable/disable the RFID reader
  pinMode(RFID_CONTROL_PIN,OUTPUT); 
  
  // Ready lLED
  pinMode(READY_LED_PIN, OUTPUT);
  
  // Piezo speaker ...
  pinMode(PIEZO_PIN, OUTPUT);
  
  activateRFID();

  // Initialize the SoftwareSerial on pin 8 to comm.
  // with the RFID reader at 2400 bauds.
  pinSerial.begin(2400);
  
  // Allocate memory
  messageQueue = new QMessage[QUEUE_LENGTH];
  for(int i=0; i<QUEUE_LENGTH; i++)
    messageQueue[i] = NONE;
}

unsigned char modulo = 1;
unsigned long cycles = 0;

void loop()
{  
  cycles++;
  
  char key[11] = {0,0,0,0,0,0,0,0,0,0,0};
  if( readSerial(key) > 0)
  {
    // We have a new key ...
    processKey(key);

    Debug("RFID Key: ", key);
  }
  else
  {
    noDiskCount++;
    if(noDiskCount > MAX_NO_DISK_COUNT)
    {
      // There are no more key on the RFID reader
      Debug("No disk ...", noDiskCount);
      
      noDiskCount = 0;
      postMessage(EMPTY);
      
      checkForTargets();
    }
  }
  
  readQueue();
  setLEDs();
}

void Debug(char* prefix, char* msg)
{
  if(DEBUG)
  {
    Serial.print(prefix);
    Serial.println(msg);
  }
}

void Debug(char* prefix, int val)
{
  if(DEBUG)
  {
    Serial.print(prefix);
    Serial.println(val, DEC);
  }
}

void activateRFID()
{
  // Activate the RFID reader
  digitalWrite(RFID_CONTROL_PIN, LOW); 
  delay(ACTIVATE_RFID_WAIT);
}

void deactivateRFID()
{
  // Activate the RFID reader
  digitalWrite(RFID_CONTROL_PIN, HIGH); 
  pinSerial.flush();
}

int readSerial(char* key)
{
  activateRFID();
  
  // Check if there is data comming from the RFID reader
  if(pinSerial.available()) 
  {
    // There is some data to read.
    // Resetting all values ...
    int inputVal = 0;     
    int byteCount = 0;
    char rfidKey[KEY_LENGTH]; 
   
   // Check for the opcode
    if((inputVal = pinSerial.read()) == SERIAL_OPCODE) 
    {
      // This is the beginning of a new code.
      // Reading all 10 bytes
      byteCount = 0; 
      while(byteCount < KEY_LENGTH) 
      {
        if( pinSerial.available()) 
        { 
          inputVal = pinSerial.read(); 
          if((inputVal == SERIAL_OPCODE) || (inputVal == SERIAL_ENDCODE)) 
          {
            // Break when receiving a control character
            break;
          } 
          
          // This is a valid character.
          // Add the value to the key ...
          rfidKey[byteCount] = inputVal;
          byteCount++;          
        } 
      } 
      
      if(byteCount == KEY_LENGTH) 
      {        
        // We have a complete key.
        strncpy(key, rfidKey, KEY_LENGTH);
        deactivateRFID();
        
        return byteCount;
      } 
    } 
  } 
  
  return 0;
}

void processKey(char* key)
{
  noDiskCount = 0;
  
  if(strncmp(key, "01068DBEF2", 10) == 0)
  {
    postMessage(BLACK);
  }
  else if(strncmp(key, "01068DC0BD", 10) == 0)
  {
    postMessage(WHITE);
  }
  else if(strncmp(key, "01068DF0A7", 10) == 0)
  {
    postMessage(RED);
  }
  else if(strncmp(key, "01068DC008", 10) == 0)
  {
    postMessage(BLUE);
  }
  else if(strncmp(key, "01068DBCE1", 10) == 0)
  {
    postMessage(GREEN);
  }
  else if(strncmp(key, "01068DD774", 10) == 0)
  {
    postMessage(YELLOW);
  }
}

void postMessage(QMessage msg)
{
  if(queueLength >= QUEUE_LENGTH)
  {
    // Flush the oldest event and shift all messages.
    Debug("Queue full ...", queueLength);
    
    shiftMessages();
  }
  
  messageQueue[queueLength++] = msg;
}

QMessage getMessage()
{
  if(queueLength > 0)
  {
    QMessage msg = messageQueue[0];
    shiftMessages();
    return msg;
  }
  
  return NONE;
}

void shiftMessages()
{
  if(queueLength == 0)
    return;
  
  for(int i=0; i<queueLength-1; i++)
    messageQueue[i] = messageQueue[i+1];
    
  queueLength -= 1;
}

void setReadyLED(QMessage msg)
{
  if(msg == EMPTY)
    digitalWrite(READY_LED_PIN, HIGH);
  else
    digitalWrite(READY_LED_PIN, LOW);   
}

unsigned long alternate = 0;
void setLEDs()
{  
  if(cycles % modulo != 0)
    return;
    
  if(modulo == FAST_FADE)
  {
    for(int i=0; i<3; i++)
    {
      if(currentColor[i] < targetColor[i])
        currentColor[i] += 1;
      else if(currentColor[i] > targetColor[i])
        currentColor[i] -= 1;
        
      analogWrite(colorPins[i], currentColor[i]);
    }
  }
  else
  {
    int i = alternate++ % 3;
    if(currentColor[i] < targetColor[i])
      currentColor[i] += 1;
    else if(currentColor[i] > targetColor[i])
      currentColor[i] -= 1;
      
    analogWrite(colorPins[i], currentColor[i]);
  }
}

void checkForTargets()
{
  boolean allTheSame = true;
  
  for(int i=0; i<3; i++)
  {
    if(currentColor[i] != targetColor[i])
    {
      allTheSame = false;
      break;
    }
  }
  
  if(allTheSame)
    postMessage(RANDOM);
}

void readQueue()
{
  while(queueLength > 0)
  {
    QMessage msg = getMessage();
    Debug("Message: ", msg);
    
    if(msg == RANDOM)
    {
      if(lastMessage == EMPTY)
      {
        modulo = SLOW_FADE;
        targetColor[0] = random(255);
        targetColor[1] = random(255);
        targetColor[2] = random(255);
      }
      
      continue;
    }
    
    if(msg == lastMessage)
      continue;
      
    // We have a new command ...
    play(msg);
    rememberMessage(msg);
      
    modulo = FAST_FADE;
      
    // Turn ON the LED if there is no disk in the lamp
    setReadyLED(msg);
    
    checkSpecialMessages();
        
    switch(msg)
    {
      case RED:
        targetColor[0] = 255;
        targetColor[1] = 0;
        targetColor[2] = 0;
      break;
      
      case GREEN:
        targetColor[0] = 0;
        targetColor[1] = 255;
        targetColor[2] = 0;
      break;
      
      case BLUE:
        targetColor[0] = 0;
        targetColor[1] = 0;
        targetColor[2] = 255;
      break;
      
      case YELLOW:
        targetColor[0] = 255;
        targetColor[1] = 200;
        targetColor[2] = 0;
      break;
      
      case WHITE:
        targetColor[0] = 255;
        targetColor[1] = 255;
        targetColor[2] = 255;
      break;
      
      case BLACK:
        targetColor[0] = 0;
        targetColor[1] = 0;
        targetColor[2] = 0;
      break;
      
      case EMPTY:
        modulo = SLOW_FADE;
        targetColor[0] = random(255);
        targetColor[1] = random(255);
        targetColor[2] = random(255);
      break;
    }
    
    lastMessage = msg;
  }
}

void playTone(int tone, int duration) 
{
  for (long i = 0; i < duration * 1000L; i += tone * 2) 
  {
    digitalWrite(PIEZO_PIN, HIGH);
    delayMicroseconds(tone);
    
    digitalWrite(PIEZO_PIN, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) 
{
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) 
  {
    if (names[i] == note) 
    {
      playTone(tones[i], duration);
    }
  }
}

void play(QMessage msg) 
{
  if(!sound)
    return;
    
  int length = 2; // the number of notes
  char notes[] = "cC "; // a space represents a rest
  int beats[] = { 1, 1 };
  int tempo = 100;

  if(msg == EMPTY)
  {
    notes[0] = 'C';
    notes[1] = 'c';
  }
  
  for (int i = 0; i < length; i++) 
  {
    if (notes[i] == ' ') 
    {
      delay(beats[i] * tempo); // rest
    } 
    else 
    {
      playNote(notes[i], beats[i] * tempo);
    }

    // pause between notes
    delay(tempo / 2); 
  }
}

void resetLastThreeMessages()
{
  lastThreeMessages[2] = NONE;
  lastThreeMessages[1] = NONE;
  lastThreeMessages[0] = NONE;
}

void rememberMessage(QMessage msg)
{
  if(msg == EMPTY || msg == RANDOM)
    return;
    
  lastThreeMessages[2] = lastThreeMessages[1];
  lastThreeMessages[1] = lastThreeMessages[0];
  lastThreeMessages[0] = msg;
  
  if(DEBUG)
  {
    Serial.print(lastThreeMessages[0]);
    Serial.print(" ");
    Serial.print(lastThreeMessages[1]);
    Serial.print(" ");
    Serial.println(lastThreeMessages[2]);
  }
}

void checkSpecialMessages()
{
  if(lastThreeMessages[0] == BLACK && lastThreeMessages[1] == BLACK && lastThreeMessages[2] == BLACK)
  {
    sound = false;
    resetLastThreeMessages();
  }
  
  if(lastThreeMessages[0] == WHITE && lastThreeMessages[1] == WHITE && lastThreeMessages[2] == WHITE)
  {
    sound = true;
    resetLastThreeMessages();
  }
  
  if(lastThreeMessages[0] == BLUE && lastThreeMessages[1] == GREEN && lastThreeMessages[2] == RED)
  {
    for(int i=0; i<6; i++)
    {      
      analogWrite(colorPins[0], 255);
      analogWrite(colorPins[1], 0);
      analogWrite(colorPins[2], 0);
      delay(111);
      
      analogWrite(colorPins[0], 0);
      analogWrite(colorPins[1], 255);
      analogWrite(colorPins[2], 0);
      delay(111);

      analogWrite(colorPins[0], 0);
      analogWrite(colorPins[1], 0);
      analogWrite(colorPins[2], 255);
      delay(111);
    }
    
    resetLastThreeMessages();    
  }
  
  if(lastThreeMessages[0] == RED && lastThreeMessages[1] == GREEN && lastThreeMessages[2] == BLUE)
  {
    for(int i=0; i<3; i++)
    {
      analogWrite(colorPins[0], 0);
      analogWrite(colorPins[1], 0);
      analogWrite(colorPins[2], 0);
      for(int j=0; j<90; j++)
      {
        analogWrite(colorPins[i], j);
        delay(40);
      }
      for(int j=90; j>0; j--)
      {
        analogWrite(colorPins[i], j);
        delay(40);
      }
    }
    
    resetLastThreeMessages(); 
  }
  
  if(lastThreeMessages[0] == BLACK && lastThreeMessages[1] == GREEN && lastThreeMessages[2] == RED)
  {
    for(int i=0; i<100; i++)
    {      
      analogWrite(colorPins[0], 255);
      analogWrite(colorPins[1], 255);
      analogWrite(colorPins[2], 255);
      delay(30);
      
      analogWrite(colorPins[0], 0);
      analogWrite(colorPins[1], 0);
      analogWrite(colorPins[2], 0);
      delay(30);
    }
    
    resetLastThreeMessages();    
  }
}
