#include <math.h>

#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MIN_PIN           3
#define X_MAX_PIN           2

#define NUM_STEP_REVOL   3200

//All capitalized (undefined) variables are the ones that varies according to user input. Before the part for the user input is coded, I assign them some values for testing.

//MODE = 1, checking mode where the table will slowly rotate for one revolution for user to adjust the camera angle, camera setting, etc.
//MODE = 2, the normal function mode
int MODE = 2;

//The angle for each turn, set to 5 degrees
float ANGLE_PER_TURN = 5.0;

//The delay between each turns, set to 1000 ms
int PAUSE_BW_TURN = 1000;

//The buffer size when rotating, a larger value will make the acceleration and deccleration more gradual, set to 10
int TURN_BUFFER_SIZE = 10;

//The number of revolution the motor will complete in total, set to 1.0
float NUM_REVOLUTION = 1.0;

float num_turn;

void setup() {
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW);
  num_turn = floor(NUM_REVOLUTION*360/ANGLE_PER_TURN);
}

void loop()
{ 
  for (int i=0;i<num_turn;i++) 
  {
    rotate(3200); // This will vary with your build, experiment or measure to find the right value
    delay(2000); 
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
