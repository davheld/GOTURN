#!/bin/bash

mkdir -p nets/models/pretrained_model 
cd nets/models/pretrained_model
wget http://cs.stanford.edu/people/davheld/public/GOTURN/trained_model/tracker.caffemodel
cd ../../../
