#!/bin/bash
# Update teensy dependencies for the propolis project

zLib=$1

echo "asking github for zip file: $zLib"

curl -H 'Authorization: token c8ab6480fbfff506ca2febdcc5e7d05c07ce2912' -o zipLib/$zLib.zip https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/teensyDependencies/$zLib.zip

unzip zipLib/$zLib.zip -d ./lib

echo "$zlib as been unzip and add to the lib folder"

rm -rf lib/__MACOSX