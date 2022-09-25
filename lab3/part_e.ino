#include <Arduino_FreeRTOS.h>
#include <semphr.h>             // add the FreeRTOS functions for Semaphores (or Flags).

#define TASK1PIN 13
#define TASK2PIN 11
#define PARTEPIN 2

SemaphoreHandle_t xtask1Semaphore = NULL;
int               semBlockTime = 600;          
BaseType_t        xtask1SemGive;
BaseType_t        xtask3SemGive;
TaskHandle_t      xtask1Handle = NULL;
TaskHandle_t      xtask2Handle = NULL;

SemaphoreHandle_t xsemaphoreInterrupt = NULL;

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void InterruptISR();

// the setup function runs once when you press reset or power the board
void setup() {
  
  pinMode(TASK1PIN, OUTPUT);
  pinMode(TASK2PIN, OUTPUT);
  pinMode(PARTEPIN, INPUT);

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
    ,  &xtask1Handle );
  

  xTaskCreate(
    Task2
    ,  "Task2"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &xtask2Handle );

    attachInterrupt(digitalPinToInterrupt(PARTEPIN), InterruptISR, CHANGE);

    xsemaphoreInterrupt = xSemaphoreCreateBinary();
}

void loop()
{
  // Empty. Things are done in Tasks.
}

void BusyWait1ms(void)
{
  volatile int i = 0;
  for(i=0;i<837;i++);
}

void CPU_work(int time, int pin)
{
  volatile int i,j,k;
  int mode=0;
  for(i=0;i<time;i++)
  for(j=0;j<8;j++){
    mode=!mode;
    digitalWrite(pin, mode);
    for(k=0;k<100;k++);
  }
}

void Task1(void *pvParameters)
{
    int period = 33;    //need to be higher than 30 ms because pdMS_TO_TICKS divides period by 16 and outputs an integer (30/16= 1)
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(period);
    xLastWakeTime = xTaskGetTickCount();

    (void) pvParameters;
    
    for (;;) // A Task shall never return or exit.
    {
        CPU_work(20, TASK1PIN);
        
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void Task2(void *pvParameters)
{
    (void) pvParameters;
    
    //TickType_t xBlockTime = pdMS_TO_TICKS(semBlockTime);  //pdMS_TO_TICKS essentially divides paramter by 16 to an int
    
    for(;;) {           // A Task shall never return or exit.
        if(xsemaphoreInterrupt != NULL){
            if(xSemaphoreTake(xsemaphoreInterrupt, (TickType_t) 5) == pdTRUE){
              CPU_work(100, TASK2PIN);
              xSemaphoreGive(xsemaphoreInterrupt);
              vTaskSuspend(xtask2Handle);
              //xtask3SemGive = pdFALSE;
            }
        }
      }
}

void InterruptISR(){
  // CPU_work(100, TASK2PIN);
  vTaskResume(xtask2Handle);
  xSemaphoreGiveFromISR(xsemaphoreInterrupt, NULL);
}
