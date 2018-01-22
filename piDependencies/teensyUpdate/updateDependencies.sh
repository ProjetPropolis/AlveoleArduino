#!/bin/bash
# Update teensy dependencies for the propolis project

zLib=$1 

echo "asking github for zip file: $zLib"

curl -H 'Authorization: token 6723862080f55bb07f31dcb5796f200b88fd4b38' -o zipLib/$zLib.zip https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/teensyDependencies/$zLib.zip

unzip zipLib/$zLib.zip -d ./lib

echo "$zlib as been unzip and add to the lib folder"

rm -rf lib/__MACOSX