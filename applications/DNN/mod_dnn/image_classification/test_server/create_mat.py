import numpy as np
import cv2

vis = np.zeros((384, 836), np.float32)
h,w = vis.shape
vis2 = cv2.CreateMat(h,w,cv.CV_32FC3)
vis0 = cv2.fromarray(vis)
