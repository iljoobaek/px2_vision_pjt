import ctypes
import cv2
import numpy as np
np.set_printoptions(threshold=np.nan)

_test = ctypes.CDLL('./server.so')
_test.getImgBuffer.argtypes = (ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int)

def make_nd_array(c_pointer, shape, dtype=np.int16, order='C', own_data=True):
    arr_size = np.prod(shape[:]) * np.dtype(dtype).itemsize
    print(1)
    buf_from_mem = ctypes.pythonapi.PyBuffer_FromMemory
    print(2)
    buf_from_mem.restype = ctypes.py_object
    print(3)
    buffer = buf_from_mem(c_pointer, arr_size)
    print(4)
    arr = np.ndarray(tuple(shape[:]), dtype, buffer, order=order)
    print(5)
    return arr

def test():
    global _test
    while(1):
        #array_type = (18984960 * ctypes.c_ubyte) ()
        array_type = (18984960 / 2 * ctypes.c_ushort) ()
        result = _test.getImgBuffer(ctypes.cast(array_type, ctypes.POINTER(ctypes.c_ubyte)), ctypes.c_int(7680), ctypes.c_int(1208))
        print(type(array_type))

        a = np.ctypeslib.as_array(array_type)
        a1 = np.reshape(a, (1236,7680,1))

        shape = np.array([1236,7680,1])
        #a = make_nd_array(array_type, shape)
        print(6)
        print(type(a))
        print(a.shape)
        #b = a1 * 4
        #b1 = np.uint8(b)
        b1 = cv2.convertScaleAbs(a1, alpha=(0.015625))
        c = cv2.cvtColor(b1, cv2.COLOR_BAYER_GB2BGR)
        c1 = cv2.resize(c, (2560,360))
        cv2.imshow('c', c1)
        cv2.waitKey(2)


    #print(a)
    #img = cv2.imdecode(a, -1)
    #print(img) 
    #height, width, channels = img.shape
    #print height, width, channels
    #print(img.type)
    #cv2.imshow('img', img)
    #cv2.waitKey(5)
    #img_data_ndarray = cv2.imdecode(file_bytes, 1)
    #cv2.imshow('img', img_data_ndarray)
    #cv2.waitKey(5)
    return int(result)
