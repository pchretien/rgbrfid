

//
// RGB & RFID Lamp
//

#define RFID 2 // Pin used to enable / Disable the RFID reader
#define RED 9
#define GREEN 10
#define BLUE 11

// Fifferent modes of operation
#define MODE_RANDOM 0
#define MODE_RFID 1

// Delay and timer to switch back to random color mode
#define RFID_DELAY 5000
unsigned long lastChange = 0;

// Delay between two color changes in random color mode
#define FADE_DELAY 100

// Keep track of the current mode
int mode = MODE_RANDOM;

// Random color values, targets and increments
int currentRGB[3] = {0,0,0};
int targetRGB[3] = {0,0,0};
int incrementRGB[3] = {0,0,0};

// Set one color at the time to smmooth the fading effect ...
long currentColor = 0;

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
}

// An RFID tag has been placed on the reader.
// Ligth up the LEDs associated to this tag
void switchColorFromRFID(long red, long green, long blue)
{
  mode = 1;
  lastChange = millis();
  
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

void setup() 
{ 
  randomSeed(analogRead(0));
  
  // Initiate the communication with the RFID reader
  Serial.begin(2400); 
  
  // The digital pin #2 used to enable/disable the RFID reader
  pinMode(RFID,OUTPUT); 
  digitalWrite(RFID, LOW); // Activate the RFID reader
  
  switchToRandomMode();
}  

void loop() 
{    
  if(mode == MODE_RFID && millis() - lastChange > RFID_DELAY)
    switchToRandomMode();
   
  if(mode == MODE_RANDOM && (millis() %FADE_DELAY)==0)
  {
    currentColor++;
     
    if(currentRGB[currentColor%3] == targetRGB[currentColor%3])
      incrementRGB[currentColor%3] = 0;
       
    currentRGB[currentColor%3] += incrementRGB[currentColor%3];
    setLEDs(currentRGB);
     
    if(incrementRGB[0] == 0 && incrementRGB[1] == 0 && incrementRGB[2] == 0)
      resetTargetColors();
  }      
   
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
      
      if(byteCount == 10) 
      {
        /*
        // DEBUG
        Serial.print("TAG code is: ");   // possibly a good TAG 
        Serial.println(code);            // print the TAG code 
        */
        
        if(strncmp(rfidKey,"01068DC0BD", 10) == 0)
        {
          switchColorFromRFID(255,0,0);
        }
        
        if(strncmp(rfidKey,"01068DBEF2", 10) == 0)
        {
          switchColorFromRFID(0,255,0);
        }
        
        if(strncmp(rfidKey,"01068DBCE1", 10) == 0)
        {
          switchColorFromRFID(0,0,255);
        }
        
        if(strncmp(rfidKey,"01068DF0A7", 10) == 0)
        {
          switchColorFromRFID(255,255,0);
        }
        
        if(strncmp(rfidKey,"01068DD774", 10) == 0)
        {
          switchColorFromRFID(80,0,80);
        }
        
        if(strncmp(rfidKey,"01068DC008", 10) == 0)
        {
          switchColorFromRFID(0,255,255);
        }
      } 
    } 
  } 
} 

