#include <Ps3Controller.h>

#define L_RPWM 25  
#define L_LPWM 26  

#define R_RPWM 32  
#define R_LPWM 33  

#define PWM_FREQ 5000
#define PWM_RES  8   

#define CH_L_RPWM 0
#define CH_L_LPWM 1
#define CH_R_RPWM 2
#define CH_R_LPWM 3

float speedScale = 1.0;

int filtLeftY  = 0;
int filtRightY = 0;

const int maxStep = 6;   

void controlMotorBTS(int value, int pwmFwd, int pwmBwd);
void stopAllMotors();
void notify();

void setup() {
  Serial.begin(115200);

  ledcSetup(CH_L_RPWM, PWM_FREQ, PWM_RES);
  ledcAttachPin(L_RPWM, CH_L_RPWM);

  ledcSetup(CH_L_LPWM, PWM_FREQ, PWM_RES);
  ledcAttachPin(L_LPWM, CH_L_LPWM);

  ledcSetup(CH_R_RPWM, PWM_FREQ, PWM_RES);
  ledcAttachPin(R_RPWM, CH_R_RPWM);

  ledcSetup(CH_R_LPWM, PWM_FREQ, PWM_RES);
  ledcAttachPin(R_LPWM, CH_R_LPWM);

  stopAllMotors();


  Ps3.begin("24:04:06:01:06:06");  // MAC ESP32 
  Ps3.attach(notify);
  Ps3.attachOnConnect([]() {
    Serial.println("Controller Connected");
  });
}

void loop() {
  if (!Ps3.isConnected()) {
    return;
  }

  bool r1 = Ps3.data.button.r1;
  bool l1 = Ps3.data.button.l1;

  if (r1) {
    
    speedScale = 0.3;   
  } else if (l1) {
    
    speedScale = 0.6;  
  } else {
    
    speedScale = 1.0;   
  }

  int rawLeftY  = -Ps3.data.analog.stick.ly;  
  int rawRightY = -Ps3.data.analog.stick.ry;   

  int dL = rawLeftY - filtLeftY;
  if (dL > maxStep)       dL = maxStep;
  else if (dL < -maxStep) dL = -maxStep;
  filtLeftY += dL;

  int dR = rawRightY - filtRightY;
  if (dR > maxStep)       dR = maxStep;
  else if (dR < -maxStep) dR = -maxStep;
  filtRightY += dR;

  controlMotorBTS(filtLeftY,  CH_L_RPWM, CH_L_LPWM); 
  controlMotorBTS(filtRightY, CH_R_RPWM, CH_R_LPWM); 

  delay(5);  
}

void controlMotorBTS(int value, int pwmFwd, int pwmBwd) {
  int deadzone = 15;

  if (value > 127)  value = 127;
  if (value < -128) value = -128;

  int magnitude = abs(value);          
  if (magnitude > 128) magnitude = 128;

  int speed = map(magnitude, 0, 128, 0, 255);

  speed = (int)(speed * speedScale);
  if (speed > 255) speed = 255;
  if (speed < 0)   speed = 0;

  if (value > deadzone) {
    
    ledcWrite(pwmFwd, speed);
    ledcWrite(pwmBwd, 0);
  } else if (value < -deadzone) {
    
    ledcWrite(pwmFwd, 0);
    ledcWrite(pwmBwd, speed);
  } else {
    
    ledcWrite(pwmFwd, 0);
    ledcWrite(pwmBwd, 0);
  }
}

void stopAllMotors() {
  ledcWrite(CH_L_RPWM, 0);
  ledcWrite(CH_L_LPWM, 0);
  ledcWrite(CH_R_RPWM, 0);
  ledcWrite(CH_R_LPWM, 0);
}

void notify() {
 
}
