import cv2
import threading
import numpy as np

def createImageDisplay(display):
    cv2.imshow("cropped", display)
    cv2.waitKey(0)

img = cv2.imread("mandel-small.jpg")
height, width, channel = img.shape

concat = cv2.hconcat([
    img[round(height/2):height, 0:width], # First image.
    cv2.rotate( # Second Image
        img[0:round(height/2), 0:width], 
        cv2.cv2.ROTATE_180
    )
])

createImageDisplay(concat)
# threading.Thread(target=createImageDisplay, args=(top_half,)).start()
# threading.Thread(target=createImageDisplay, args=(bottom_half,)).start()