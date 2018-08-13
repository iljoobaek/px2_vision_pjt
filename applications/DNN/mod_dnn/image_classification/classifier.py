# Copyright 2015 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

"""Simple image classification with Inception.

Run image classification with Inception trained on ImageNet 2012 Challenge data
set.

This program creates a graph from a saved GraphDef protocol buffer,
and runs inference on an input JPEG image. It outputs human readable
strings of the top 5 predictions along with their probabilities.

Change the --image_file argument to any jpg image to compute a
classification of that image.

Please see the tutorial and website for a detailed description of how
to use this script to perform image recognition.

https://tensorflow.org/tutorials/image_recognition/
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import time
import argparse
#import cv2
import os
import re
import socket
import sys
# roi_1_height = 220
# roi_1_width = 120
# roi_2_height = 260
# roi_2_width = 240
#
# roi_1_px = 10
# roi_1_py = 40
# roi_2_px = 10
# roi_2_py = 200
# roi_3_px = 10
# roi_3_py = 480
#
# x_off = 360
# y_off = 640
roi_1_height_1 = 240
roi_1_width_1 = 150
roi_2_height_1 = 260
roi_2_width_1 = 300

roi_1_height_2 = 190
roi_1_width_2 = 145
roi_2_height_2 = 300
roi_2_width_2 = 290

roi_1_px_1 = 60
roi_1_py_1 = 20
roi_2_px_1 = 60
roi_2_py_1 = 170
roi_3_px_1 = 60
roi_3_py_1 = 470

roi_1_px_2 = 30
roi_1_py_2 = 30
roi_2_px_2 = 30
roi_2_py_2 = 175
roi_3_px_2 = 30
roi_3_py_2 = 465
x_off = 360
y_off = 640
im_size = 224
import numpy as np
import tensorflow as tf
from scipy.misc import imread,imresize
FLAGS = None

current_dir_path = os.path.dirname(os.path.realpath(__file__))

class NodeLookup(object):
  """Converts integer node ID's to human readable labels."""

  def __init__(self,
               label_lookup_path=None,
               uid_lookup_path=None):
    if not label_lookup_path:
      label_lookup_path = '/tmp/output_labels.txt'
    if not uid_lookup_path:
      uid_lookup_path = os.path.join(
          FLAGS.model_dir, 'imagenet_synset_to_human_label_map.txt')
    self.node_lookup = self.load(label_lookup_path, uid_lookup_path)

  def load(self, label_lookup_path, uid_lookup_path):
    """Loads a human readable English name for each softmax node.

    Args:
      label_lookup_path: string UID to integer node ID.
      uid_lookup_path: string UID to human-readable string.

    Returns:
      dict from integer node ID to human-readable string.
    """
    if not tf.gfile.Exists(uid_lookup_path):
      tf.logging.fatal('File does not exist %s', uid_lookup_path)
    if not tf.gfile.Exists(label_lookup_path):
      tf.logging.fatal('File does not exist %s', label_lookup_path)

    # Loads mapping from string UID to human-readable string
    proto_as_ascii_lines = tf.gfile.GFile(uid_lookup_path).readlines()
    uid_to_human = {}
    p = re.compile(r'[n\d]*[ \S,]*')

    for line in proto_as_ascii_lines:
      parsed_items = p.findall(line)
      uid = parsed_items[0]
      human_string = parsed_items[2]
      uid_to_human[uid] = human_string

    # Loads mapping from string UID to integer node ID.
      node_id_to_uid = {}
      proto_as_ascii = tf.gfile.GFile(label_lookup_path).readlines()

    for line in proto_as_ascii:
      if line.startswith('  target_class:'):
        target_class = int(line.split(': ')[1])
      if line.startswith('  target_class_string:'):
        target_class_string = line.split(': ')[1]
        node_id_to_uid[target_class] = target_class_string[1:-2]

    # Loads the final mapping of integer node ID to human-readable string
    node_id_to_name = {}
    for key, val in node_id_to_uid.items():
      if val not in uid_to_human:
        tf.logging.fatal('Failed to locate: %s', val)
      name = uid_to_human[val]
      node_id_to_name[key] = name

    return node_id_to_name



def id_to_string(self, node_id):
    if node_id not in self.node_lookup:
      return ''
    return self.node_lookup[node_id]


def create_graph():
  """Creates a graph from saved GraphDef file and returns a saver."""
  # Creates graph from saved graph_def.pb.
  with tf.gfile.FastGFile('/home/nvidia/mod_dnn/all/output141.pb','rb') as f:
    graph_def = tf.GraphDef()
    graph_def.ParseFromString(f.read())
    _ = tf.import_graph_def(graph_def, name='')


def run_inference_on_image(image):

  """Runs inference on an image.

  Args:
    image: Image file name.

  Returns:
    Nothing
  """
  # if not tf.gfile.Exists(image):
  #   tf.logging.fatal('File does not exist %s', image)
  # image_data = tf.gfile.FastGFile(image, 'rb').read()

  # Creates graph from saved GraphDef.
  s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
  s.bind(("127.0.0.1",8891))
  s.listen(1)
  print("waiting for connection")
  create_graph()
  PATH_TO_TEST_IMAGES_DIR = '/home/nvidia/mod_dnn/data_forbes'
  TEST_IMAGE_PATHS = [ os.path.join(PATH_TO_TEST_IMAGES_DIR, 'frame_{}.jpg'.format(i)) for i in range(1300) ]
  f = open('result_nouse.txt','w')
  with tf.Session() as sess:
    # Some useful tensors:
    # 'softmax:0': A tensor containing the normalized prediction across
    #   1000 labels.
    # 'pool_3:0': A tensor containing the next-to-last layer containing 2048
    #   float description of the image.
    # 'DecodeJpeg/contents:0': A tensor containing a string providing JPEG
    #   encoding of the image.
    # Runs the softmax tensor by feeding the image_data as input to the graph.
    softmax_tensor = sess.graph.get_tensor_by_name('final_result:0')
    conn, addr = s.accept()

    print("CCC")
    conn.sendall(str.encode("Hello\n"))
    iii = 0
    image_path = TEST_IMAGE_PATHS[iii]
    img = imread(image_path)
    img1 = img[60-1:60+240-1,30-1:30+140-1,:]
    img2 = img[60-1:60+290-1,170-1:170+300-1,:]
    img3 = img[60-1:60+240-1,470-1:470+150-1,:]
            
    img4 = img[60-1:60+155-1,640+30-1:640+30+145-1,:]
    img5 = img[60-1:60+265-1,640+175-1:640+175+290-1,:]
    img6 = img[60-1:60+175-1,640+465-1:640+465+145-1,:]
            
    img7 = img[360+80-1:360+80+215-1,60-1:60+110-1,:]
    img8 = img[360+80-1:360+80+260-1,170-1:170+300-1,:]
    img9 = img[360+80-1:360+80+260-1,470-1:470+150-1,:]
            
    img10 = img[360+40-1:360+40+190-1,640+30-1:640+30+145-1,:]
    img11 = img[360+40-1:360+40+285-1,640+175-1:640+175+290-1,:]
    img12 = img[360+40-1:360+40+235-1,640+465-1:640+465+145-1,:]
    im = np.zeros((12,im_size,im_size,3))
    im[0,:,:,:] = imresize(img1,(im_size,im_size))
    im[1,:,:,:] = imresize(img2,(im_size,im_size))
    im[2,:,:,:] = imresize(img3,(im_size,im_size))
    im[3,:,:,:] = imresize(img4,(im_size,im_size))
    im[4,:,:,:] = imresize(img5,(im_size,im_size))
    im[5,:,:,:] = imresize(img6,(im_size,im_size))
    im[6,:,:,:] = imresize(img7,(im_size,im_size))
    im[7,:,:,:] = imresize(img8,(im_size,im_size))
    im[8,:,:,:] = imresize(img9,(im_size,im_size))
    im[9,:,:,:] = imresize(img10,(im_size,im_size))
    im[10,:,:,:] = imresize(img11,(im_size,im_size))
    im[11,:,:,:] = imresize(img12,(im_size,im_size))
    while True:

        data = conn.recv(1024)

        if len(data) > 0 and data[0:6] == b'Frame:':




                # left above
            #CIC Day

            # img1 = img[60-1:60+230-1,30-1:30+140-1,:]
            # img2 = img[60-1:60+250-1,170-1:170+300-1,:]
            # img3 = img[60-1:60+230-1,470-1:470+150-1,:]
            #
            # img4 = img[30-1:30+165-1,640+30-1:640+30+145-1,:]
            # img5 = img[30-1:30+295-1,640+175-1:640+175+290-1,:]
            # img6 = img[30-1:30+205-1,640+465-1:640+465+145-1,:]
            #
            # img7 = img[360+60-1:360+60+225-1,30-1:30+140-1,:]
            # img8 = img[360+60-1:360+60+270-1,170-1:170+300-1,:]
            # img9 = img[360+60-1:360+60+240-1,470-1:470+150-1,:]
            #
            # img10 = img[360+30-1:360+30+215-1,640+30-1:640+30+145-1,:]
            # img11 = img[360+30-1:360+30+300-1,640+175-1:640+175+290-1,:]
            # img12 = img[360+30-1:360+30+185-1,640+465-1:640+465+145-1,:]

            #CICNight

           # img1 = img[60-1:60+220-1,30-1:30+140-1,:]
           # img2 = img[60-1:60+290-1,170-1:170+300-1,:]
           # img3 = img[60-1:60+240-1,470-1:470+150-1,:]
           #
           # img4 = img[100-1:100+115-1,640+30-1:640+30+145-1,:]
           # img5 = img[100-1:100+255-1,640+175-1:640+175+290-1,:]
           # img6 = img[100-1:100+235-1,640+465-1:640+465+145-1,:]
           #
           # img7 = img[360+80-1:360+80+215-1,30-1:30+140-1,:]
           # img8 = img[360+80-1:360+80+260-1,170-1:170+300-1,:]
           # img9 = img[360+80-1:360+80+230-1,470-1:470+150-1,:]
           #
           # img10 = img[360+170-1:360+170+110-1,640+30-1:640+30+145-1,:]
           # img11 = img[360+170-1:360+170+175-1,640+175-1:640+175+290-1,:]
           # img12 = img[360+170-1:360+170+140-1,640+465-1:640+465+145-1,:]

            # NSH

           # img1 = img[60-1:60+220-1,30-1:30+140-1,:]
           # img2 = img[60-1:60+290-1,170-1:170+300-1,:]
           # img3 = img[60-1:60+255-1,470-1:470+150-1,:]

           # img4 = img[60-1:60+185-1,640+50-1:640+50+125-1,:]
           # img5 = img[60-1:60+285-1,640+175-1:640+175+290-1,:]
           # img6 = img[60-1:60+190-1,640+465-1:640+465+145-1,:]

           # img7 = img[360+30-1:360+30+265-1,50-1:50+120-1,:]
           # img8 = img[360+30-1:360+30+310-1,170-1:170+300-1,:]
           # img9 = img[360+30-1:360+30+255-1,470-1:470+150-1,:]

           # img10 = img[360+20-1:360+20+210-1,640+40-1:640+40+135-1,:]
           # img11 = img[360+20-1:360+20+315-1,640+175-1:640+175+290-1,:]
           # img12 = img[360+20-1:360+20+245-1,640+465-1:640+465+145-1,:]
            # Forbes
            # img1 = img[60-1:60+240-1,30-1:30+140-1,:]
            # img2 = img[60-1:60+290-1,170-1:170+300-1,:]
            # img3 = img[60-1:60+240-1,470-1:470+150-1,:]
            #
            # img4 = img[60-1:60+155-1,640+30-1:640+30+145-1,:]
            # img5 = img[60-1:60+265-1,640+175-1:640+175+290-1,:]
            # img6 = img[60-1:60+175-1,640+465-1:640+465+145-1,:]
            #
            # img7 = img[360+80-1:360+80+215-1,60-1:60+110-1,:]
            # img8 = img[360+80-1:360+80+260-1,170-1:170+300-1,:]
            # img9 = img[360+80-1:360+80+260-1,470-1:470+150-1,:]
            #
            # img10 = img[360+40-1:360+40+190-1,640+30-1:640+30+145-1,:]
            # img11 = img[360+40-1:360+40+285-1,640+175-1:640+175+290-1,:]
            # img12 = img[360+40-1:360+40+235-1,640+465-1:640+465+145-1,:]


            #img1 = img[roi_1_px_1-1:roi_1_px_1-1+roi_1_height_1,roi_1_py_1-1:roi_1_py_1-1+roi_1_width_1,:]
            #img2 = img[roi_2_px_1-1:roi_2_px_1-1+roi_2_height_1,roi_2_py_1-1:roi_2_py_1-1+roi_2_width_1,:]
            #img3 = img[roi_3_px_1-1:roi_3_px_1-1+roi_1_height_1,roi_3_py_1-1:roi_3_py_1-1+roi_1_width_1,:]
            #img4 = img[roi_1_px_2-1:roi_1_px_2-1+roi_1_height_2,roi_1_py_2-1+y_off:roi_1_py_2-1+y_off+roi_1_width_2,:]
            #img5 = img[roi_2_px_2-1:roi_2_px_2-1+roi_2_height_2,roi_2_py_2-1+y_off:roi_2_py_2-1+y_off+roi_2_width_2,:]
            #img6 = img[roi_3_px_2-1:roi_3_px_2-1+roi_1_height_2,roi_3_py_2-1+y_off:roi_3_py_2-1+y_off+roi_1_width_2,:]
            #img7 = img[roi_1_px_1-1+x_off:roi_1_px_1-1+x_off+roi_1_height_1,roi_1_py_1-1:roi_1_py_1-1+roi_1_width_1,:]
            #img8 = img[roi_2_px_1-1+x_off:roi_2_px_1-1+x_off+roi_2_height_1,roi_2_py_1-1:roi_2_py_1-1+roi_2_width_1,:]
            #img9 = img[roi_3_px_1-1+x_off:roi_3_px_1-1+x_off+roi_1_height_1,roi_3_py_1-1:roi_3_py_1-1+roi_1_width_1,:]
            #img10 = img[roi_1_px_2-1+x_off:roi_1_px_2-1+x_off+roi_1_height_2,roi_1_py_2-1+y_off:roi_1_py_2-1+y_off+roi_1_width_2,:]
            #img11 = img[roi_2_px_2-1+x_off:roi_2_px_2-1+x_off+roi_2_height_2,roi_2_py_2-1+y_off:roi_2_py_2-1+y_off+roi_2_width_2,:]
            #img12 = img[roi_3_px_2-1+x_off:roi_3_px_2-1+x_off+roi_1_height_2,roi_3_py_2-1+y_off:roi_3_py_2-1+y_off+roi_1_width_2,:]
                # imwrite('tmp.jpg',img1)


            # im[0,:,:,:] = img1[0:224,0:224,:]
            # im[1,:,:,:] = img2[0:224,0:224,:]
            # im[2,:,:,:] = img3[0:224,0:224,:]
            # im[3,:,:,:] = img4[0:224,0:224,:]
            # im[4,:,:,:] = img5[0:224,0:224,:]
            # im[5,:,:,:] = img6[0:224,0:224,:]
            # im[6,:,:,:] = img7[0:224,0:224,:]
            # im[7,:,:,:] = img8[0:224,0:224,:]
            # im[8,:,:,:] = img9[0:224,0:224,:]
            # im[9,:,:,:] = img10[0:224,0:224,:]
            # im[10,:,:,:] = img11[0:224,0:224,:]
            # im[11,:,:,:] = img12[0:224,0:224,:]

            num = data[6:23+6].decode()
            num = np.fromstring(num,dtype=int,sep=' ')
            nnn = np.sum(num)
            if nnn == 0:
                iii = iii+1
                rr = "0000"
                conn.sendall(str.encode(rr))
                time.sleep(0.1)
                conn.sendall(str.encode(rr))
                continue
            iim=np.zeros((nnn,im_size,im_size,3))
            cc = 0
            for ii,nn in enumerate(num):
                if nn == 1:
                    print(ii)
                    iim[cc,:,:,:] = im[ii,:,:,:]
                    cc = cc+1

            predictions = sess.run(softmax_tensor,
                                         {'input:0': np.divide(iim,255.0)})
            predictions = np.squeeze(predictions)

            print(predictions)
            if nnn > 1:
                result = np.array2string(predictions[:,1],formatter={'float_kind':lambda x: "%.2f" % x})
            else:
                result = np.array2string(predictions[1],formatter={'float_kind':lambda x: "%.2f" % x})
                result = '['+result+']'
            rr = "Result:"+result
            print(rr)
            conn.sendall(str.encode(rr))
            if nnn > 1:
                result = np.array2string(predictions[:,2],formatter={'float_kind':lambda x: "%.2f" % x})
            else:
                result = np.array2string(predictions[2],formatter={'float_kind':lambda x: "%.2f" % x})
                result = '['+result+']'
            rr = "Result:"+result
            print(rr)
            conn.sendall(str.encode(rr))
            iii = iii+1
            image_path = TEST_IMAGE_PATHS[iii]
            img = imread(image_path)

            img1 = img[60-1:60+240-1,30-1:30+140-1,:]
            img2 = img[60-1:60+290-1,170-1:170+300-1,:]
            img3 = img[60-1:60+240-1,470-1:470+150-1,:]
            
            img4 = img[60-1:60+155-1,640+30-1:640+30+145-1,:]
            img5 = img[60-1:60+265-1,640+175-1:640+175+290-1,:]
            img6 = img[60-1:60+175-1,640+465-1:640+465+145-1,:]
            
            img7 = img[360+80-1:360+80+215-1,60-1:60+110-1,:]
            img8 = img[360+80-1:360+80+260-1,170-1:170+300-1,:]
            img9 = img[360+80-1:360+80+260-1,470-1:470+150-1,:]
            
            img10 = img[360+40-1:360+40+190-1,640+30-1:640+30+145-1,:]
            img11 = img[360+40-1:360+40+285-1,640+175-1:640+175+290-1,:]
            img12 = img[360+40-1:360+40+235-1,640+465-1:640+465+145-1,:]
            im = np.zeros((12,im_size,im_size,3))
            im[0,:,:,:] = imresize(img1,(im_size,im_size))
            im[1,:,:,:] = imresize(img2,(im_size,im_size))
            im[2,:,:,:] = imresize(img3,(im_size,im_size))
            im[3,:,:,:] = imresize(img4,(im_size,im_size))
            im[4,:,:,:] = imresize(img5,(im_size,im_size))
            im[5,:,:,:] = imresize(img6,(im_size,im_size))
            im[6,:,:,:] = imresize(img7,(im_size,im_size))
            im[7,:,:,:] = imresize(img8,(im_size,im_size))
            im[8,:,:,:] = imresize(img9,(im_size,im_size))
            im[9,:,:,:] = imresize(img10,(im_size,im_size))
            im[10,:,:,:] = imresize(img11,(im_size,im_size))
            im[11,:,:,:] = imresize(img12,(im_size,im_size))



        # f.write("%f\t" % predictions[1])
        # img2 = img[roi_2_px_1-1:roi_2_px_1-1+roi_2_height_1,roi_2_py_1-1:roi_2_py_1-1+roi_2_width_1,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img2})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        # img3 = img[roi_3_px_1-1:roi_3_px_1-1+roi_1_height_1,roi_3_py_1-1:roi_3_py_1-1+roi_1_width_1,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img3})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        #
        # # right above
        # img4 = img[roi_1_px_2-1:roi_1_px_2-1+roi_1_height_2,roi_1_py_2-1+y_off:roi_1_py_2-1+y_off+roi_1_width_2,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img4})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        # img5 = img[roi_2_px_2-1:roi_2_px_2-1+roi_2_height_2,roi_2_py_2-1+y_off:roi_2_py_2-1+y_off+roi_2_width_2,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img5})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        # img6 = img[roi_3_px_2-1:roi_3_px_2-1+roi_1_height_2,roi_3_py_2-1+y_off:roi_3_py_2-1+y_off+roi_1_width_2,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img6})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        #
        # #left bottom
        # img7 = img[roi_1_px_1-1+x_off:roi_1_px_1-1+x_off+roi_1_height_1,roi_1_py_1-1:roi_1_py_1-1+roi_1_width_1,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img7})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        # img8 = img[roi_2_px_1-1+x_off:roi_2_px_1-1+x_off+roi_2_height_1,roi_2_py_1-1:roi_2_py_1-1+roi_2_width_1,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img8})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        # img9 = img[roi_3_px_1-1+x_off:roi_3_px_1-1+x_off+roi_1_height_1,roi_3_py_1-1:roi_3_py_1-1+roi_1_width_1,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img9})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        #
        # img10 = img[roi_1_px_2-1+x_off:roi_1_px_2-1+x_off+roi_1_height_2,roi_1_py_2-1+y_off:roi_1_py_2-1+y_off+roi_1_width_2,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img10})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        # img11 = img[roi_2_px_2-1+x_off:roi_2_px_2-1+x_off+roi_2_height_2,roi_2_py_2-1+y_off:roi_2_py_2-1+y_off+roi_2_width_2,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img11})
        # predictions = np.squeeze(predictions)
        # f.write("%f\t" % predictions[1])
        # img12 = img[roi_3_px_2-1+x_off:roi_3_px_2-1+x_off+roi_1_height_2,roi_3_py_2-1+y_off:roi_3_py_2-1+y_off+roi_1_width_2,:]
        #
        # predictions = sess.run(softmax_tensor,
        #                    {'input:0': img12})
        # predictions = np.squeeze(predictions)
        # f.write("%f\n" % predictions[1])

    # Creates node ID --> English string lookup.
    #top_k = predictions.argsort()[-FLAGS.num_top_predictions:][::-1]
    #for node_id in top_k:
    #  human_string = node_lookup.id_to_string(node_id)
    #  score = predictions[node_id]
      #print('%s (score = %.5f)' % (human_string, score))


def main(_):
  image = (FLAGS.image_file if FLAGS.image_file else
           os.path.join(FLAGS.model_dir, 'cropped_panda.jpg'))
  run_inference_on_image(image)



if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  # classify_image_graph_def.pb:
  #   Binary representation of the GraphDef protocol buffer.
  # imagenet_synset_to_human_label_map.txt:
  #   Map from synset ID to a human readable string.
  # imagenet_2012_challenge_label_map_proto.pbtxt:
  #   Text representation of a protocol buffer mapping a label to synset ID.
  parser.add_argument(
      '--model_dir',
      type=str,
      default= current_dir_path + '/inception',
      help="""\
      Path to classify_image_graph_def.pb,
      imagenet_synset_to_human_label_map.txt, and
      imagenet_2012_challenge_label_map_proto.pbtxt.\
      """
  )
  parser.add_argument(
      '--image_file',
      type=str,
      default='',
      help='Absolute path to image file.'
  )
  parser.add_argument(
      '--num_top_predictions',
      type=int,
      default=1,
      help='Display this many predictions.'
  )
  FLAGS, unparsed = parser.parse_known_args()
  tf.app.run(main=main, argv=[sys.argv[0]] + unparsed)
