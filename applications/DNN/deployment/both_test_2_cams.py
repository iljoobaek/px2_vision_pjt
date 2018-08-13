import ctypes
import cv2
import tensorflow as tf
import numpy as np
import gc
from time import sleep

#global _test
#global _test2

_test = ctypes.CDLL('./both_2.so')#, mode = ctypes.RTLD_GLOBAL)
#_test2 = ctypes.CDLL('./getImgBuffer.so')

_test.eglconsumer_main.argtypes = ()
_test.getImgBuffer.argtype = (ctypes.POINTER(ctypes.c_ubyte))

_test.eglconsumer_main()

sleep(1)

img_buf = ((2319360 * 2) * ctypes.c_ubyte) ()
while(1):
    result = _test.getImgBuffer(ctypes.cast(img_buf, ctypes.POINTER(ctypes.c_ubyte)))
    #print(type(img_buf))

    nd_arr = np.ctypeslib.as_array(img_buf)
    nd_arr_rs = np.reshape(nd_arr, (604, 1920, 4))
    
    #print nd_arr_rs.shape
    img = cv2.cvtColor(nd_arr_rs, cv2.COLOR_RGBA2BGR);

    '''top_img = img[0:604, 0:1920]
    bot_img = img[0:604, 1920:3840]

    img = np.vstack((top_img, bot_img))

    img = cv2.resize(img, (1280, 640))'''

    #img = nd_arr_rs[:,:,:3]

    cv2.imshow('img', img)
    key = cv2.waitKey(2)
    if(key == 27):
        break
