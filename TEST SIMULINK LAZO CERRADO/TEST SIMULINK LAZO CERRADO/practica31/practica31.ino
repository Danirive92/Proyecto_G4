#include <PIDController.hpp>

const int PIN_INPUT = 0;
const int PIN_OUTPUT = 3;

PID::PIDParameters<double> parameters(0.000109, 0.000416, 0);
PID::PIDController<double> pidController(parameters);

//initial setting for data acquisition
int dutyCycleInitial = 255;
int dutyCycleFinish = 0;
int period = 13000;
int cycles = 10;
int dutyCycle = 0;

//separador library
#include <Separador.h>
Separador s;

//motor
int motor1Pin1 = 33;
int motor1Pin2 = 25;
int enable1Pin = 32;
int motor_status = 1;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;

//move
String move_motor = "counterclockwise";

int encoder = 27;

int proy = pidController.Output*0.00392156862;

void motor( void *pvParameters );
//void enviar( void *pvParameters );
void RPM( void *pvParameters );
void pwm( void *pvParameters );

volatile int counter = 0;

void interruption()    // Function that runs during each interrupt
{
  counter++;
}


void setup()
{
  Serial.begin(115200);
  // sets the pins as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
  attachInterrupt(encoder, interruption, RISING);

  xTaskCreatePinnedToCore(
    motor
    ,  "MotorDC"         // Descriptive name of the function (MAX 8 characters)
    ,  2048              // Size required in STACK memory
    ,  NULL              // INITIAL parameter to receive (void *)
    ,  1                 // Priority, priority = 3 (configMAX_PRIORITIES - 1) is the highest, priority = 0 is the lowest.
    ,  NULL              // Variable that points to the task (optional)
    , 1);                // core 1

  xTaskCreatePinnedToCore(
    RPM
    ,  "RPM"              // Descriptive name of the function (MAX 8 characters)
    ,  2048              // Size required in STACK memory
    ,  NULL              // INITIAL parameter to receive (void *)
    ,  1                 // Priority, priority = 3 (configMAX_PRIORITIES - 1) is the highest, priority = 0 is the lowest.
    ,  NULL              // Variable that points to the task (optional)
    , 1);                // core 0

  xTaskCreatePinnedToCore(
    pwm
    ,  "PWM"              // Descriptive name of the function (MAX 8 characters)
    ,  2048              // Size required in STACK memory
    ,  NULL              // INITIAL parameter to receive (void *)
    ,  1                 // Priority, priority = 3 (configMAX_PRIORITIES - 1) is the highest, priority = 0 is the lowest.
    ,  NULL              // Variable that points to the task (optional)
    , 1);    
  pidController.Input = 0;
  pidController.Setpoint = dutyCycle;

  pidController.TurnOn();
}

void loop()
{

}


void motor( void *pvParameters ) {
  while (1) {    
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW);
      //ledcWrite(pwmChannel, pidController.Output);
      ledcWrite(pwmChannel, dutyCycle);
      //vTaskDelay(period);
  }
}
// Calcula las RPM quye tiene el motor
void RPM( void *pvParameters ) {
  while (1) {
    vTaskDelay(999);
    //Serial.println(counter * 60);//255 -> 0x32 0x35 0x35 
    pidController.Input = counter*3.035;
    pidController.Update();
    Serial.println(proy);//255 -> 0x32 0x35 0x35 
    //Serial.write(counter);//0-255
    //Serial.write(proy);//0-255
    counter = 0;
  }
}

// Read del PWM que viene desde Matlab
void pwm( void *pvParameters ) {
  while (1) {
    //Serial.println("hola");
    if (Serial.available())
    {
      String string = Serial.readStringUntil('\n');
      dutyCycle = string.toInt();
    }
  }
}
