#!/bin/bash

mkdir -p nets/models/weights_init 
cd nets/models/weights_init
wget http://cs.stanford.edu/people/davheld/public/GOTURN/weights_init/tracker_init.caffemodel
cd ../../../
