#include <math.h>

#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MIN_PIN           3
#define X_MAX_PIN           2

#define NUM_STEP_REVOL   3200

//All capitalized (undefined) variables are the ones pre-set according to user input. 
//Before the part for the user input is coded, I assign them some values for testing.

//MODE = 0, device checking mode where the motor will be turned on until being disabled
//MODE = 1, checking mode where the table will slowly rotate for one revolution for user 
//  to adjust the camera angle, camera setting, etc.
//MODE = 2, the normal function mode, 
//  required input: ANGLE_PER_ROTATION, PAUSE_BW_ROTATION;
//MODE = 3, the manual mode, ALL parameters are required, according to user input
int MODE = 2;
//The angle for each rotation, set to 5 degrees
int ANGLE_PER_ROTATION = 5;
//The delay between each rotations, set to 1000 ms
int PAUSE_BW_ROTATION = 1000;

//only in manual mode that the following is pre-set by user input.

//The buffer size when rotating, a larger value will make the acceleration and deccleration more gradual
int ROTATION_BUFFER_SIZE;
//The speed level of rotation, lower limit 1
int SPEED_ROTATION;
//The direction of rotating
int DIR;
//The number of rotations
int NUM_ROTATION;

int num_rotation;
int num_step_full;
int num_step_partial;
int pause_bw_rotation;
int rotation_buffer_size;
int speed_rotation;

void setup() {
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(X_ENABLE_PIN, LOW);

  if (DIR==0) {
    pinMode(X_DIR_PIN, HIGH);
  }
  else if (DIR==1) {
    pinMode(X_DIR_PIN, LOW);
  }
  else {
    for (int i=0;i<5;i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
    digitalWrite(X_ENABLE_PIN, HIGH);
    exit(1); 
  }

  if (MODE==1) {
    num_rotation = 1;
    num_step_full = NUM_STEP_REVOL;
    pause_bw_rotation = 0;
    rotation_buffer_size = 20;
    speed_rotation = 5;
  }
  else if (MODE==2) {
    num_rotation = ceil(360/ANGLE_PER_ROTATION);
    num_step_full = ANGLE_PER_ROTATION/360*NUM_STEP_REVOL;
    num_step_partial = (360%ANGLE_PER_ROTATION)/360*NUM_STEP_REVOL;
    pause_bw_rotation = PAUSE_BW_ROTATION;
    rotation_buffer_size = 20;
    speed_rotation = 5;
  }
  else if (MODE==3) {
    num_rotation = NUM_ROTATION;
    num_step_full = ANGLE_PER_ROTATION/360*NUM_STEP_REVOL;
    pause_bw_rotation = PAUSE_BW_ROTATION;
    rotation_buffer_size = ROTATION_BUFFER_SIZE;
    speed_rotation = SPEED_ROTATION;
  }
  else if (MODE==0) {
    num_rotation = 10000;
    num_step_full = NUM_STEP_REVOL;
    pause_bw_rotation = 0;
    rotation_buffer_size = 20;
    speed_rotation = 5;
  }
  else {
    for (int i=0;i<3;i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
    digitalWrite(X_ENABLE_PIN, HIGH);
    exit(1); 
  }
}

void loop()
{ 
  if (MODE==2) {
     for (int i=0;i<num_rotation;i++) {
    
     }
  }
  else {
    for (int i=0;i<num_rotation;i++) {
    
    }
    rotate(3200);
    delay(pause_bw_rotation); 
  }
  exit(0);
}
  

void rotate(int steps)
{ 
  for(int i=0; i < steps; i++)
  { 
    // Handle accelration and decceleration in a very naive way (but it works)
    int d=2;
    if (i<10) d=12-i;
    if (i>steps-10) d=11-(steps-i);
    
    // Do a step
    digitalWrite(X_STEP_PIN, HIGH);   
    delay(1); 
    digitalWrite(X_STEP_PIN, LOW); 
    delay(1); 
  } 
} 
