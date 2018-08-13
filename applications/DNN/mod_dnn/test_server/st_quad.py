import ctypes
import cv2
import numpy as np
import gc
np.set_printoptions(threshold=np.nan)

_test = ctypes.CDLL('./server.so')
_test.getImgBuffer.argtypes = (ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int)

def make_nd_array(c_pointer, shape, dtype=np.int16, order='C', own_data=True):
    arr_size = np.prod(shape[:]) * np.dtype(dtype).itemsize
    buf_from_mem = ctypes.pythonapi.PyBuffer_FromMemory
    buffer = buf_from_mem(c_pointer, arr_size)
    arr = np.ndarray(tuple(shape[:]), dtype, buffer, order=order)
    return arr

def test():
    img_counter = 0
    global _test

    img_buf = (18984960 / 2 * ctypes.c_ushort) ()
    while(1):
        result = _test.getImgBuffer(ctypes.cast(img_buf, ctypes.POINTER(ctypes.c_ubyte)), ctypes.c_int(7680), ctypes.c_int(1208))
        #print(type(img_buf))

        nd_arr = np.ctypeslib.as_array(img_buf)
        nd_arr_rs = np.reshape(nd_arr, (1236,7680,1))

        bayer_img = cv2.convertScaleAbs(nd_arr_rs, alpha=(0.015625))
        rgb_img = cv2.cvtColor(bayer_img, cv2.COLOR_BAYER_GB2BGR)
        rgb_img_scaled = cv2.resize(rgb_img, (2560,360))

        #C++ equivs
        #top_img = rgb_img_scaled(Rect(0,0,1280,360));
        #bot_img = rgb_img_scaled(Rect(1280,0,1280,360));
        #vconcat(top_img, bot_img, quad_img);

        top_img = rgb_img_scaled[0:360, 0:1280]
        bot_img = rgb_img_scaled[0:360, 1280:2560]
        quad_img = np.vstack((top_img, bot_img))

        cv2.imshow('quad_img', quad_img)
        cv2.waitKey(2)
        img_counter += 1
        print("Image #: ", img_counter)
        '''img_buf = None
        result = None
        nd_arr = None
        nd_arr_rs = None
        bayer_img = None
        rgb_img = None
        rgb_img_scaled = None
        top_img = None
        bot_img = None
        quad_img = None'''
        '''del img_buf
        del result
        del nd_arr
        del nd_arr_rs
        del bayer_img
        del rgb_img
        del rgb_img_scaled
        del top_img
        del bot_img
        del quad_img'''
        #gc.collect()
    return int(result)
