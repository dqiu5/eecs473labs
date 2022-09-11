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
        LCDInterface(int RS, int RW, int E, int DB0, int DB1, int DB2, int DB3, int DB4, int DB5, int DB6, int DB7);

        // Constructor for 4 bit mode
        LCDInterface(int RS, int RW, int E, int DB4, int DB5, int DB6, int DB7);

        // initialize LCD and pins on arduino
        void init();
        
        // Config dot mode, line mode, display, cursor, blink according to user
        void config( bool dotMode,                               //(0)5x8 dot matrix (up to two lines) (1)5x10 dot matrix
                     bool lineMode,                              //(0)one line mode (1)two line mode
                     bool blinkOn,                               //true to turn on cursor blink
                     bool cursorOn,                              //true to turn on cursor
                     bool displayOn);                            //true to turn on display     

        /* 
         * Replace the current content with text and its length from the specified row and col
         * 
         * When write empty strings with certain length, it will work as delete function start
         * from the position with the length of the empty string
         * 
         * When row are set to -1, this function should append text to the current location of the cursor
         *
         */
        void print(String message, int row = -1, int col = -1);
        
        // write arbitary data, in int
        void write(int data); 

        // set cursor to specified spot on display
        void setCursor(int row, int col);
        
        // set cursor to 1st row and 1st column (upper-left corner)
        void cursorHome();

        // clear what is currently displayed and places cursor at upper left corner
        void clear(int row = -1);

    //"Backdoor"
    private:
        // initialize pins on arduino
        void initPins(int RS, int RW, int E, int DB[8]);

        //convert data into individual bits for the pin outputs
        void writeArray(int pins[8], int data);
        
        //enable (E) pin pulse
        void pulse(int pin, int delayNum);
        
        // Set the address of CGRAM
        void setCGRAMAddr(int CGRAM_addr);

        // Set the address of DDRAM
        void setDDRAMAddr(int DDRAM_addr);

        //Send command routine to the pins based on bit mode
        void writeCommand(int command, bool isStart = false);

        // writes a binary 8-bit data to CGRAM at address location
        void writeCGRAM(int data, int addr);
        
        // writes a binary 8-bit data to DDRAM at address location
        void writeDDRAM(int data, int addr);

        //convert string to byte data (for display)
        int writeMessage(String message, bool autowrap = true);
        
        //Set the direction of the cursor and display
        void entryModeSet(bool cursorRight,     //Set direction of cursor and display
                          bool displayShift);
};