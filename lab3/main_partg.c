/**< C libraries includes*/
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
/**< FreeRTOS port includes*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
/**< Drivers includes*/
#include "gpio.h"

/*
 * Input commands and pin mounts
 */

#define FORWARD 'F'
#define LEFT    'L'
#define BACK    'B'
#define RIGHT   'R'
#define STOP    'S'

//Adjust to add intermediate speeds
#define IDLE    0
#define SPEED   1

//Adjust to reverse motor polarity
int LEFT_MOTOR  = 1;
int RIGHT_MOTOR = 0;

//Testing for RPI4B

//Motor Control 
#define EN1 20  //pwm pin, left motor
#define EN2 21  //pwm pin, right motor
#define A_1 6   //Y1, left motor positive
#define A_2 13  //Y2, left motor negative
#define A_3 19  //Y3, right motor positive
#define A_4 26  //Y4, right motor negative

//Pins for your distance sensor
#define TRIG 9
#define ECHO 11

//Trigger pins tied to each task
// #define T1_PIN 14	//this pin doesn't work!
// #define T2_PIN 15
// #define T3_PIN 18

#define T1_PIN 12
#define T2_PIN 23
#define T3_PIN 5

//LCD pins. Feel free to change.
#define RS 2
#define RW 3
#define E_ 4
#define D4 17
#define D5 27
#define D6 22
#define D7 10

#define DEBUGPIN 23

//moveRobot as used in lab 1
void moveRobot(char command);

//Helper function for moveRobot as used in lab 1
void motorControl(int ifLeftMotor, char command);


//LCD functions
void LCD_init();
void LCD_config( int dotMode,                               //(0)5x8 dot matrix (up to two lines) (1)5x10 dot matrix
				int lineMode,                              //(0)one line mode (1)two line mode
				int blinkOn,
				int cursorOn,
				int displayOn);
void LCD_print(char* message, int row, int col);
void LCD_write(int data);
void LCD_cursorHome();
void LCD_setCursor(int row, int col);
void LCD_clear(int row);
void LCD_pinInitOutput(int pin);
void LCD_pinInitInput(int pin);
void LCD_writePin(int pin, int value);
void LCD_initPins(int RS_in, int RW_in, int E_in, int DB_in[8]);
void LCD_writeArray(int pins[8], int data);
void LCD_pulse(int pin, int delayNum);
void LCD_setCGRAMAddr(int CGRAM_addr);
void LCD_setDDRAMAddr(int DDRAM_addr);
void LCD_writeCommand(int command);
void LCD_writeStartCommand(int command);
void LCD_writeCGRAM(int data, int addr);
void LCD_writeDDRAM(int data, int addr);
void LCD_writeMessage(char message);

void cputime(unsigned long numofms) { volatile unsigned long i = 0; for(i=0;i<numofms*1851;i++); }

void cputimeMicro(unsigned long numofms) { volatile unsigned long i = 0; for(i=0;i<numofms*2;i++); }

// void delay1ms() {
// 	volatile TickType_t last_wake_time = xTaskGetTickCount();
// 	for (;;) {
// 		vTaskDelayUntil(&last_wake_time, 1/portTICK_PERIOD_MS);
// 	}
// }

// void delay(int delay) {

// }

void delay1ms() {
	cputime(1);
}

void delay1us(){
	cputimeMicro(1);
}

void delay(int delay){
	volatile int i;
	for(i = 0; i < delay; i++) {
		delay1ms();
	}
}

// void delay(int delay) {
// 	clock_t start = clock();
// 	while(clock() < start + delay){}
// }


void delayMicroseconds(int delay){
	// volatile unsigned int i;
	// for(i = 0; i < delay / portTICK_RATE_MS / 1000; i++);
	//delay1ms();
	volatile int i;
	for(i = 0; i < delay; i++) {
		delay1us();
	}
}

//

/**
	LCD function declarations. See GPIO pin settings above.
**/

//(0)4_bit MPU interface (1)8_bit MPU interface
int bitInterfaceMode = 0;

//writeCommand pins
int _RS = RS;
int _RW = RW;
int _E = E_;
int _DB[8] = {0, 0, 0, 0, D4, D5, D6, D7};

//Some Commands
int CLEARDISPLAY = 0b00000001;    
int RETURNHOME   = 0b00000010;
int DISPLAYON    = 0b00001111;

// initialize LCD and pins on arduino
void LCD_init() {
	LCD_initPins(_RS, _RW, _E, _DB);

	delay(20);
	LCD_writeStartCommand(0b00110000);
	delay(5);
	LCD_writeStartCommand(0b00110000);
	delay(1);
	LCD_writeStartCommand(0b00110000);
	delay(5);
	LCD_writeStartCommand(0b00100000);
	delay(1);
	LCD_writeCommand(0b00001000); // display off
	delay(1);
	LCD_writeCommand(0b00000001); // clear display
	delay(2);
	LCD_writeCommand(0b00000110);                               // entry mode, set ddram increasing
	delay(1);

	// initialization completed
	LCD_writeCommand(0b00001111);                               // turn on display with cursor
	delay(5);
	LCD_writeCommand(0b00101000);                               // 4-bit, 2 lines, 5x8
	delay(1);
	
	LCD_cursorHome();                                           //set cursor to top left corner
}

// Enable dot mode, line mode, display, cursor, blink according to user
void LCD_config( int dotMode,                               //(0)5x8 dot matrix (up to two lines) (1)5x10 dot matrix
				int lineMode,                              //(0)one line mode (1)two line mode
				int blinkOn,
				int cursorOn,
				int displayOn) {

	//Enable dot mode, line mode
	int functionSetCmd = 0b00100000 + 
		(lineMode ? 0b1000 : 0) +
		(dotMode ? 0b100 : 0);
	LCD_writeCommand(functionSetCmd);
	delayMicroseconds(50);

	//Enable display, cursor, blink according to user
	int displayCmd = 0b00001000 + 
		(displayOn ? 0b100 : 0) + 
		(cursorOn ? 0b10 : 0) +
		(blinkOn ? 0b1 : 0);
	LCD_writeCommand(displayCmd);
}     

int getSize(char *s) {
	char* t;
	for (t = s; *t != '\0'; t++) {
	}
	return t - s;
}

/* 
	* Replace the current content with text and its length from the specified row and col
	* 
	* When write empty strings with certain length, it will work as delete function start
	* from the position with the length of the empty string
	* 
	* When row are set to -1, this function should append text to the current location of the cursor
	*
	*/
void LCD_print(char* message, int row, int col) {
	if ((row >= 0 && row < 2) || (col >= 0 && col < 16)) {
		LCD_setCursor(row, col);
	}
	for (int i = 0; i < getSize(message); i++) {
		LCD_write(message[i]);
	}
}

// write arbitary data, in int
void LCD_write(int data) {
	if (bitInterfaceMode) {
		// 8-bit mode
		LCD_writePin(_RS, 1);
		LCD_writePin(_RW, 0);
		LCD_writePin(_E, 0);
		LCD_writeArray(_DB, data);
		LCD_pulse(_E, 1);
		delay(1);
	}
	else {
		// 4-bit mode
		LCD_writePin(_RS, 1);
		LCD_writePin(_RW, 0);
		LCD_writePin(_E, 0);
		LCD_writeArray(_DB, data);
		LCD_pulse(_E, 1);
		delay(1);
		LCD_writeArray(_DB, data << 4);
		LCD_pulse(_E, 1);
		delay(1);
	}
}

// set cursor to specied spot on display
void LCD_setCursor(int row, int col){                    
	LCD_cursorHome();  
	LCD_writePin(_RS, 0);                             
	LCD_writePin(_RW, 0);
	if(row == 0){
		for(int i=0; i < col; i++){
			LCD_writeCommand(0b00010100);
			delayMicroseconds(50);
		}
	}
	if(row == 1){
		for(int i=0; i < col + 40; i++){
			LCD_writeCommand(0b00010100);
			delayMicroseconds(50);
		}
	}
}

// set cursor to 1st row and 1st column (upper-left corner)
void LCD_cursorHome(){                                      
	LCD_writeCommand(RETURNHOME);
	delay(2);
}

// clear what is currently displayed and places cursor at upper left corner
void LCD_clear(int row){     
	if (row == 0) {
		LCD_print("                ", 0, -1);
	}
	else if (row == 1) {
		LCD_print("                ", 1, -1);
	}
	else {
		LCD_writeCommand(CLEARDISPLAY);
	}
	delay(4);
}

//"Backdoor"
void LCD_pinInitOutput(int pin) {
	gpio_pin_init(pin, OUT, GPIO_PIN_PULL_UP);
}

void LCD_pinInitInput(int pin) {
	gpio_pin_init(pin, IN, GPIO_PIN_PULL_UP);
}

void LCD_writePin(int pin, int value) {
	gpio_pin_set(pin, value);
}

// initialize pins on arduino
void LCD_initPins(int RS_in, int RW_in, int E_in, int DB_in[8]){
	
	LCD_pinInitOutput(RS_in);
	LCD_pinInitOutput(RW_in);
	LCD_pinInitOutput(E_in);
	
	LCD_pinInitOutput(DB_in[7]);
	LCD_pinInitOutput(DB_in[6]);
	LCD_pinInitOutput(DB_in[5]);
	LCD_pinInitOutput(DB_in[4]);
	LCD_pinInitOutput(DB_in[3]);
	LCD_pinInitOutput(DB_in[2]);
	LCD_pinInitOutput(DB_in[1]);
	LCD_pinInitOutput(DB_in[0]);
}

//convert data into individual bits for the pin outputs
void LCD_writeArray(int pins[8], int data) {
	for (int i = 0; i < 8; i ++) {
		LCD_writePin(pins[i], (data >> i) & 0b1);
	}
}

//enable (E) pin pulse
void LCD_pulse(int pin, int delayNum){
	LCD_writePin(pin, 0);
	delayMicroseconds(delayNum);        //enable pulse width > 450ns
	LCD_writePin(pin, 1);
	delayMicroseconds(delayNum);
	LCD_writePin(pin, 0);
}

// Set the address of CGRAM
void LCD_setCGRAMAddr(int CGRAM_addr){
	int command = 0b0010000000;
	command = command | (CGRAM_addr & 0b111111);
	LCD_writeCommand(command);
	delayMicroseconds(50);
}

// Set the address of DDRAM
void LCD_setDDRAMAddr(int DDRAM_addr){
	int command = 0b0010000000;
	command = command | (DDRAM_addr & 0b1111111);
	LCD_writeCommand(command);
	delayMicroseconds(50);
}

//Send command routine to the pins based on bit mode
void LCD_writeCommand(int command){
	if (bitInterfaceMode) {
		// 8-bit mode
		LCD_writePin(_RS, 0);
		LCD_writePin(_RW, 0);
		LCD_writePin(_E, 0);
		LCD_writeArray(_DB, command);
		LCD_pulse(_E, 1);
	}
	else {
		// 4-bit mode- writes the data two times
		LCD_writePin(_RS, 0);
		LCD_writePin(_RW, 0);
		LCD_writePin(_E, 0);
		LCD_writeArray(_DB, command);
		LCD_pulse(_E, 1);
		LCD_writeArray(_DB, command << 4);
		LCD_pulse(_E, 1);
	}
}

void LCD_writeStartCommand(int command){
	// 8-bit mode
	LCD_writePin(_RS, 0);
	LCD_writePin(_RW, 0);
	LCD_writePin(_E, 0);
	LCD_writeArray(_DB, command);
	LCD_pulse(_E, 1);
}

// writes a binary 8-bit data to CGRAM at address location
void LCD_writeCGRAM(int data, int addr) {
	LCD_setCGRAMAddr(addr);
	delay(1);
	LCD_write(data);
	delay(1);
}

// writes a binary 8-bit data to DDRAM at address location
void LCD_writeDDRAM(int data, int addr) {
	LCD_setDDRAMAddr(addr);
	delay(1);
	LCD_write(data);
	delay(1);
}

//convert string to byte data (for display)
void LCD_writeMessage(char message) {
	// for (int i = 0; i < message.length(); i ++) {
	// 	if(autowrap && i == 16){
	// 		LCD_setCursor(1, 0);
	// 	}
	// 	Serial.println((int)message.charAt(i));
	// 	LCD_write((int)message.charAt(i));
	// 	delay(1);
	// }
	LCD_write((int)message);
	delay(1);
}

int DISTANCE_IN_TICKS = 0;
char command[10];
int direction = 3;
SemaphoreHandle_t distSem = NULL;

//Task 1 is implemented for you. It interfaces with the distance sensor and 
//calculates the number of ticks the echo line is high after trigger, storing that value in
//the global variable DISTANCE_IN_TICKS

//You may need to alter some values as described in the lab documentation.

void task1() {
	portTickType xLastWakeTime;
	const portTickType xFrequency = 200 / portTICK_RATE_MS;
	
	xLastWakeTime = xTaskGetTickCount();

	if(distSem == NULL){
		distSem = xSemaphoreCreateBinary();
	}

	while(1) {
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		
		//IN TASK
		gpio_pin_set(T1_PIN, 1);
		//

		gpio_pin_set(TRIG, 1);

		vTaskDelay(1);

		gpio_pin_set(TRIG, 0);
		while(gpio_pin_read(ECHO) == 0);
		
		portTickType curr = xTaskGetTickCount();
		while(gpio_pin_read(ECHO) == 1);
		portTickType traveltime_in_ticks = xTaskGetTickCount() - curr;

		DISTANCE_IN_TICKS = traveltime_in_ticks;
		xSemaphoreGive(distSem);
		// LCD_clear(-1);
		// int distance_ticks = DISTANCE_IN_TICKS;
		// while (distance_ticks > 0) {
		// 	LCD_write(distance_ticks % 10 + '0');
		// 	distance_ticks = distance_ticks / 10;
		// }
		
		//END TASK
		gpio_pin_set(T1_PIN, 0);
	}
}

void task2(){
    portTickType xLastWakeTime;
    const portTickType xFrequency = 100 / portTICK_RATE_MS;
	xLastWakeTime = xTaskGetTickCount();
                                                           
    portTickType tickdistanceThreshold = 3; //pdMS_TO_TICKS((distanceThreshold * 58) / 1000);  

	// int localDistance = 0;

     for( ;; ){
		// if(xSemaphoreTake(distSem, 50)== pdTRUE){
		// 	localDistance = DISTANCE_IN_TICKS;
		// 	xSemaphoreGive(distSem);
		// }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);

		//IN TASK
		gpio_pin_set(T2_PIN, 1);
		//
        if((TickType_t)DISTANCE_IN_TICKS < tickdistanceThreshold){
            moveRobot(STOP);
            // strcpy(command, "STOP");
			direction = 1;

        }
        else{
            moveRobot(FORWARD);
            // strcpy(command, "FOWARD");
			direction = 2;
        }
		
		//END TASK
		gpio_pin_set(T2_PIN, 0);
     }
}

// char* intToCharArray(int num, int array_size){
// 	char* charArray;
// 	sprintf(charArray, "%ld", num);
// 	// char* charArray = calloc(array_size + 2, sizeof(char));
// 	// int i = 0;
// 	// while(num != 0 && i < array_size){
// 	// 	charArray[i] = num % 10 + '0';
// 	// 	num = num / 10;
// 	// 	i++;
// 	// }
// 	return charArray;
// }

int modeDisplay = 0;    //0- display direction of motion; 1-display the current distance in cm

void task3(){
    portTickType xLastWakeTime;
    const portTickType xFrequency = 100 / portTICK_RATE_MS;
	xLastWakeTime = xTaskGetTickCount();
	// int localDistance = 0;

    for( ;; ){
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		// if(xSemaphoreTake(distSem, 50)== pdTRUE){
		// 	localDistance = DISTANCE_IN_TICKS;
		// 	xSemaphoreGive(distSem);
		// }

		//IN TASK
		gpio_pin_set(T3_PIN, 1);

        if(modeDisplay == 0){
            LCD_clear(-1);
			if(direction == 1){
				// LCD_write('S');
				// LCD_write('T');
				// LCD_write('O');
				// LCD_write('P');
				LCD_print("STOP", 0, 0);
			}
			else if(direction == 2){
				// LCD_write('F');
				// LCD_write('O');
				// LCD_write('R');
				// LCD_write('W');
				// LCD_write('A');
				// LCD_write('R');
				// LCD_write('D');
				LCD_print("FORWARD", 0, 0);
			}
			// modeDisplay = 1;
        }
        else if(modeDisplay == 1){
            LCD_clear(-1);
			int distance_ticks = DISTANCE_IN_TICKS;
			while (distance_ticks > 0) {
				LCD_write(distance_ticks % 10 + '0');
				distance_ticks = distance_ticks / 10;
			}
            //LCD_print(intToCharArray(distance, 10), -1, -1);
			LCD_print("0 cm", -1, -1);
			// LCD_write(' ');
			// LCD_write('c');
			// LCD_write('m');
			//modeDisplay = 0;
        }
        // modeDisplay = !modeDisplay;
		//END TASK
		gpio_pin_set(T3_PIN, 0);
    }
}

void task4() {
	portTickType xLastWakeTime;
    const portTickType xFrequency = 2000 / portTICK_RATE_MS;
	xLastWakeTime = xTaskGetTickCount();

	while (1) {
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		modeDisplay = !modeDisplay;
	}
}

void testTask() {
	for (;;) {
		gpio_pin_set(T1_PIN, 1);
		// gpio_pin_set(DEBUGPIN, 1);
		delay(100);
		// delay1ms();
		// vTaskDelay(100);
		//cputime(10);
		gpio_pin_set(T1_PIN, 0);
		// gpio_pin_set(DEBUGPIN, 0);
		delay(100);
		//cputime(10);
		// vTaskDelay(100);
	}
}

int main(void) {
	gpio_pin_init(T1_PIN, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_init(T2_PIN, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_init(T3_PIN, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_set(T1_PIN, 0);
	gpio_pin_set(T2_PIN, 0);
	gpio_pin_set(T3_PIN, 0);

	gpio_pin_init(DEBUGPIN, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_set(DEBUGPIN, 0);

	gpio_pin_init(EN1, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_init(EN2, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_init(A_1, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_init(A_2, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_init(A_3, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_init(A_4, OUT, GPIO_PIN_PULL_UP);

	gpio_pin_init(TRIG, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_init(ECHO, IN, GPIO_PIN_PULL_NON);
	gpio_pin_set(TRIG, 0);

	gpio_pin_init(GPIO_14, OUT, GPIO_PIN_PULL_UP);
	gpio_pin_set(GPIO_14, 0);

	LCD_init();
	char* message = "I'm a robot.";
	LCD_print(message, 0, 0);
	message = "A real one!";
	LCD_print(message, 1, 0);
	// LCD_write('S');
	// LCD_write('T');
	// LCD_write('O');
	// LCD_write('P');
	delay(5000);
	LCD_clear(-1);
	LCD_write('I');
	

	//initFB();

	//DisableInterrupts();
	//InitInterruptController();

	xTaskCreate(task1, "t1", 128, NULL, 2, NULL);

    xTaskCreate(task2, "t2", 128, NULL, 1, NULL);
    
    xTaskCreate(task3, "t3", 128, NULL, 0, NULL);
	
	xTaskCreate(task4, "t4", 128, NULL, 0, NULL);
    // xTaskCreate(testTask, "tt", 128, NULL, 2, NULL);

	//set to 0 for no debug, 1 for debug, or 2 for GCC instrumentation (if enabled in config)
	//loaded = 1;

	vTaskStartScheduler();

	/*
	 *	We should never get here, but just in case something goes wrong,
	 *	we'll place the CPU into a safe loop.
	 */
	while(1) {
		;
	}
}

/****
		TODO: Add LCD functions here.






	****/

void moveRobot(char command) {
    switch(command) {
        case FORWARD:
            motorControl(LEFT_MOTOR, FORWARD);
            motorControl(RIGHT_MOTOR, FORWARD);
            break;
        case LEFT:
            motorControl(LEFT_MOTOR, STOP);
            motorControl(RIGHT_MOTOR, FORWARD);
            break; 
        case BACK:
            motorControl(LEFT_MOTOR, BACK);
            motorControl(RIGHT_MOTOR, BACK);
            break; 
        case RIGHT:
            motorControl(LEFT_MOTOR, FORWARD);
            motorControl(RIGHT_MOTOR, STOP);
            break;
        case STOP:
            motorControl(LEFT_MOTOR, STOP);
            motorControl(RIGHT_MOTOR, STOP);
            break;
        default:
            break;
    }   
}

void motorControl(int ifLeftMotor, char command) {
    int enable      = ifLeftMotor ? EN1 : EN2;
    int motorPos    = ifLeftMotor ? A_1 : A_3;
    int motorNeg    = ifLeftMotor ? A_2 : A_4;

    switch (command) {
        case FORWARD:
            gpio_pin_set(enable, SPEED);
            gpio_pin_set(motorPos, 1);
            gpio_pin_set(motorNeg, 0);
            break;
        case BACK:
            gpio_pin_set(enable, SPEED);
            gpio_pin_set(motorPos, 0);
            gpio_pin_set(motorNeg, 1);
            break;    
        case STOP:
            gpio_pin_set(motorPos, 0);
            gpio_pin_set(motorNeg, 0);
            break;
        default:
            break;           
    }
}

void vApplicationIdleHook( void ){}
void vApplicationTickHook( void ){}