#include <EEPROM.h>

#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MIN_PIN           3
#define X_MAX_PIN           2

#define RELAY_PIN          11
#define RAMP_LED_PIN       13

#define NUM_STEP_REVOL   3200

boolean DEBUG = true;

//in DEBUG mode, print in serial monitor
void dbgmsg(char* str)
{
  if (DEBUG) {
    Serial.println(str);
  }
}

//All capitalized (undefined) variables are the ones pre-set according to user input. 
//The assigned values are default.

//to be able to stop the motor; 1: enabled, 0: disabled
int ENABLED = 1;
//MODE = 0, device checking mode where the motor will be turned on until being disabled
//MODE = 1, checking mode where the table will slowly rotate for one revolution for user 
//to adjust the camera angle, camera setting, etc.
//MODE = 2, the normal function mode, 
//required input: ANGLE_PER_ROTATION, PAUSE_BW_ROTATION;
//MODE = 3, the manual mode, ALL parameters are required, according to user input
int MODE = 2;
//The angle for each rotation, set to 5 degrees
int ANGLE_PER_ROTATION = 10;
//The delay between each rotations, set to 1000 ms
int PAUSE_BW_ROTATION = 1000;
//The direction of rotating, 1: clockwise, 0: counterclockwise
int DIR = 1;

//only in manual mode that the following is pre-set by user input.

//The buffer size when rotating, a larger value will make the acceleration and deccleration more gradual
int ROTATION_BUFFER_SIZE = 15;
//The speed level of rotation, lower limit 5; the smaller the faster
int SPEED_ROTATION = 5;
//The number of rotations
int NUM_ROTATION = 3;

int num_rotation;
int num_step_full;
int num_step_partial;
int pause_bw_rotation;
int rotation_buffer_size;
int speed_rotation;

void setup() {
  //clear board memory
  for (int i=0; i<EEPROM.length(); i++) {
    EEPROM.write(i, 0);
    LED_blink(1,0,1);
  }
 
  if (DEBUG){
    Serial.begin(9600);
  }
  
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RAMP_LED_PIN, OUTPUT);

  //motor activate and inactivate
  if (ENABLED==1) {
    digitalWrite(X_ENABLE_PIN, LOW);
    dbgmsg("Motor...enabled");
  }
  else {
    digitalWrite(X_ENABLE_PIN, HIGH);
    dbgmsg("Motor...disabled");
    for (;;);
  }

  //assign parameter based on MODE
  switch(MODE) { 
    case 1:
      num_rotation = 1;
      num_step_full = NUM_STEP_REVOL;
      pause_bw_rotation = 0;
      rotation_buffer_size = 13;
      speed_rotation = 5;
      dbgmsg("MODE 1...checked");
      break;
    
    case 2:
      num_rotation = (int)ceil((float)360/ANGLE_PER_ROTATION);
      num_step_full = (int)ceil(((float)ANGLE_PER_ROTATION)/360*NUM_STEP_REVOL);
      num_step_partial = (int)(360%11/(float)360*NUM_STEP_REVOL);
      pause_bw_rotation = PAUSE_BW_ROTATION;
      rotation_buffer_size = 13;
      speed_rotation = 5;
      dbgmsg("MODE 2...checked");
      break;
  
    case 3 :
      num_rotation = NUM_ROTATION;
      num_step_full = (int)ceil(((float)ANGLE_PER_ROTATION)/360*NUM_STEP_REVOL);
      pause_bw_rotation = PAUSE_BW_ROTATION;
      rotation_buffer_size = ROTATION_BUFFER_SIZE;
      speed_rotation = SPEED_ROTATION;
      dbgmsg("MODE 3...checked");
      break;
  
    case 0:
      num_rotation = 10000;
      num_step_full = NUM_STEP_REVOL;
      pause_bw_rotation = 0;
      rotation_buffer_size = 13;
      speed_rotation = 5;
      dbgmsg("MODE 0...checked");
      break;
  
    default:
      digitalWrite(X_ENABLE_PIN, HIGH);
      dbgmsg("MODE not accepted");
      LED_blink(500, 500, 3);
      for (;;); 
  }

  //check direction input
  if (DIR==0) {
    digitalWrite(X_DIR_PIN, HIGH);
    dbgmsg("DIR 0...checked");
  }
  else if (DIR==1) {
    digitalWrite(X_DIR_PIN, LOW);
    dbgmsg("DIR 1...checked");
  }
  else {
    digitalWrite(X_ENABLE_PIN, HIGH);
    dbgmsg("DIR not accepted");
    LED_blink(500, 500, 5);
    for (;;);
  }

  //check speed input
  if (speed_rotation<1) {
    digitalWrite(X_ENABLE_PIN, HIGH);
    dbgmsg("speed not accepted");
    LED_blink(100, 100, 7);
    for (;;);
  }
  else if (speed_rotation<5) {
    dbgmsg("speed may be too fast");
  }
  else {
    dbgmsg("speed...checked");
  }

  //check buffer size
  if (rotation_buffer_size>((int)((float)num_step_full)) &&
      rotation_buffer_size>((int)((float)num_step_partial))){
    digitalWrite(X_ENABLE_PIN, HIGH);
    dbgmsg("Buffer size too large. It should be smaller than num_step/2");
    LED_blink(100, 100, 3);
    for (;;);
  }
  else {
    dbgmsg("Buffer size...checked");
  }

  //display settings
//  msg(DEBUG, DIR, MODE, ANGLE_PER_ROTATION, num_rotation, num_step_full, num_step_partial, 
//      pause_bw_rotation, rotation_buffer_size, speed_rotation);
}

void loop()
{ 
  dbgmsg("Start");
  char count[20];
  if (MODE==2) {
    for (int i=0; i<num_rotation; i++) {
      sprintf(count, "Rotation Count...%d/%d", i+1, num_rotation);
      dbgmsg(count);
      //Do partial rotation if in last rotation
      if (i==num_rotation-1) {
        digitalWrite(LED_BUILTIN, HIGH);
        rotate(num_step_partial, rotation_buffer_size, speed_rotation);
        digitalWrite(LED_BUILTIN, LOW);
      }
      //Do full rotation if not in last rotation
      else {
        digitalWrite(LED_BUILTIN, HIGH);
        rotate(num_step_full, rotation_buffer_size, speed_rotation);
        digitalWrite(LED_BUILTIN, LOW);
      }
      trigger_camera(3000, pause_bw_rotation);
//      delay(pause_bw_rotation); 
    }
  }
  else {
    for (int i=0; i<num_rotation; i++) {
      sprintf(count, "Rotation Count...%d/%d", i+1, num_rotation);
      dbgmsg(count);
      digitalWrite(LED_BUILTIN, HIGH);
      rotate(num_step_full, rotation_buffer_size, speed_rotation);
      digitalWrite(LED_BUILTIN, LOW);
      delay(pause_bw_rotation); 
    }
  }
  dbgmsg("Finish");
  digitalWrite(X_ENABLE_PIN, HIGH);
  for (;;); 
}
  
void rotate(int num_step, int buffer_size, int speed_rotation)
{ 
  for(int i=0; i<num_step; i++)
  { 
    //Handle accelration and decceleration
    int d = speed_rotation;
    if (i<buffer_size) {
      d = buffer_size+speed_rotation-i;
    }
    else if (i>num_step-buffer_size) {
      d = buffer_size+speed_rotation-1-(num_step-i);
    }
    
    //Do a step
    digitalWrite(X_STEP_PIN, HIGH);   
    delay(d); 
    digitalWrite(X_STEP_PIN, LOW); 
    delay(d); 
  } 
} 

//blink green LED; different speed and number of blink indicates respective error
void LED_blink(int ontime, int offtime, int num_blink)
{
  for (int i=0; i<num_blink; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(ontime);
      digitalWrite(LED_BUILTIN, LOW);
      delay(offtime);
    }
}

void msg(int DEBUG, int MODE, int DIR, int ANGLE_PER_ROTATION, int num_rotation, int num_step_full, 
         int num_step_partial, int pause_bw_rotation, int rotation_buffer_size, int speed_rotation)
{
  char str[500];
  sprintf(str, 
          "DIR...%d\nMODE...%d\nDEBUG...%d\nRotation_Angle...%d\
          \nNum_Rotation...%d\nNum_Step_Full...%d\nNum_Step_Partial...%d\
          \nPause_bw_Rotation...%d\nRotation_Buffer_Size...%d\nSpeed_Rotation...%d\n",
          MODE, DIR, DEBUG, ANGLE_PER_ROTATION, num_rotation, num_step_full, num_step_partial, 
          pause_bw_rotation,rotation_buffer_size, speed_rotation);
  Serial.print(str);
}

void trigger_camera(int relay_ontime, int relay_offtime){
      digitalWrite(RELAY_PIN, HIGH);
      delay(relay_ontime);
      LED_blink(100,1,1);
      digitalWrite(RELAY_PIN, LOW);
      delay(relay_offtime);
  }

