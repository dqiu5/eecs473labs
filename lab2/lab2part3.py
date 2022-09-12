#from msilib.schema import Binary
from pynput.keyboard import Key, Listener, KeyCode
import serial
ser = serial.Serial('/dev/cu.usbserial-A9ETXC71', 9600)



def on_press(key):
    #print('{0} pressed'.format(key))
    global enterMode
    global textBuffer
    global binaryMode

    if enterMode:
        if key == Key.enter:
            print(textBuffer)
            print("\nEnter finished.")
            textMsg = "C" + chr(ord('0') + (len(textBuffer) + 1)) + "4" + "".join(textBuffer) + "E"
            print(textMsg)
            ser.write(bytes(textMsg, 'utf-8'))
            enterMode = False
            textBuffer = []
        else:
            try:
                if (key == Key.space):
                    textBuffer.append(' ')
                else:
                    #print(str(key.char), end = '')
                    #if capsFlag:
                    #    textBuffer.append(str(key.char).upper())
                    #else:
                    textBuffer.append(str(key.char))
            except AttributeError:
                pass
    elif binaryMode:
        if key == Key.caps_lock:
            print(textBuffer)
            print("\nEnter finished.")
            if (len(textBuffer) != 8):
                print("Error: binary mode requires 8 bits.")
            else:
                textMsg = "C" + chr(ord('0') + (len(textBuffer) + 1)) + "5" + "".join(textBuffer) + "E"
                print(textMsg)
                ser.write(bytes(textMsg, 'utf-8'))
            binaryMode = False
            textBuffer = []
        else:
            try:
                #print(key.char == "1"  key.char == "0")
                #print(type(key.char))
                if (key.char != "1" and key.char != "0"):
                    print("Error: Binary mode only accepts 0 and 1.")
                else:
                    textBuffer.append(str(key.char))
            except AttributeError:
                print("Error: Binary mode only accepts 0 and 1.")
    else:
        if key == Key.enter:
            print("\nPlease enter text:")
            enterMode = True
        elif key == Key.caps_lock:
            print("\nPlease enter binary:")
            binaryMode = True
        elif key == KeyCode.from_char('o') or key == KeyCode.from_char('p') or key == Key.ctrl_l or key == Key.shift:
            if (key == KeyCode.from_char('o') or key == Key.shift):
                for i in range(3):
                    ser.write(bytes('C21OE', 'utf-8'))
            elif (key == KeyCode.from_char('p') or key == Key.ctrl_l):
                for i in range(3):
                    ser.write(bytes('C21PE', 'utf-8'))
        # maxspeed
        elif key == KeyCode.from_char('z'):
            ser.write(bytes('C51V150E', 'utf-8'))
        # minspeed
        elif key == KeyCode.from_char('x'):
            ser.write(bytes('C51V000E', 'utf-8'))
        #control keys
        elif key == KeyCode.from_char('w'):
            ser.write(bytes('C21FE', 'utf-8'))
        elif key == KeyCode.from_char('a'):
            ser.write(bytes('C21LE', 'utf-8'))
        elif key == KeyCode.from_char('s'):
            ser.write(bytes('C21BE', 'utf-8'))
        elif key == KeyCode.from_char('d'):
            ser.write(bytes('C21RE', 'utf-8'))
        elif key == Key.space:
            ser.write(bytes('C21SE', 'utf-8'))

def on_release(key):
    #print('{0} release'.format(key))
    if key == Key.esc:
        # Stop listener
        return False
    elif key == KeyCode.from_char('o') or key == KeyCode.from_char('p') or key == Key.ctrl_l or key == Key.shift or key == KeyCode.from_char('z') or key == KeyCode.from_char('x'):
        pass
    else:
        ser.write(bytes('C21IE', 'utf-8'))
    

# Collect events until released
with Listener(
        on_press=on_press,
        on_release=on_release) as listener:
    textBuffer = []
    binaryMode = False
    enterMode = False
    listener.join()