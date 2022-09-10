/**
 * @brief Control commands available for the robot.
 */
#define FORWARD     'F'
#define LEFT        'L'
#define BACKWARD    'B'
#define RIGHT       'R'
#define STOP        'S'

/**
 * @brief Start and end delimiters
 * @details Ideally these would be high ASCII characters and MUST
 * be something that doesn't occur inside of a legal packet!
 */
const char SoP = 'C';
const char EoP = 'E';

/**
 * Carry previous detected start of packet to ignore random characters
 */
bool StartCarry = false;

/**
 * @brief Other constants and variables for communication
 */
const char nullTerminator = '\0';
unsigned char inByte;
#define MESSAGE_MAX_SIZE 5
char message[MESSAGE_MAX_SIZE];
char command;

/**
 * @brief Definitions of different speed levels.
 * @details Here we define idle as 0 and full speed as 150. For now,
 * we don't have intermediate speed levels in the program. But you
 * are welcome to add them and change the code.
 */
const int IDLE  = 0;
const int SPEED = 150;

/**
 * @brief H bridge (SN754410) pin connections to Arduino.
 * @details The main ideas is to drive two enable signals with PWMs to
 * control the speed. Connect the 4 logic inputs to any 4 Arduino
 * digital pins. We can also connect enables to vcc to save pins, but
 * then we need 2 or 4 pwm pins. These methods are all feasible, but you
 * need to adjust the setup and the function 'motorControl' accordingly.
 */
/// pwm pin, control left motor
const int EN1 = 3;
/// pwm pin, control right motor
const int EN2 = 5;
/// control Y1 (left motor positive)
const int A_1 = 7;
/// control Y2 (left motor negative)
const int A_2 = 8;
/// control Y3 (right motor positive)
const int A_3 = 9;
/// control Y4 (right motor negative)
const int A_4 = 13;
#define LEFT_MOTOR  true
#define RIGHT_MOTOR false

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
void setup() {
    Serial.begin(9600);
    Serial.println("START");
    pinMode(EN1, OUTPUT);
    pinMode(EN2, OUTPUT);
    pinMode(A_1, OUTPUT);
    pinMode(A_2, OUTPUT);
    pinMode(A_3, OUTPUT);
    pinMode(A_4, OUTPUT);

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
        // Move command
        command = message[1];
        moveRobot(command);
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
        Serial.println(message);
        return;
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
    if (!StartCarry) {
        if (inByte != SoP) {
            Serial.print("ERROR: Expected SOP, got: ");
            Serial.write((byte)inByte);
            Serial.print("\n");
            return false;
        }

        /// step 2. get message length
        while (Serial.available() < 1) {};
        inByte = Serial.read();
    }
    else  {
        StartCarry = false;
    }
    if (inByte == SoP) {
        StartCarry = true;
        Serial.println("ERROR: SoP/EoP in length field");
        return false;
    }
    else if (inByte == EoP) {
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
        if ((inByte == EoP || inByte == SoP)) {
            Serial.println("ERROR: SoP/EoP in command field");
            return false;
        }
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
        default:
            break;
    }
}