#!/bin/sh

#
# Script to convert makefile generate from atmel studio 7 (windows)
#  to linux path, assuming that you have arm-none-eabi-* tools in PATH
# 
# based on https://gist.github.com/corsiferrao/329d71d36436cf7311a7
#   by Rafael Corsi (corsiferrao@gmail.com)
# GPL


PROJECT_NAME=$1

if [ "$PROJECT_NAME" != "" ]; then

  #TARGET_DIR=${PROJECT_NAME}/Debug
  #TARGET_DIR=${PROJECT_NAME}
  TARGET_DIR=Debug

  # copy Makefile to Makefile-linux
  cp ${TARGET_DIR}/Makefile ${TARGET_DIR}/Makefile-linux
    
  # remove ref to gcc windows path
  sed -i 's/C:\\.*\\bin\\//g' ${TARGET_DIR}/Makefile-linux

  # remove ref to extension .bin
  sed -i 's/.exe//g' ${TARGET_DIR}/Makefile-linux

  # remove SHELL
  sed -i 's/SHELL := cmd//g' ${TARGET_DIR}/Makefile-linux

else

  echo "Usage: convert_studio7_linux.sh PROJECT_NAME"

fi

