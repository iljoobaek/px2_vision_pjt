""" The file defines object detector. """
import functools
import time
import sys
import os
dirname = os.path.dirname(__file__)
slim_path = os.path.join(dirname, './slim/')
sys.path.append(slim_path)

#os.environ['CUDA_VISIBLE_DEVICES'] = '-1'


import cv2
import tensorflow as tf
import numpy as np
import ctypes
import gc
#_test = ctypes.CDLL('./server.so')
#_test.getImgBuffer.argtypes = (ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int, ctypes.c_int)

#global _test

from google.protobuf import text_format
from object_detection.builders import model_builder
from object_detection.protos import pipeline_pb2
from object_detection.utils import label_map_util
from utils import np_methods
from utils import visualization
from utils import hot_key

'''ADDITIONS FOR IMG_CAP'''
_test = ctypes.CDLL('./both_1_cam.so')
_test.eglconsumer_main.argtypes = ()
_test.getImgBuffer.argtype = (ctypes.POINTER(ctypes.c_ubyte))

_test.eglconsumer_main()

time.sleep(1)

DEFAULT_NETWORK_INPUT_SIZE=300

class ObjectDetector(object):
    """ A class of DNN object detector. """
    def __init__(self, checkpoint_path, pipeline_config_path, label_map_path,
                 network_size=None, gpu_memusage=-1):
        """
        Args:
            checkpoint_path: a string containing the path to
                the checkpoint files.
            pipeline_config_path: a string containing the path to
                the configuration file.
            label_path: a string containing path to the label map.
            network_size: the size of the network input,
                1D int32 [height, width]. The minimum input size is
                DEFAULT_NETWORK_INPUT_SIZE. If set to None, the network_size
                will auto adjust to the input image size.
            gpu_memusage: the percentage of the gpu memory to use, valid value
                should be (0,1]. It is set dynamically if set a negative value.
        """
        self._checkpoint_path = checkpoint_path
        self._pipeline_config_path = pipeline_config_path
        self._label_map_path = label_map_path
        self._network_size = network_size
        # =====================================================================
        # Configure model
        # =====================================================================
        model_config = self._get_configs_from_pipeline_file(
            self._pipeline_config_path)
        model_fn = functools.partial(
              model_builder.build,
              model_config=model_config,
              is_training=False)
        # instantiate model architecture
        model = model_fn()
        # replace the default non-max function with a dummy
        model._non_max_suppression_fn = self._dummy_nms
        # get the labels
        label_map = label_map_util.load_labelmap(self._label_map_path)
        max_num_classes = max([item.id for item in label_map.item])
        categories = label_map_util.convert_label_map_to_categories(
            label_map, max_num_classes=max_num_classes, use_display_name=True)
        self._category_index = label_map_util.create_category_index(categories)
        # =====================================================================
        # Construct inference graph
        # =====================================================================
        self._image = tf.placeholder(dtype=tf.float32, shape=(None,None,3))
        # opencv image is BGR by default, reverse its channels
        image_RGB = tf.reverse(self._image, axis=[-1])
        image_4D = tf.stack([image_RGB], axis=0)
        # resize image to fit network size
        if self._network_size:
            input_size = tf.cast(self._network_size, tf.float32)
            input_height = input_size[0]
            input_width = input_size[1]
        else:
            # Auto adjust network size for the image
            image_shape = tf.cast(tf.shape(self._image), tf.float32)
            input_height = image_shape[0]
            input_width =  image_shape[1]
        min_input_size = tf.minimum(input_width, input_height)
        min_input_size = tf.cast(min_input_size, tf.float32)
        scale = DEFAULT_NETWORK_INPUT_SIZE / min_input_size
        resized_image_height = tf.cond(scale > 1.0,
            lambda: scale*input_height, lambda: input_height)
        resized_image_width = tf.cond(scale > 1.0,
            lambda: scale*input_width, lambda: input_width)
        self._resized_image_size = tf.stack([resized_image_height,
                                             resized_image_width],
                                             axis=0)
        self._resized_image_size = tf.cast(self._resized_image_size,
                                           tf.int32)
        # replace resizer and anchor generator for ROI
        model._image_resizer_fn = functools.partial(self._resizer_fn,
            new_image_size=self._resized_image_size)

        model._anchor_generator._base_anchor_size = [tf.minimum(scale, 1.0),
                                                     tf.minimum(scale, 1.0)]
        preprocessed_image, true_image_shapes = model.preprocess(
            tf.to_float(image_4D))
        prediction_dict = model.predict(preprocessed_image, true_image_shapes)
        self._detection_dict = model.postprocess(prediction_dict, true_image_shapes)
        # =====================================================================
        # Fire up session
        # =====================================================================
        saver = tf.train.Saver()
        if gpu_memusage < 0:
            gpu_options = tf.GPUOptions(allow_growth=True)
        else:
            gpu_options = tf.GPUOptions(
                per_process_gpu_memory_fraction=gpu_memusage)
        self._sess = tf.Session(config=tf.ConfigProto(gpu_options=gpu_options))
        saver.restore(self._sess, self._checkpoint_path)

    def __del__(self):
        """ Destructor"""
        self._sess.close()

    def detect(self, image):
        """ Detect objects in image.
        Args:
            image: a uint8 [height, width, depth] array representing the image.

        """
        rbboxes, rscores, rclasses = self._sess.run(
            [self._detection_dict['detection_boxes'],
             self._detection_dict['detection_scores'],
             self._detection_dict['detection_classes']],
            feed_dict={self._image:image})
        # numpy NMS method
        rbboxes, rscores, rclasses = self._np_nms(rbboxes,rscores)
        #visualization
        visualization.plt_bboxes(
            image,
            rclasses+1,
            rscores,
            rbboxes,
            self._category_index,
            Threshold=None
        )
        return image

    def _dummy_nms(self, detection_boxes, detection_scores, additional_fields=None,
                   clip_window=None):
        """ A dummy function to replace the tensorflow non_max_suppression_fn.
        We use a numpy version for a more flexiable post_processing. """
        return (detection_boxes, detection_scores, detection_scores,
                detection_scores, additional_fields, detection_scores)

    def _resizer_fn(self, image, new_image_size):
        """ Image resizer. """
        with tf.device('/gpu:0'):
            image = tf.image.resize_images(image, new_image_size)
        return [image, tf.concat([new_image_size, tf.constant([3])], axis=0)]

    def _np_nms(
        self,
        detection_boxes,
        detection_scores,
        clip_window=(0,0,1.0,1.0),
        # select_threshold=(
        #                     0.7,    #Car
        #                     0.8,    #Van
        #                     0.5,    #Truck
        #                     0.5,    #Cyclist
        #                     0.6,    #Pedestrian
        #                     0.5,    #Person_sitting
        #                     0.5,    #Tram
        #                     0.5,    #Misc
        #                     0.5,    #Sign
        #                     0.5,    #Traffic_cone
        #                     0.5,    #Traffic_light
        #                     0.5,    #School_bus
        #                     0.75,   #Bus
        #                     0.5,    #Barrier
        #                     0.5,    #Bike
        #                     0.5,    #Moter
        #                     ),
        select_threshold=(0.5,)*16,
        nms_threshold=0.1):
        # Get classes and bboxes from the net outputs.
        rclasses, rscores, rbboxes = np_methods.ssd_bboxes_select_layer(
                detection_scores,
                detection_boxes,
                anchors_layer=None,
                select_threshold=select_threshold,
                img_shape=None,
                num_classes=None,
                decode=False,
                label_offset=0)
        rbboxes = np_methods.bboxes_clip(clip_window, rbboxes)
        rclasses, rscores, rbboxes = np_methods.bboxes_sort(
            rclasses, rscores, rbboxes, top_k=400)
        rclasses, rscores, rbboxes = np_methods.bboxes_nms(
            rclasses, rscores, rbboxes, nms_threshold=nms_threshold)
        # Resize bboxes to original image shape. Note: useless for Resize.WARP!
        rbboxes = np_methods.bboxes_resize(clip_window, rbboxes)
        return rbboxes, rscores, rclasses

    def _get_configs_from_pipeline_file(self, pipeline_config_path):
        """Reads model configuration from a pipeline_pb2.TrainEvalPipelineConfig.
        Reads model config from file specified by pipeline_config_path.
        Returns:
        model_config: a model_pb2.DetectionModel
        """
        pipeline_config = pipeline_pb2.TrainEvalPipelineConfig()
        with tf.gfile.GFile(pipeline_config_path, 'r') as f:
            text_format.Merge(f.read(), pipeline_config)
            model_config = pipeline_config.model
        return model_config

if __name__ == '__main__':
    checkpoint_path = os.path.join(dirname,
        './train_kitti_mot_lisa_bdd_distort_color_focal_loss_300k/model.ckpt-530316')
    pipeline_config_path = os.path.join(dirname,
        './train_kitti_mot_lisa_bdd_distort_color_focal_loss_300k/ssd_mobilenet_v1_kitti_mot_lisa_bdd_distort_color.config')
    label_map_path = os.path.join(dirname,
        './train_kitti_mot_lisa_bdd_distort_color_focal_loss_300k/kitti_mot_bdd100k_lisaExtended2Coco_(train).tfrecord.pbtxt')
    # detector = ObjectDetector(checkpoint_path, pipeline_config_path, label_map_path, network_size = [300, 300])
    detector = ObjectDetector(checkpoint_path, pipeline_config_path, label_map_path,
        network_size = [600, 600])
        #network_size = [300, 1280])
    video_path=os.path.join(dirname,
        './videos/test11.avi_0000000000.avi')
    cap = cv2.VideoCapture(video_path)
    frame_ctr = 0
    time_start = time.time()
   

    #img_buf = (4746240 * ctypes.c_ushort) ()
    img_buf = ((2319360) * ctypes.c_ubyte) ()
    
    sum_time = 0
    num_f = 0
    max_time = 0
    frame_id = 0
    
    while(True):
        ###START TIMING###
        time_0 = time.time()
        ###START TIMING###
        #_, img = cap.read()
        #if img is None:
        #    break
        num_f = num_f + 1
        #gc.collect()
        frame_id = frame_id + 1
        
        result = _test.getImgBuffer(ctypes.cast(img_buf, ctypes.POINTER(ctypes.c_ubyte)))
        #print(type(img_buf))

        nd_arr = np.ctypeslib.as_array(img_buf)
        nd_arr_rs = np.reshape(nd_arr, (604, 960, 4))
        #img = np.reshape(nd_arr, (604, 960, 4))
        
        #print nd_arr_rs.shape

        img = cv2.cvtColor(nd_arr_rs, cv2.COLOR_RGBA2BGR);
        
        '''result = _test.getImgBuffer(ctypes.cast(img_buf, ctypes.POINTER(ctypes.c_ubyte)), ctypes.c_int(3840), ctypes.c_int(1208))

        nd_arr = np.ctypeslib.as_array(img_buf)
        nd_arr_rs = np.reshape(nd_arr, (1236,3840,1))

        bayer_img = cv2.convertScaleAbs(nd_arr_rs, alpha=(0.015625))
        rgb_img = cv2.cvtColor(bayer_img, cv2.COLOR_BAYER_GB2BGR)
        img = cv2.resize(rgb_img, (1280,360))
        #rgb_img_scaled = cv2.resize(rgb_img, (1280,360))

        #cv2.imshow('IMG', img)
        #cv2.waitKey(0)
        
        #top_img = rgb_img_scaled[0:360, 0:1280]
        #bot_img = rgb_img_scaled[0:360, 1280:2560]
        #img = np.vstack((top_img, bot_img))
        '''
        #crop ROI
        '''crop_height = 300
        crop_width = 1280
        crop_ymin = 300
        crop_xmin = 0
        crop_ymax = crop_ymin+crop_height
        crop_xmax = crop_xmin+crop_width
        img_crop = img[crop_ymin:crop_ymax,
                       crop_xmin:crop_xmax]'''
        # draw ROI
        #cv2.rectangle(img, (crop_xmin, crop_ymin), (crop_xmax, crop_ymax),
        #    (0,0,255), 2)
        #cv2.putText(img, '{:s}'.format('ROI normal (300)'),
        #    (crop_xmin, crop_ymin+30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,255),2)
        #detector.detect(img_crop)
        detector.detect(img)
        cv2.imshow('Output',img)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break

        #timing
        frame_ctr = frame_ctr + 1
        if frame_ctr == 10:
            time_now = time.time()
            fps = frame_ctr/(time_now-time_start)
            print("fps: %f"%fps)
            frame_ctr = 0
            time_start = time_now
        ###END TIMING###
        time_f = time.time()
        diff_time = time_f - time_0
        if frame_id > 20:
            if diff_time > max_time:
                max_time = diff_time
            sum_time = sum_time + diff_time
            num_f = num_f + 1
        ###END TIMING###

    avg_time = (sum_time / num_f) * 1000
    print("%f "%sum_time)
    print("%d "%num_f)
    print("%f "%avg_time)
