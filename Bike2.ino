#include <Joystick.h>

// Create the Joystick
Joystick_ Joystick;

const int JoyXPin = A1;
const int JoyYPin = A0;

const int HalPIN = PIN2;
const int AccelButton = 0;

const int NumButtons = 5;

int ControllerButtonPins[NumButtons] = { PIN4, PIN3, PIN5, PIN6, PIN7 };
int GCButtons[NumButtons] = { 1, 2, 3, 4, 5 };
int lastButtonReadings[NumButtons] = { 0, 0, 0, 0, 0 };



const int every_millis = 1000;
const int sensorReadCooldownMillis = 100;

/*
     3
|<------>|    
|----____|----____|----____|----____|
 ^   ^
 1   2 
1 - Accel_PWM_Frame_Pointer, where we are  currently. Right now we output a 1
2 - Accel_PWM_Frame_End, when we will switch to outputting a 0
3 - PWM_Frame_Length
*/

const int PWM_Frame_Length = 20;
int Accel_PWM_Frame_Pointer = 0;
int Accel_PWM_Frame_End = 0;

int clamp(int x, int min, int max) {
  if (x < min) {
    return min;
  } else if (x > max) {
    return max;
  }
  return x;
}



void setup() {

  pinMode(HalPIN, INPUT_PULLUP);
  for (int i = 0; i < NumButtons; i++) {
    pinMode(ControllerButtonPins[i], INPUT);
  }

  Serial.begin(9600);
  while (!Serial) {};

  attachInterrupt(digitalPinToInterrupt(HalPIN), tickReading, FALLING);

  // Joystick stuff
  Joystick.begin();
  Joystick.setXAxisRange(0, 960);
  Joystick.setYAxisRange(0, 1023);
}


long int lastTickTime = 0;
long int last_calculate_time = 0;

int ticks_per_sec = 0;
int last_hal_ticks = 0;
volatile int hal_ticks = 0;




void tickReading() {

  long int tickTime = millis();
  if (tickTime - lastTickTime > sensorReadCooldownMillis) {
    hal_ticks++;
    lastTickTime = tickTime;
  }
}

void loop() {
  // Calculate the speed of the bike

  long int rn = millis();
  if (rn - last_calculate_time > every_millis) {
    ticks_per_sec = hal_ticks - last_hal_ticks;
    Accel_PWM_Frame_End = map(clamp(ticks_per_sec, 0, 4), 0, 4, 0, PWM_Frame_Length);
    last_calculate_time = rn;
    hal_ticks = 0;
    Serial.println("calc");
  }

  //Accelerator PWM using speed of bike
  if (Accel_PWM_Frame_Pointer == 0 && Accel_PWM_Frame_End > 0) {
    Joystick.setButton(AccelButton, 1);
  } else if (Accel_PWM_Frame_Pointer == Accel_PWM_Frame_End) {
    Joystick.setButton(AccelButton, 0);
  }
  Accel_PWM_Frame_Pointer++;
  Accel_PWM_Frame_Pointer %= PWM_Frame_Length;



  //Joy Axes
  int yReading = analogRead(JoyYPin);
  int xReading = analogRead(JoyXPin);
  Joystick.setXAxis(xReading);
  Joystick.setYAxis(yReading);

  //Buttons Button

  for (int i = 0; i < NumButtons; i++) {
    int ButtonReading = digitalRead(ControllerButtonPins[i]);
    if (ButtonReading != lastButtonReadings[i]) {
      Joystick.setButton(GCButtons[i], ButtonReading);
      lastButtonReadings[i] = ButtonReading;
    }
  }

  // Serial.println(Accel_PWM_Frame_Pointer);

  delay(5);
}