#include <ClickEncoder.h>

#include <TimerOne.h>

// include the library code:
//#include <U8glib.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 16, en = 17, d5 = 23, d6 = 25, d7 = 27, d8 = 29;
LiquidCrystal lcd(rs, en, d5, d6, d7, d8);

ClickEncoder *encoder;

void timerIsr() {
  encoder->service();
}


#define BEEPER_PIN        37

#define BTN_EN1           31
#define BTN_EN2           33
#define BTN_ENC           35

#define SD_DETECT_PIN     49
//#define KILL_PIN          41

#define LCD_BACKLIGHT_PIN 39



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
long PAUSE_BW_ROTATION = 1000;
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

void startModeMenu(){
  int16_t prev_state, curr_state;
  prev_state = -1;
  curr_state = -1;
  bool isButtonPressed = false;
  bool isKnobTurned = false;
  int btn_enc_val;
  String modes[4] = {"Test", "Calibration", "Automatic", "Manual"};
  MODE = 0;
  lcd.setCursor(0,1);
  lcd.print(modes[MODE]);
  while(!isButtonPressed){
    curr_state = curr_state+(encoder->getValue());
//    lcd.setCursor(0, 2);
//    lcd.print(prev_state);
//    lcd.setCursor(0, 3);
//    lcd.print(curr_state);
      lcd.setCursor(0, 2);
      lcd.print(MODE);
      
    if(curr_state != prev_state) {
      
      if(curr_state > prev_state){
        MODE += 1;
      
      }
      else{
        MODE += 3;
      }
      MODE = MODE%4;
      updateModeMenu(modes[MODE]);
      prev_state = curr_state;
    }

    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {
      lcd.setCursor(8,4);
      switch (b) {
        case ClickEncoder::Clicked:
          showSelectedMode(modes[MODE]);
          isButtonPressed = true;
      }
    }
  }
  startModeSubMenu(MODE);
}

bool confirmOrCancelMenu(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ready to start?");
  lcd.setCursor(0,1);
  lcd.print("Yes");
  int16_t prev_state, curr_state;
  prev_state = 0;
  curr_state = 0;
  bool isButtonPressed = false;
  bool startJob = true;
  String option;
  while(!isButtonPressed){
          curr_state = curr_state+encoder->getValue();     
          
          if(curr_state != prev_state) {
            startJob = !startJob;
            if(startJob){
              option = "Yes";
            }
            else {
              option = "No" ;
            }
            updateConfirmOrCancelMenu(option);
            prev_state = curr_state;
          }
      
          ClickEncoder::Button b = encoder->getButton();
          if (b != ClickEncoder::Open) {
            switch (b) {
              case ClickEncoder::Clicked:
                isButtonPressed = true;
            }
          }
        }
  return startJob;
}

void startModeSubMenu(int MODE){
  switch(MODE){
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Calibrating Mode");
      break;

    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Automatic Mode");
      startRotationAngleMenu(ANGLE_PER_ROTATION);
      startRotationPauseMenu(PAUSE_BW_ROTATION);
      break;
      
    case 3:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Manual Mode");
      startRotationAngleMenu(ANGLE_PER_ROTATION);
      startRotationPauseMenu(PAUSE_BW_ROTATION);
      startRotationBufferMenu(ROTATION_BUFFER_SIZE);
      startRotationSpeedMenu(SPEED_ROTATION);
      startRotationDirMenu(DIR);
      startRotationNumberMenu(NUM_ROTATION);
      break;
  }
    
}

void startRotationAngleMenu(int ANGLE_PER_ROTATION) {
        int16_t prev_state, curr_state;
        prev_state = 0;
        curr_state = 0;
        bool isButtonPressed = false;
        bool isKnobTurned = false;
        updateRotationAngleMenu(ANGLE_PER_ROTATION);
        while(!isButtonPressed){
          curr_state = curr_state+encoder->getValue();     
          
          if(curr_state != prev_state) {
            
            if(curr_state > prev_state){
              ANGLE_PER_ROTATION += 2;
            }
            else{
              ANGLE_PER_ROTATION += 358;
            }
            ANGLE_PER_ROTATION = ANGLE_PER_ROTATION%360;
            updateRotationAngleMenu(ANGLE_PER_ROTATION);
            prev_state = curr_state;
          }
      
          ClickEncoder::Button b = encoder->getButton();
          if (b != ClickEncoder::Open) {
            switch (b) {
              case ClickEncoder::Clicked:
                isButtonPressed = true;
            }
          }
        }
    
}

void startRotationPauseMenu(int PAUSE_BW_ROTATION) {
        PAUSE_BW_ROTATION = PAUSE_BW_ROTATION/100;
        int16_t prev_state, curr_state;
        prev_state = 0;
        curr_state = 0;
        bool isButtonPressed = false;
        bool isKnobTurned = false;
        updateRotationPauseMenu(PAUSE_BW_ROTATION);
        while(!isButtonPressed){
          curr_state = curr_state+encoder->getValue();     
          
          if(curr_state != prev_state) {
            
            if(curr_state > prev_state){
              PAUSE_BW_ROTATION += 2;
            }
            else{
              PAUSE_BW_ROTATION += 598;
            }
            
            PAUSE_BW_ROTATION = PAUSE_BW_ROTATION%600;
            updateRotationPauseMenu(PAUSE_BW_ROTATION);
            prev_state = curr_state;
          }
      
          ClickEncoder::Button b = encoder->getButton();
          if (b != ClickEncoder::Open) {
            switch (b) {
              case ClickEncoder::Clicked:
                
                isButtonPressed = true;
            }
          }
        }

        PAUSE_BW_ROTATION = PAUSE_BW_ROTATION*100;
}

void startRotationBufferMenu(int ROTATION_BUFFER_SIZE) {
        int16_t prev_state, curr_state;
        prev_state = 0;
        curr_state = 0;
        bool isButtonPressed = false;
        bool isKnobTurned = false;
        updateRotationBufferMenu(ROTATION_BUFFER_SIZE);
        while(!isButtonPressed){
          curr_state = curr_state+encoder->getValue();     
          
          if(curr_state != prev_state) {
            
            if(curr_state > prev_state){
              ROTATION_BUFFER_SIZE += 1;
            }
            else{
              ROTATION_BUFFER_SIZE += 49;
            }
            ROTATION_BUFFER_SIZE = ROTATION_BUFFER_SIZE%50;
            updateRotationBufferMenu(ROTATION_BUFFER_SIZE);
            prev_state = curr_state;
          }
      
          ClickEncoder::Button b = encoder->getButton();
          if (b != ClickEncoder::Open) {
            switch (b) {
              case ClickEncoder::Clicked:
                isButtonPressed = true;
            }
          }
        }
    
}

void startRotationSpeedMenu(int SPEED_ROTATION) {
        SPEED_ROTATION -= 5;
        int speed_temp = 15-SPEED_ROTATION;
        int16_t prev_state, curr_state;
        prev_state = 0;
        curr_state = 0;
        bool isButtonPressed = false;
        bool isKnobTurned = false;
        updateRotationSpeedMenu(SPEED_ROTATION);
        while(!isButtonPressed){
          curr_state = curr_state+encoder->getValue();     
          
          if(curr_state != prev_state) {
            
            if(curr_state > prev_state){
              speed_temp += 1;
            }
            else{
              speed_temp += 15;
            }
            speed_temp = speed_temp%16;
            updateRotationSpeedMenu(speed_temp);
//            lcd.setCursor(0,4);
//            lcd.print((15-speed_temp)+5);
            prev_state = curr_state;
          }
      
          ClickEncoder::Button b = encoder->getButton();
          if (b != ClickEncoder::Open) {
            switch (b) {
              case ClickEncoder::Clicked:
                isButtonPressed = true;
            }
          }
        }
        SPEED_ROTATION = 15-speed_temp;
        SPEED_ROTATION += 5;  
}

void startRotationDirMenu(int DIR) {
        int16_t prev_state, curr_state;
        prev_state = 0;
        curr_state = 0;
        bool isButtonPressed = false;
        bool isKnobTurned = false;
        updateRotationDirMenu(DIR);
        while(!isButtonPressed){
          curr_state = curr_state+encoder->getValue();     
          
          if(curr_state != prev_state) {
            
            DIR += 1;
            DIR = DIR%2;
            updateRotationDirMenu(DIR);
            prev_state = curr_state;
          }
      
          ClickEncoder::Button b = encoder->getButton();
          if (b != ClickEncoder::Open) {
            switch (b) {
              case ClickEncoder::Clicked:
                isButtonPressed = true;
            }
          }
        }
    
}

void startRotationNumberMenu(int NUM_ROTATION) {
        int16_t prev_state, curr_state;
        prev_state = 0;
        curr_state = 0;
        bool isButtonPressed = false;
        bool isKnobTurned = false;
        updateRotationNumberMenu(NUM_ROTATION);
        while(!isButtonPressed){
          curr_state = curr_state+encoder->getValue();     
          
          if(curr_state != prev_state) {
            
            if(curr_state > prev_state){
              NUM_ROTATION += 1;
            }
            else{
              NUM_ROTATION += 999;
            }
            NUM_ROTATION = NUM_ROTATION%1000;
            updateRotationNumberMenu(NUM_ROTATION);
            prev_state = curr_state;
          }
      
          ClickEncoder::Button b = encoder->getButton();
          if (b != ClickEncoder::Open) {
            switch (b) {
              case ClickEncoder::Clicked:
                isButtonPressed = true;
            }
          }
        }
    
}

void updateModeMenu(String option){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Mode Menu");
  lcd.setCursor(0,1); 
  lcd.print(option);
}

void showSelectedMode(String MODE){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("selected MODE: ");  
  lcd.setCursor(0,1);
  lcd.print(MODE);
}


void updateConfirmOrCancelMenu(String option){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ready to start?");
  lcd.setCursor(0,1);
  lcd.print(option);
}
void updateRotationAngleMenu(int ANGLE_PER_ROTATION){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set parameter:");
  lcd.setCursor(0,1);
  lcd.print("Angle per rotation =");
  lcd.setCursor(0, 2);
  lcd.print(ANGLE_PER_ROTATION);
  lcd.setCursor(4, 2);
  lcd.print("Deg");
}

void updateRotationPauseMenu(int PAUSE_BW_ROTATION){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set parameter:");
  lcd.setCursor(0,1);
  lcd.print("Pause bw rotation =");
  lcd.setCursor(0, 2);
  lcd.print(PAUSE_BW_ROTATION);
  lcd.setCursor(5, 2);
  lcd.print("x100ms");
}

void updateRotationBufferMenu(int ROTATION_BUFFER_SIZE){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set parameter:");
  lcd.setCursor(0,1);
  lcd.print("Rotation buffer =");
  lcd.setCursor(0, 2);
  lcd.print(ROTATION_BUFFER_SIZE);
}

void updateRotationSpeedMenu(int SPEED_ROTATION){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set parameter:");
  lcd.setCursor(0,1);
  lcd.print("Rotation speed =");
  lcd.setCursor(0, 2);
  lcd.print(SPEED_ROTATION);
}

void updateRotationDirMenu(int DIR){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set parameter:");
  lcd.setCursor(0,1);
  lcd.print("Rotation direction =");
  lcd.setCursor(0, 2);
  if (DIR==1){
    lcd.print("Clockwise");
  }
  else{
    lcd.print("Counter-clockwise");
  }
}

void updateRotationNumberMenu(int NUM_ROTATION){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set parameter:");
  lcd.setCursor(0,1);
  lcd.print("Rotation number =");
  lcd.setCursor(0, 2);
  lcd.print(NUM_ROTATION);
}



void setup() {
    // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.print("Welcome!");

//  int16_t prev_state, curr_state;
  encoder = new ClickEncoder( BTN_EN1,BTN_EN2,BTN_ENC,4);

  Timer1.initialize(2000);
  Timer1.attachInterrupt(timerIsr); 
  
//  prev_state = -1;
//  curr_state = -1;

//  encoder->setAccelerationEnabled(!encoder->getAccelerationEnabled());
  
  
  //clear board memory
//  for (int i=0; i<EEPROM.length(); i++) {
//    EEPROM.write(i, 0);
//    LED_blink(1,0,1);
//  }
 
  if (DEBUG){
    Serial.begin(9600);
  }

  //pinMode(KILL_PIN, INPUT);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RAMP_LED_PIN, OUTPUT);

//  bool isResetPressed = false;
//  while (!isResetPressed){
//    startModeMenu();
//    lcd.clear();
//    ClickEncoder::Button b = encoder->getButton();
//    if (b != ClickEncoder::Open) {
//      lcd.setCursor(8,4);
//      switch (b) {
//        case ClickEncoder::Held:
//          isResetPressed = true;
//      }
//    }
//  }

  startModeMenu();
  
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
  
  bool start = confirmOrCancelMenu();
  if(!start){
      //loop back to setup
      setup();
  } 
  else { 
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
  //  lcd.setCursor(0, 1);
  //  // print the number of seconds since reset:
  //  lcd.print(millis() / 1000);
    digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
  
    
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
  //  for (;;);
    setup(); 
  }
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

