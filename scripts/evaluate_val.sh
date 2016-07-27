#!/bin/bash

if [ -z "$1" ]
  then
    echo "No folder supplied!"
    echo "Usage: bash `basename "$0"` alov_video_folder alov_annotations_folder"
    exit
fi

# Choose which GPU the tracker runs on
GPU_ID=0

# Whether to evaluate on the training set or the validation set
USE_TRAIN=0

# Whether or not to save videos of the tracking output
SAVE_VIDEOS=0

VIDEOS_FOLDER=$1
ANNOTATIONS_FOLDER=$2

FOLDER=GOTURN1_val

DEPLOY_PROTO=nets/tracker.prototxt

CAFFE_MODEL=nets/models/pretrained_model/tracker.caffemodel

OUTPUT_FOLDER=nets/tracker_output/$FOLDER

echo "Saving output to " $OUTPUT_FILE

# Run tracker on validation set
build/test_tracker_alov $VIDEOS_FOLDER $ANNOTATIONS_FOLDER $DEPLOY_PROTO $CAFFE_MODEL $OUTPUT_FOLDER $USE_TRAIN $SAVE_VIDEOS $GPU_ID 

# Compute validation score
matlab -nodisplay -r "addpath(genpath('scripts/Fscore_v1.0')); evaluate_all $ANNOTATIONS_FOLDER $OUTPUT_FOLDER; exit"

