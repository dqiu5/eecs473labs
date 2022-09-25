#include <Arduino_FreeRTOS.h>
#include <semphr.h>             // add the FreeRTOS functions for Semaphores (or Flags).

#define TASK1PIN 12
#define TASK2PIN 13

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
// void TaskAnalogRead( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  
  pinMode(TASK1PIN, OUTPUT);
  pinMode(TASK2PIN, OUTPUT);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  xTaskCreate(
    Task1
    ,  "Task1"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    Task2
    ,  "Task2"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

    xTaskCreate(
    Task2
    ,  "Task2"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

    SemaphoreHandle_t task1Sem = xSemaphoreCreateBinary();

  // Now set up two tasks to run independently.
  //xTaskCreate(
  //  TaskBlink
  //  ,  "Blink"   // A name just for humans
  //  ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
  //  ,  NULL
  //  ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
  //  ,  NULL );

//   xTaskCreate(
//     TaskAnalogRead
//     ,  "AnalogRead"
//     ,  128  // Stack size
//     ,  NULL
//     ,  1  // Priority
//     ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters) {
     (void) pvParameters;
     pinMode(LED_BUILTIN, OUTPUT);
     for (;;) // A Task shall never return or exit.
     {
       digitalWrite(LED_BUILTIN, HIGH);
       CPU_work(100);
       digitalWrite(LED_BUILTIN, LOW);
       CPU_work(900);
} }

void BusyWait1ms(void)
{
  volatile int i = 0;
  for(i=0;i<837;i++);
}

void CPU_work(int time) {
  volatile int i = 0;
  
  for(i=0;i<time;i++) {
    BusyWait1ms();
  }
}



void Task1(void *pvParameters)
{
    (void) pvParameters;
    
    for(;;) {           // A Task shall never return or exit.
        if ()
        CPU_work(20);
        digitalWrite(TASK1PIN, LOW);
    }
    
}

void Task2(void *pvParameters)
{
    int period = 33;    //need to be higher than 30 ms because pdMS_TO_TICKS divides period by 16 and outputs an integer (30/16= 1)
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(period);
    xLastWakeTime = xTaskGetTickCount();

    (void) pvParameters;
    
    for (;;) // A Task shall never return or exit.
    {
        digitalWrite(TASK2PIN, HIGH);
        CPU_work(10);
        digitalWrite(TASK2PIN, LOW);
        
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void Task3(void *pvParameters)
{
    int period = 85;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(period);
    xLastWakeTime = xTaskGetTickCount();

    (void) pvParameters;
    pinMode(TASK1PIN, OUTPUT);
    
    for (;;) // A Task shall never return or exit.
    {
        digitalWrite(TASK1PIN, HIGH);
        
        
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}


/*
void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
*/

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, LEONARDO, MEGA, and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN takes care 
  of use the correct LED pin whatever is the board used.
  
  The MICRO does not have a LED_BUILTIN available. For the MICRO board please substitute
  the LED_BUILTIN definition with either LED_BUILTIN_RX or LED_BUILTIN_TX.
  e.g. pinMode(LED_BUILTIN_RX, OUTPUT); etc.
  
  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products
  
  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
  
  modified 2 Sep 2016
  by Arturo Guadalupi
*/

  // initialize digital LED_BUILTIN on pin 13 as an output.

  /*
  pinMode(LED_BUILTIN, OUTPUT);

  #define GPIOPIN 12
  pinMode(GPIOPIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(GPIOPIN, HIGH);
    vTaskDelay( 898 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(GPIOPIN, LOW);
    vTaskDelay( 898 / portTICK_PERIOD_MS ); // wait for one second
  }
}
*/

// void TaskAnalogRead(void *pvParameters)  // This is a task.
// {
//   (void) pvParameters;
  
// /*
//   AnalogReadSerial
//   Reads an analog input on pin 0, prints the result to the serial monitor.
//   Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
//   Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

//   This example code is in the public domain.
// */

//   for (;;)
//   {
//     // read the input on analog pin 0:
//     int sensorValue = analogRead(A0);
//     // print out the value you read:
//     Serial.println(sensorValue);
//     vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
//   }
// }