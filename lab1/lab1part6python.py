import msvcrt
import serial
ser = serial.Serial('COM9', 9600)
while 1:
    key = b""
    # Poll keyboard
    if msvcrt.kbhit():
        key = msvcrt.getch()
        print(key)
        msg = 'C21' + str(key,'utf-8').capitalize() + 'E'
        print(msg)
        ser.write(bytes(msg, 'utf-8'))
    '''
    if key == b'f':
        ser.write(b'C21FE')
    elif key == b's':
        ser.write(b'C21SE')
    elif key == b'b':
        ser.write(b'C21BE')
    elif key == b'l':
        ser.write(b'C21LE')
    elif key == b'r':
        ser.write(b'C21RE')
    elif key == b'i':
        ser.write(b'C21IE')
    '''