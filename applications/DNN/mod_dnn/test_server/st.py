import ctypes
import cv2
import numpy as np
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
    global _test
    while(1):
        img_buf = (18984960 / 2 * ctypes.c_ushort) ()
        result = _test.getImgBuffer(ctypes.cast(img_buf, ctypes.POINTER(ctypes.c_ubyte)), ctypes.c_int(7680), ctypes.c_int(1208))
        print(type(img_buf))

        nd_arr = np.ctypeslib.as_array(img_buf)
        nd_arr_rs = np.reshape(nd_arr, (1236,7680,1))

        bayer_img = cv2.convertScaleAbs(nd_arr_rs, alpha=(0.015625))
        rgb_img = cv2.cvtColor(bayer_img, cv2.COLOR_BAYER_GB2BGR)
        rgb_img_scaled = cv2.resize(rgb_img, (2560,360))
        cv2.imshow('rgb_img_small', rgb_img_scaled)
        cv2.waitKey(2)
    return int(result)
