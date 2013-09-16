

//
// RGB & RFID Lamp
//

// Arduino PINs
#define RFID 2 // Pin used to enable / Disable the RFID reader
#define RED 9
#define GREEN 10
#define BLUE 11
#define PWR 3

// Different modes of operation
#define MODE_RANDOM 0
#define MODE_RFID 1
#define MODE_BUTTON 2

// True if the lamp is off
boolean isOn = true;

// Delay and timer to switch back to random color mode
#define RFID_DELAY 1000
unsigned long lastChange = 0;

// Delay between two color changes in random color mode
#define FADE_DELAY 100

// Turn off the LEDs after being idle delay milliseconds
#define IDDLE_DELAY 3600000 // 30 minutes
unsigned long iddleTimer = 0;
char lastKey[10];
  
// Keep track of the current mode
int mode = MODE_RANDOM;

// Random color values, targets and increments
int currentRGB[3] = {0,0,0};
int targetRGB[3] = {0,0,0};
int incrementRGB[3] = {0,0,0};

// Set one color at the time to smmooth the fading effect ...
unsigned long currentColor = 0;

// Power button
int powerButton = 0; // Debounces
int powerButtonMode = 0;


// Set new target colors for the random mode
void resetTargetColors()
{
  for(int i=0;i<3;i++)
  {
    targetRGB[i] = random(255);
    incrementRGB[i] = (targetRGB[i]>currentRGB[i])?1:-1;
  }
}

// The RFID tag has been removed ... switch to random mode
void switchToRandomMode()
{
  currentColor = 0;
  mode = MODE_RANDOM;
    
  if(isOn)
  {
    iddleTimer = millis();
  }
}

// An RFID tag has been placed on the reader.
// Ligth up the LEDs associated to this tag
void displayRFID(long red, long green, long blue)
{
  currentRGB[0] = red;
  currentRGB[1] = green;
  currentRGB[2] = blue;  
  
  resetTargetColors();
  
  setLEDs(currentRGB);
}

void setLEDs(int rgb[3])
{
  analogWrite(RED,   rgb[0]);
  analogWrite(GREEN, rgb[1]);
  analogWrite(BLUE,  rgb[2]);
}

void turnOff()
{
  isOn = false;
  
  currentRGB[0] = 0;
  currentRGB[1] = 0;
  currentRGB[2] = 0;
  
  analogWrite(RED,   0);
  analogWrite(GREEN, 0);
  analogWrite(BLUE,  0);
  powerButtonMode = 0;
}

void turnOn()
{
  isOn = true;
  iddleTimer = millis();
  powerButtonMode = 0;
  mode = MODE_RANDOM;
  resetTargetColors();
}

void executeCode(char* rfidKey)
{  
  if(strncmp(rfidKey,"01068DC0BD", 10) == 0)
  {
    displayRFID(255,0,0);
  }
  
  if(strncmp(rfidKey,"01068DBEF2", 10) == 0)
  {
    displayRFID(0,255,0);
  }
  
  if(strncmp(rfidKey,"01068DBCE1", 10) == 0)
  {
    displayRFID(0,0,255);
  }
  
  if(strncmp(rfidKey,"01068DF0A7", 10) == 0)
  {
    displayRFID(255,255,0);
  }
  
  if(strncmp(rfidKey,"01068DD774", 10) == 0)
  {
    displayRFID(80,0,80);
  }
  
  if(strncmp(rfidKey,"01068DC008", 10) == 0)
  {
    displayRFID(0,255,255);
  }
}

void readSerial()
{
  if(Serial.available() > 0) 
  {
    int  inputVal = 0; 
    char rfidKey[10]; 
    int byteCount = 0;
   
    if((inputVal = Serial.read()) == 10) 
    {
      byteCount = 0; 
      while(byteCount < 10) 
      {
        if( Serial.available() > 0) 
        { 
          inputVal = Serial.read(); 
          if((inputVal == 10) || (inputVal == 13)) 
          {
            break;
          } 
          rfidKey[byteCount] = inputVal;
          byteCount++;          
        } 
      } 
      
      // Check if we have a complete key
      if(byteCount == 10) 
      {
        
        // DEBUG
        //Serial.print("Tag key: ");
        //Serial.println(rfidKey);         
        
        if(isOn)
        {          
          powerButtonMode = 0;
          executeCode(rfidKey);
        }
        else
        {
          if(mode == MODE_RANDOM)
          {
            turnOn();
          }
        }
        
        // No matter what ... we reset the counter.
        lastChange = millis();
        mode = MODE_RFID;

        if(strncmp(lastKey, rfidKey, 10) != 0)
        {
          strncpy(lastKey, rfidKey, 10);
          iddleTimer = millis();
        }        
      } 
    } 
  } 
}

void powerButtonPressed()
{
  iddleTimer = millis();
  
  if(isOn)
  {
    if(mode == MODE_RFID)
    {
      turnOff();
      return;
    }
    
    mode = MODE_BUTTON;
    
    switch(++powerButtonMode)
    {
    case 1:
      displayRFID(255,0,0);
      break;
    case 2:
      displayRFID(0,255,0);
      break;
    case 3:
      displayRFID(0,0,255);
      break;
    case 4:
      turnOff();
      break;
    }
  }
  else
  {
    turnOn();
  }
}

void setup() 
{ 
  // Seed the random number generator
  randomSeed(analogRead(0));
  
  // Initiate the communication with the RFID reader
  Serial.begin(2400); 
  
  // The digital pin #2 used to enable/disable the RFID reader
  pinMode(RFID,OUTPUT); 
  digitalWrite(RFID, LOW); // Activate the RFID reader
  
  // At startup, the lamp is in random mode
  switchToRandomMode();
  
  // Button pin
  pinMode(PWR, INPUT);
  
  memset(lastKey, 0, 10);
}  

void loop() 
{      
  // Power button debounce
  if(digitalRead(PWR) == HIGH)
  {
    powerButton++;
  }
  else
  {
    if(powerButton > 25)
      powerButtonPressed();
      
    powerButton = 0;
  }
  
  if(millis() - iddleTimer > IDDLE_DELAY)
  {
    turnOff();
  }
  
  switch(mode)
  {
    case MODE_RANDOM:
      if(isOn && mode == MODE_RANDOM && (millis() %FADE_DELAY)==0)
      {
        currentColor++;
         
        if(currentRGB[currentColor%3] == targetRGB[currentColor%3])
          incrementRGB[currentColor%3] = 0;
           
        currentRGB[currentColor%3] += incrementRGB[currentColor%3];
        setLEDs(currentRGB);
         
        if(incrementRGB[0] == 0 && incrementRGB[1] == 0 && incrementRGB[2] == 0)
          resetTargetColors();
      }
      break;
      
    case MODE_RFID:
      if(millis() - lastChange > RFID_DELAY)
      {
        switchToRandomMode();
      }
      break;
      
    case MODE_BUTTON:
      memset(lastKey, 0, 10);
      break;
  };
  
  // Check for a tag from the RFID
  readSerial();
} 

