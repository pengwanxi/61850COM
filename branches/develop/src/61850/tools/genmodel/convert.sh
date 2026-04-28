#!/bin/sh

java -jar genconfig.jar model_pxr25.icd > model_PXR25.cfg
java -jar genconfig.jar model_pxr20.icd > model_PXR20.cfg

dest_dir=../../cfg/iec61850/
cp model_PXR25.cfg $dest_dir
cp model_PXR20.cfg $dest_dir
