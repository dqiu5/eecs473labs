'''
Object detection and tracking with OpenCV
    ==> Turning a LED on detection and
    ==> Printing object position Coordinates 

    Based on original tracking object code developed by Adrian Rosebrock
    Visit original post: https://www.pyimagesearch.com/2016/05/09/opencv-rpi-gpio-and-gpio-zero-on-the-raspberry-pi/

Developed by Marcelo Rovai - MJRoBot.org @ 9Feb2018 
'''

# import the necessary packages
from __future__ import print_function
from imutils.video import VideoStream
import imutils
import time
import cv2
import os
import RPi.GPIO as GPIO
import numpy as np
import sys

np.set_printoptions(threshold = sys.maxsize)

# initialize LED GPIO
redLed = 21
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(redLed, GPIO.OUT)

#check center bounds
leftbound = 100
rightbound = 400
leftInnerbound = 200
rightInnerbound = 300

outside_flag = True

# print object coordinates
def mapObjectPosition (x, y):
    print ("[INFO] Object Center coordenates at X0 = {0} and Y0 =  {1}".format(x, y))

def checkCenter(x, leftbound, rightbound):
    if(x > leftbound and x < rightbound):
        return 0
    elif(x <= leftbound):
        return 1
    elif(x >= rightbound):
        return 2
    else:
        return 3

def write_to_memory(inst):
    f = open("/dev/memory", "a")
    f.write(inst)
    f.close()

def move_robot(inst):
    write_to_memory(inst)

# initialize the video stream and allow the camera sensor to warmup
print("[INFO] waiting for camera to warmup...")
vs = VideoStream(0).start()
time.sleep(5.0)

# define the lower and upper boundaries of the object
# to be tracked in the HSV color space
colorLower = (24, 100, 100)
colorUpper = (44, 255, 255)

# Start with LED off
GPIO.output(redLed, GPIO.LOW)
ledOn = False

# loop over the frames from the video stream
while True:
    # grab the next frame from the video stream, Invert 180o, resize the
    # frame, and convert it to the HSV color space
    frame = vs.read()
    frame = imutils.resize(frame, width=500)
    frame = imutils.rotate(frame, angle=180)

    # print(frame)
    # break

    # cv2.imshow('frame', frame)
    # if cv2.waitKey(1) & 0xFF == ord('q'):
    #     break
    # continue

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    # print(hsv)
    # break

    # construct a mask for the object color, then perform
    # a series of dilations and erosions to remove any small
    # blobs left in the mask
    mask = cv2.inRange(hsv, colorLower, colorUpper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    # find contours in the mask and initialize the current
    # (x, y) center of the object
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE)
    #print(cnts)
    cnts = cnts[0] #if imutils.is_cv2() else cnts[1]
    center = None

    # only proceed if at least one contour was found
    if len(cnts) > 0:
        # find the largest contour in the mask, then use
        # it to compute the minimum enclosing circle and
        # centroid
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

        # only proceed if the radius meets a minimum size
        if radius > 10:
            print("center: {},{}".format(int(x), int(y)))

            # draw the circle and centroid on the frame,
            # then update the list of tracked points
            cv2.circle(frame, (int(x), int(y)), int(radius),
                (0, 255, 255), 2)
            cv2.circle(frame, center, 5, (0, 0, 255), -1)
            
            # position Servo at center of circle
            mapObjectPosition(int(x), int(y))
            
            # if the led is not already on, turn the LED on
            if not ledOn:
                GPIO.output(redLed, GPIO.HIGH)
                ledOn = True

        if (outside_flag):
            centerVal = checkCenter(int(x), leftInnerbound, rightInnerbound)
        else:
            centerVal = checkCenter(int(x), leftbound, rightbound)
        if(centerVal == 0):
            move_robot('F')
            outside_flag = False
        elif(centerVal == 1):
            move_robot('L')
            outside_flag = True
        elif(centerVal == 2):
            move_robot('R')
            outside_flag = True
        else:
            move_robot('S')

    # if the ball is not detected, turn the LED off
    elif ledOn:
        GPIO.output(redLed, GPIO.LOW)
        ledOn = False

    # show the frame to our screen
    cv2.imshow("Frame", frame)

    # if [ESC] key is pressed, stop the loop
    key = cv2.waitKey(1) & 0xFF
    if key == 27:
            break

# do a bit of cleanup
print("\n [INFO] Exiting Program and cleanup stuff \n")
GPIO.cleanup()
cv2.destroyAllWindows()
vs.stop()