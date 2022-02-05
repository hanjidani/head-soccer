import cv2
import numpy as np
import os
import time

def incircle(x, y):
    temp = (x - 50)**2 + (y - 50)**2
    if temp <= 2500:
        return True
    return False

def crop(frame):
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2BGRA)
    for x in range(frame.shape[0]):
        for y in range(frame.shape[1]):
            if not incircle(x, y):
                frame[x][y][3] = 0
    return frame
# capture video
cam = cv2.VideoCapture(0)

pl1 = "1"
pl2 = "1"

ret, frame = cam.read()
cv2.imshow("Hi Sexy Face ...", frame)
c = 0
while True:
    c += 1
    ret, frame = cam.read()
    if not ret:
        break
    data = cv2.getWindowImageRect("Hi Sexy Face ...")
    wi = data[2]
    he = data[3]
    start = (int(wi / 2 - 150), int(he / 2 - 150))
    end = (int(wi / 2 + 150), int(he / 2 + 150))
    fcopy = frame
    frame = cv2.circle(frame, (int(wi / 2), int(he / 2)), 150, (0, 0, 255), 1)
    frame = cv2.putText(frame, "Press space to cpature or ESC to ignore :|", (30, 20), cv2.FONT_HERSHEY_SIMPLEX, .8, (0, 0, 255), 1, cv2.LINE_AA)
    cv2.imshow("Hi Sexy Face ...", frame)
    now = "Stick/" + str(int(time.time())).replace(" ", "") + ".png"
    
    k = cv2.waitKey(1)

    if k%256 == 27:
        # ESC pressed
        cv2.imwrite(now, fcopy)
        break
    elif k%256 == 32:
        # SPACE pressed
        cv2.imwrite(now, fcopy)
        pl1 = "Textures/Player/Head/D.png"
        fcopy = fcopy[start[1]:end[1],start[0]:end[0]]
        fcopy = cv2.resize(fcopy, (100, 100), interpolation = cv2.INTER_AREA)
        fcopy = crop(fcopy)
        cv2.imwrite(pl1, fcopy)
        cv2.imwrite("pic/head/13.png", fcopy)
        break
cv2.destroyAllWindows()


ret, frame = cam.read()
cv2.imshow("Hi Sexy Face ...", frame)
while True:
    ret, frame = cam.read()
    if not ret:
        break
    data = cv2.getWindowImageRect("Hi Sexy Face ...")
    wi = data[2]
    he = data[3]
    start = (int(wi / 2 - 150), int(he / 2 - 150))
    end = (int(wi / 2 + 150), int(he / 2 + 150))
    fcopy = frame
    frame = cv2.circle(frame, (int(wi / 2), int(he / 2)), 150, (0, 0, 255), 1)
    frame = cv2.putText(frame, "Press space to cpature or ESC to ignore :|", (30, 20), cv2.FONT_HERSHEY_SIMPLEX, .8, (0, 0, 255), 1, cv2.LINE_AA)
    cv2.imshow("Hi Sexy Face ...", frame)
    now = "Stick/" + str(int(time.time())).replace(" ", "") + ".png"
    k = cv2.waitKey(1)
    if k%256 == 27:
        # ESC pressed
        cv2.imwrite(now, fcopy)
        break
    elif k%256 == 32:
        # SPACE pressed
        cv2.imwrite(now, fcopy)
        fcopy = fcopy[start[1]:end[1],start[0]:end[0]]
        fcopy = cv2.resize(fcopy, (100, 100), interpolation = cv2.INTER_AREA)
        fcopy = crop(fcopy)
        if not pl1 == "1":
            pl2 = "Textures/Player/Head/E.png"
            cv2.imwrite(pl2, fcopy)
            cv2.imwrite("pic/head/14.png", fcopy)
        else:
            pl2 = "Textures/Player/Head/D.png"
            cv2.imwrite(pl2, fcopy)
            cv2.imwrite("pic/head/13.png", fcopy)
        break
cam.release()
cv2.destroyAllWindows()

# send data to cpp app
print(os.system("startmenu.exe" + " " + str(pl1) + " " + str(pl2)))