class LCDInterface {
	public:
        //(0)4_bit MPU interface (1)8_bit MPU interface
        bool bitInterfaceMode;

        //writeCommand pins
        int RS;
        int RW;
        int E;
        int DB[8] = {};

        //Some Commands
        int CLEARDISPLAY = 0b00000001;    
        int RETURNHOME   = 0b00000010;
        int DISPLAYON    = 0b00001111;

        // Constructor for 8 bit mode
        LCDInterface(int RS, int RW, int E, int DB0, int DB1, int DB2, int DB3, int DB4, int DB5, int DB6, int DB7) {
            this->RS = RS;
            this->RW = RW;
            this->E = E;
            this->DB[0] = DB0;
            this->DB[1] = DB1;
            this->DB[2] = DB2;
            this->DB[3] = DB3;
            this->DB[4] = DB4;
            this->DB[5] = DB5;
            this->DB[6] = DB6;
            this->DB[7] = DB7;
            this->bitInterfaceMode = true;
        }

        // Constructor for 4 bit mode
        LCDInterface(int RS, int RW, int E, int DB4, int DB5, int DB6, int DB7) {
            this->RS = RS;
            this->RW = RW;
            this->E = E;
            this->DB[0] = 0;
            this->DB[1] = 0;
            this->DB[2] = 0;
            this->DB[3] = 0;
            this->DB[4] = DB4;
            this->DB[5] = DB5;
            this->DB[6] = DB6;
            this->DB[7] = DB7;
            this->bitInterfaceMode = false;
        }

        // initialize LCD and pins on arduino
        void init() {
            initPins(RS, RW, E, DB);

            delay(20);
            writeCommand(0b00110000, true);
            delay(5);
            writeCommand(0b00110000, true);
            delay(1);
            writeCommand(0b00110000, true);
            delay(5);
            writeCommand(0b00100000, true);
            delay(1);
            writeCommand(0b00001000); // display off
            delay(1);
            writeCommand(0b00000001); // clear display
            delay(2);
            writeCommand(0b00000110);                               // entry mode, set ddram increasing
            delay(1);
        
            // initialization completed
            writeCommand(0b00001111);                               // turn on display with cursor
            delay(5);
            writeCommand(0b00101000);                               // 4-bit, 2 lines, 5x8
            delay(1);
            
            cursorHome();                                           //set cursor to top left corner
        }
        
        // Enable dot mode, line mode, display, cursor, blink according to user
        void config( bool dotMode,                               //(0)5x8 dot matrix (up to two lines) (1)5x10 dot matrix
                     bool lineMode,                              //(0)one line mode (1)two line mode
                     bool blinkOn,
                     bool cursorOn,
                     bool displayOn) {

            //Enable dot mode, line mode
            int functionSetCmd = 0b00100000 + 
                (lineMode ? 0b1000 : 0) +
                (dotMode ? 0b100 : 0);
            writeCommand(functionSetCmd);
            delayMicroseconds(50);

            //Enable display, cursor, blink according to user
            int displayCmd = 0b00001000 + 
                (displayOn ? 0b100 : 0) + 
                (cursorOn ? 0b10 : 0) +
                (blinkOn ? 0b1 : 0);
            writeCommand(displayCmd);
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
        void print(String message, int row = -1, int col = -1) {
            if ((row >= 0 && row < 2) || (col >= 0 && col < 16)) {
                setCursor(row, col);
            }
            writeMessage(message);
        }
        
        // write arbitary data, in int
        void write(int data) {
            if (bitInterfaceMode) {
                // 8-bit mode
                digitalWrite(RS, 1);
                digitalWrite(RW, 0);
                digitalWrite(E, 0);
                writeArray(DB, data);
                pulse(E, 1);
            }
            else {
                // 4-bit mode
                digitalWrite(RS, 1);
                digitalWrite(RW, 0);
                digitalWrite(E, 0);
                writeArray(DB, data);
                pulse(E, 1);
                writeArray(DB, data << 4);
                pulse(E, 1);
            }
        }
        
        // set cursor to specied spot on display
        void setCursor(int row, int col){                    
            cursorHome();  
            digitalWrite(RS, 0);                             
            digitalWrite(RW, 0);
            if(row == 0){
                for(int i=0; i < col; i++){
                    writeCommand(0b00010100);
                    delayMicroseconds(50);
                }
            }
            if(row == 1){
                for(int i=0; i < col + 40; i++){
                    writeCommand(0b00010100);
                    delayMicroseconds(50);
                }
            }
        }
        
        // set cursor to 1st row and 1st column (upper-left corner)
        void cursorHome(){                                      
            writeCommand(RETURNHOME);
            delay(2);
        }

        // clear what is currently displayed and places cursor at upper left corner
        void clear(int row = -1){     
            if (row == 0) {
                print("                ", 0, -1);
            }
            else if (row == 1) {
                print("                ", 1, -1);
            }
            else {
                writeCommand(CLEARDISPLAY);
            }
            delay(4);
        }

    //"Backdoor"
    private:
        // initialize pins on arduino
        void initPins(int RS, int RW, int E, int DB[8]){
            pinMode(RS, OUTPUT);
            pinMode(RW, OUTPUT);
            pinMode(E, OUTPUT);
            
            pinMode(DB[7], OUTPUT);
            pinMode(DB[6], OUTPUT);
            pinMode(DB[5], OUTPUT);
            pinMode(DB[4], OUTPUT);
            pinMode(DB[3], OUTPUT);
            pinMode(DB[2], OUTPUT);
            pinMode(DB[1], OUTPUT);
            pinMode(DB[0], OUTPUT);
        }

        //convert data into individual bits for the pin outputs
        void writeArray(int pins[8], int data) {
            for (int i = 0; i < 8; i ++) {
                digitalWrite(pins[i], (data >> i) & 0b1);
            }
        }
        
        //enable (E) pin pulse
        void pulse(int pin, int delayNum){
            digitalWrite(pin, 0);
            delayMicroseconds(delayNum);        //enable pulse width > 450ns
            digitalWrite(pin, 1);
            delayMicroseconds(delayNum);
            digitalWrite(pin, 0);
        }
        
        // Set the address of CGRAM
        void setCGRAMAddr(int CGRAM_addr){
            int command = 0b0010000000;
            command = command | (CGRAM_addr & 0b111111);
            writeCommand(command);
            delayMicroseconds(50);
        }

        // Set the address of DDRAM
        void setDDRAMAddr(int DDRAM_addr){
            int command = 0b0010000000;
            command = command | (DDRAM_addr & 0b1111111);
            writeCommand(command);
            delayMicroseconds(50);
        }

        //Send command routine to the pins based on bit mode
        void writeCommand(int command, bool isStart = false){
            if (bitInterfaceMode || isStart) {
                // 8-bit mode
                digitalWrite(RS, 0);
                digitalWrite(RW, 0);
                digitalWrite(E, 0);
                writeArray(DB, command);
                pulse(E, 1);
            }
            else {
                // 4-bit mode- writes the data two times
                digitalWrite(RS, 0);
                digitalWrite(RW, 0);
                digitalWrite(E, 0);
                writeArray(DB, command);
                pulse(E, 1);
                writeArray(DB, command << 4);
                pulse(E, 1);
            }
        }

        // writes a binary 8-bit data to CGRAM at address location
        void writeCGRAM(int data, int addr) {
            setCGRAMAddr(addr);
            delay(1);
            write(data);
            delay(1);
        }
        
        // writes a binary 8-bit data to DDRAM at address location
        void writeDDRAM(int data, int addr) {
            setDDRAMAddr(addr);
            delay(1);
            write(data);
            delay(1);
        }
    
        //convert string to byte data (for display)
        int writeMessage(String message, bool autowrap = true) {
            for (int i = 0; i < message.length(); i ++) {
                if(autowrap && i == 16){
                    setCursor(1, 0);
                }
                Serial.println((int)message.charAt(i));
                write((int)message.charAt(i));
                delay(1);
            }
        }
        
        //Set the direction of the cursor and display
        // void entryModeSet(bool cursorRight,     //Set direction of cursor and display
        //                   bool displayShift){
        //     if(!cursorRight && !displayShift){
        //         writeCommand(0b00000100);       //cursor left, display shift off
        //     }
        //     if(!cursorRight && displayShift){
        //         writeCommand(0b00000101);       //cursor left, display shift on
        //     }
        //     if(cursorRight && !displayShift){
        //         writeCommand(0b00000110);       //cursor right, display shift off***
        //     }
        //     if(cursorRight && displayShift){
        //         writeCommand(0b00000111);       //cursor right, display shift on
        //     }
        //     delayMicroseconds(50);
        // }
};



/**
 * @brief Control commands available for the robot.
 */
#define FORWARD     'F'
#define LEFT        'L'
#define BACKWARD    'B'
#define RIGHT       'R'
#define STOP        'S'
#define FREE        'I'
#define SETSPEED    'V'
#define INCSPEED    'O'
#define DECSPEED    'P'

/**
 * @brief Start and end delimiters
 * @details Ideally these would be high ASCII characters and MUST
 * be something that doesn't occur inside of a legal packet!
 */
const char SoP = 'C';
const char EoP = 'E';

/**
 * @brief Other constants and variables for communication
 */
const char nullTerminator = '\0';
unsigned char inByte;
#define MESSAGE_MAX_SIZE 42
char message[MESSAGE_MAX_SIZE];
char command;
char previousCommand = STOP;

/**
 * @brief Definitions of different speed levels.
 * @details Here we define idle as 0 and full speed as 150. For now,
 * we don't have intermediate speed levels in the program. But you
 * are welcome to add them and change the code.
 */
const int IDLE  = 0;
int SPEED = 75;
int localSpeed = SPEED;
//const int SPEED = 150;
//const int HALFSPEED = 75;

/**
 * @brief H bridge (SN754410) pin connections to Arduino.
 * @details The main ideas is to drive two enable signals with PWMs to
 * control the speed. Connect the 4 logic inputs to any 4 Arduino
 * digital pins. We can also connect enables to vcc to save pins, but
 * then we need 2 or 4 pwm pins. These methods are all feasible, but you
 * need to adjust the setup and the function 'motorControl' accordingly.
 */
/// pwm pin, control left motor
const int EN1 = 10;
/// pwm pin, control right motor
const int EN2 = 9;
/// control Y1 (left motor positive)
const int A_1 = 8;
/// control Y2 (left motor negative)
const int A_2 = 11;
/// control Y3 (right motor positive)
const int A_3 = 12;
/// control Y4 (right motor negative)
const int A_4 = 13;
#define LEFT_MOTOR  true
#define RIGHT_MOTOR false

//LCD pins
#define RS_pin 7
#define RW_pin 14
#define E_pin 6
#define DB4_pin 5
#define DB5_pin 4
#define DB6_pin 3
#define DB7_pin 2

/**
 * @brief Packet parser for serial communication
 * @details Called in the main loop to get legal message from serial port.
 * @return true on success, false on failure
 */
bool parsePacket();

/**
 * @brief Control the robot
 * @param command FORWARD, LEFT, BACKWARD or STOP.
 */
void moveRobot(char command);

/**
 * @brief Control motor
 * @details Called by `moveRobot` to break down high level command
 * to each motor.
 *
 * @param ifLeftMotor Either Left motor or right motor.
 * @param command FORWARD, STOP or BACKWARD
 */
void motorControl(bool ifLeftMotor, char command);

/**
 * @brief Pin setup and initialize state
 * @details Enable serial communication, set up H bridge connections,
 * and make robot stop at the beginning.
 */

LCDInterface lcd(RS_pin, RW_pin, E_pin, DB4_pin, DB5_pin, DB6_pin, DB7_pin);

void setup() {
    Serial.begin(9600);
    Serial.println("START");
    pinMode(EN1, OUTPUT);
    pinMode(EN2, OUTPUT);
    pinMode(A_1, OUTPUT);
    pinMode(A_2, OUTPUT);
    pinMode(A_3, OUTPUT);
    pinMode(A_4, OUTPUT);

    lcd.init();
    lcd.print("I'm a robot.");
    lcd.setCursor(1, 0);
    lcd.print("A real one.");

    moveRobot(STOP);
    delay(10);
    Serial.println("Ready, Steady, Go");
    delay(10);
}

/**
 * @brief Main loop of the program
 * @details In this function, we get messages from serial port and
 * execute them accordingly.
 */
void loop() {
    /// 1. get legal message
    if (!parsePacket())
        return;

    /// 2. action, for now we only use option 1
    if (message[0] == '1') {
        Serial.println(message);
        // Move command
        command = message[1];
        if (command == SETSPEED || command == INCSPEED || command == DECSPEED) {
            // Set speed command
            
            if (command == SETSPEED) {
                localSpeed = (message[2] - '0') * 100 + (message[3] - '0') * 10 + (message[4] - '0');
            }
            else {
                if (command == INCSPEED) {
                    localSpeed += 1;
                }
                else {
                    localSpeed -= 1;
                }
            }
            localSpeed = localSpeed > 150 ? 150 : localSpeed;
            localSpeed = localSpeed < 0 ? 0 : localSpeed;
            Serial.println(localSpeed);
            setRobotSpeed(localSpeed);
            moveRobot(previousCommand);
        } else {
            // Move command
            moveRobot(command);
            previousCommand = command;
        }
    }
    else if (message[0] == '2') {
        // Display Read
        // ...
    }
    else if (message[0] == '3') {
        // Distance Read
        // ...
    }
    else if (message[0] == '4') {
        // Display Write
        writeDisplay(String(message));
        Serial.println(message);
        return;
    }
    else if (message[0] == '5') {
        Serial.println(message);
        writeBinary(String(message));
    }
    else {
        Serial.println("ERROR: unknown message");
        return;
    }
}


/**
 * @brief FUNCTION IMPLEMENTATIONS BELOW
 */
bool parsePacket() {
    /// step 1. get SoP
    while (Serial.available() < 1) {};
    inByte = Serial.read();
    if (inByte != SoP) {
        Serial.print("ERROR: Expected SOP, got: ");
        Serial.write((byte)inByte);
        Serial.print("\n");
        return false;
    }

    /// step 2. get message length
    while (Serial.available() < 1) {};
    inByte = Serial.read();
    if (inByte == EoP || inByte == SoP) {
        Serial.println("ERROR: SoP/EoP in length field");
        return false;
    }
    int message_size = inByte - '0';
    if (message_size > MESSAGE_MAX_SIZE || message_size < 0) {
        Serial.println("ERROR: Packet Length out of range");
        return false;
    }

    /// step 3. get message
    for (int i = 0; i < message_size; i++) {
        while (Serial.available() < 1) {};
        inByte = Serial.read();
        // if ((inByte == EoP || inByte == SoP)) {
        //     Serial.println("ERROR: SoP/EoP in command field");
        //     return false;
        // }
        message[i] = (char)inByte;
    }
    message[message_size] = nullTerminator;

    /// step 4. get EoP
    while (Serial.available() < 1) {};
    inByte = Serial.read();
    if (inByte != EoP) {
        Serial.println("EoP not found");
        return false;
    } else {
        return true;
    }
}

void writeDisplay(String message) {
    String text = message.substring(1);
    lcd.clear();
    lcd.print(text, 0, 0);
}

void writeBinary(String message){
    int command = 0b00000000;
    for(int i = 0; i < message.length() - 1; i++){
        command = command | ((((int)(message.charAt(message.length() - i - 1) - '0')) & 0b1) << i);
        // Serial.println("#");
        // Serial.println(message.charAt(message.length() - i - 1) - '0');
        // Serial.println((int)(message.charAt(message.length() - i - 1) - '0') >> i);
        // Serial.println((((int)(message.charAt(message.length() - i - 1) - '0') >> i) & 0b1));
        // Serial.println(((((int)(message.charAt(message.length() - i - 1) - '0') >> i) & 0b1) << i));
    }
    Serial.println(String(command));
    lcd.write(command);
}

void setRobotSpeed(int speed) {
    SPEED = speed;
}

void moveRobot(char command) {
    switch(command) {
        case FORWARD:
            Serial.println("FORWARD");
            motorControl(LEFT_MOTOR, FORWARD);
            motorControl(RIGHT_MOTOR, FORWARD);
            break;
        case LEFT:
            Serial.println("LEFT");
            motorControl(LEFT_MOTOR, STOP);
            motorControl(RIGHT_MOTOR, FORWARD);
            break;
        case BACKWARD:
            Serial.println("BACKWARD");
            motorControl(LEFT_MOTOR, BACKWARD);
            motorControl(RIGHT_MOTOR, BACKWARD);
            break;
        case RIGHT:
            Serial.println("RIGHT");
            motorControl(LEFT_MOTOR, FORWARD);
            motorControl(RIGHT_MOTOR, STOP);
            break;
        case STOP:
            Serial.println("STOP");
            motorControl(LEFT_MOTOR,STOP);
            motorControl(RIGHT_MOTOR,STOP);
            break;
        case FREE:
            Serial.println("FREE");
            motorControl(LEFT_MOTOR,FREE);
            motorControl(RIGHT_MOTOR,FREE);
            break;
        default:
            Serial.println("ERROR: Unknown command in legal packet");
            break;
    }
}

/**
 * @brief Please rewrite the function if you change H bridge connections.
 */
void motorControl(bool ifLeftMotor, char command) {
    int enable   = ifLeftMotor ? EN1 : EN2;
    int motorPos = ifLeftMotor ? A_1 : A_3;
    int motorNeg = ifLeftMotor ? A_2 : A_4;
    switch (command) {
        case FORWARD:
            analogWrite(enable, SPEED);
            digitalWrite(motorPos, HIGH);
            digitalWrite(motorNeg, LOW);
            break;
        case BACKWARD:
            analogWrite(enable, SPEED);
            digitalWrite(motorPos, LOW);
            digitalWrite(motorNeg, HIGH);
            break;
        case STOP:
            digitalWrite(motorPos, LOW);
            digitalWrite(motorNeg, LOW);
            break;
        case FREE:
            //IDLE 
            analogWrite(enable, IDLE);
            break;
        default:
            break;
    }
}
