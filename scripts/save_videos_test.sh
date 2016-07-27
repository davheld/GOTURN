#!/bin/bash

if [ -z "$1" ]
  then
    echo "No folder supplied!"
    echo "Usage: bash `basename "$0"` vot_videos_folder"
    exit
fi

# Choose which GPU the tracker runs on
GPU_ID=0

VIDEOS_FOLDER=$1

FOLDER=GOTURN1_test

DEPLOY_PROTO=nets/tracker.prototxt

CAFFE_MODEL=nets/models/pretrained_model/tracker.caffemodel

OUTPUT_FOLDER=nets/tracker_output/$FOLDER

echo "Saving output to " $OUTPUT_FILE

# Run tracker on test set and save vidoes 
build/save_videos_vot $VIDEOS_FOLDER $DEPLOY_PROTO $CAFFE_MODEL $OUTPUT_FOLDER $GPU_ID 
