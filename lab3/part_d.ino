#include <Arduino_FreeRTOS.h>
#include <semphr.h>             // add the FreeRTOS functions for Semaphores (or Flags).

#define TASK1PIN 13
#define TASK2PIN 11

SemaphoreHandle_t xtask1Semaphore = NULL;
int               semBlockTime = 600;          
BaseType_t        xtask1SemGive;
BaseType_t        xtask3SemGive;
TaskHandle_t      xtask1Handle = NULL;
TaskHandle_t      xtask3Handle = NULL;

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );

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
    ,  1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &xtask1Handle );
  

  xTaskCreate(
    Task2
    ,  "Task2"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    Task3
    ,  "Task3"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    , 3 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &xtask3Handle );

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
    
    //TickType_t xBlockTime = pdMS_TO_TICKS(semBlockTime);  //pdMS_TO_TICKS essentially divides paramter by 16 to an int
    
    for(;;) {           // A Task shall never return or exit.
        if(xtask1Semaphore != NULL){
          if(xtask3SemGive == pdTRUE) {
            if(xSemaphoreTake(xtask1Semaphore, semBlockTime)== pdTRUE){
              CPU_work(20);
              digitalWrite(TASK1PIN, LOW);
              xtask1SemGive = xSemaphoreGive(xtask1Semaphore);
              vTaskSuspend(xtask1Handle);
              //xtask3SemGive = pdFALSE;
            }
          }
        }
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

    if ( xtask1Semaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
    {
      xtask1Semaphore = xSemaphoreCreateBinary();
      xSemaphoreGive(xtask1Semaphore);  
    }

    xtask1SemGive = pdTRUE;
    xtask3SemGive = pdFALSE;

    (void) pvParameters;
    
    for (;;) // A Task shall never return or exit.
    {
        xLastWakeTime = xTaskGetTickCount();
        
        if (xtask1Semaphore != NULL) {
            //Serial.println("Enter first if");
            if(xtask1SemGive == pdTRUE){
              if(xSemaphoreTake(xtask1Semaphore, semBlockTime) == pdTRUE){
                vTaskResume(xtask1Handle);
                //Serial.println("Enter second if");
                digitalWrite(TASK1PIN, HIGH);
                xtask3SemGive = xSemaphoreGive(xtask1Semaphore);
                //xtask1SemGive = pdFALSE;
              }
            }
        }

        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(86) );
    }
}
