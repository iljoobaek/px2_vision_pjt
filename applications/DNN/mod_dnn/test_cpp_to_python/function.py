import ctypes

_square = ctypes.CDLL('./library.so')
#_square.square.argtypes = (ctypes.c_int)

def square(n):
    global _square
    result = _square.square(ctypes.c_int(n))
    return int(result)
