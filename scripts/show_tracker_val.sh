#!/bin/bash

if [ -z "$2" ]
  then
    echo "No folder supplied!"
    echo "Usage: bash `basename "$0"` alov_video_folder alov_annotations_folder"
    exit
fi

# Choose which GPU the tracker runs on
GPU_ID=0

# Choose which video from the test set to start displaying
START_VIDEO_NUM=0

# Set to 0 to pause after each frame
PAUSE_VAL=1

VIDEOS_FOLDER=$1
ANNOTATIONS_FOLDER=$2

DEPLOY=nets/tracker.prototxt
CAFFE_MODEL=nets/models/pretrained_model/tracker.caffemodel

build/show_tracker_alov $DEPLOY $CAFFE_MODEL $VIDEOS_FOLDER $ANNOTATIONS_FOLDER $GPU_ID $START_VIDEO_NUM $PAUSE_VAL
