/*#include <OneWire.h>
#include <DallasTemperature.h>

//GPIO pin 0 is set as OneWire bus
OneWire ourWire1(0);
//GPIO pin 4 is set as OneWire bus
OneWire ourWire2(4);

//A variable or object is declared for our sensor 1
DallasTemperature sensors1(&ourWire1);
//A variable or object is declared for our sensor 2
DallasTemperature sensors2(&ourWire2);*/



#include <WiFi.h>
#include <HTTPClient.h>


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

void motor( void *pvParameters );
//void enviar( void *pvParameters );
void RPM( void *pvParameters );
void pwm( void *pvParameters );

volatile int counter = 0;

void interruption()    // Function that runs during each interrupt
{
  counter++;
}



// WiFi
const char *ssid = "Familia_ZuVe"; // Enter your WiFi name
const char *password = "fZV@0986278171";  // Enter WiFi password

//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.0.8:1880/update-sensor"; //repleace  192.168.0.101 for your addres




String temperatura1 = "";
String temperatura2 = "";
float temp1;
float temp2;




void setup() {
  //sensors
  /*sensors1.begin();   //Sensor 1 starts
  sensors2.begin();   //Sensor 2 starts

  // Set software serial baud to 115200;
  Serial.begin(115200);*/

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
    , 1);                // core 0

  // connecting to a WiFi network
  
  //wifi
  WiFi.mode(WIFI_STA); 
  


}



void loop() {
  connect_wifi();  
  publicData();
  vTaskDelay(1000);
}



void motor( void *pvParameters ) {
  while (1) {    
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW);
      ledcWrite(pwmChannel, dutyCycle);
      //vTaskDelay(period);
  }
}

// Calcula las RPM quye tiene el motor
void RPM( void *pvParameters ) {
  while (1) {
    vTaskDelay(999);
    //Serial.println(counter * 60);//255 -> 0x32 0x35 0x35 
    Serial.write(counter);//0-255
    
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




void connect_wifi(){
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, password);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
}

void publicData(){
  WiFiClient client;
      HTTPClient http;
    temperatura1 = String(dutyCycle);
    temperatura2 = String(dutyCycle);
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);

      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=DS18B20&value1="+temperatura1+"&value2="+temperatura2;           
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      
      
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
}
