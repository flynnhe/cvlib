#!/bin/bash
for i in $(find . -regextype posix-egrep -regex ".*/[0-9]{10}")
do
  echo "Entering $i/ir"
  cwd=$PWD
  cd $i/ir;
  ffmpeg -f image2 -r 15 -sameq -s 574x484 -vcodec copy -i "%06d.pgm" video.avi
  cd $cwd;
  echo "Entering $i/eo"
  cd $i/eo;
  ffmpeg -f image2 -r 15 -sameq -s 640x480 -vcodec copy -i "%06d.pgm" video.avi
  cd $cwd;
done
