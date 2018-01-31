#!/bin/bash
# Update teensy code for the propolis project

gitToken=$GIT_KEY
sketch=$1

rm -r src/*

if [ $1 = "hexgroup" ]; then
	curl -H "Authorization: token $gitToken" -o src/AlveoleArduino.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/AlveoleArduino.ino
	echo "script AlveoleArduino has been update"

elif [ $1 = "atomgroup" ]; then
	curl -H "Authorization: token $gitToken" -o src/AlveoleArduino.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/molecules/AlveoleArduino.ino
	echo "script MoleculeArduino has been update"

else
	curl -H "Authorization: token $gitToken" -o src/$sketch.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/sketchTest/$sketch/$sketch.ino
	echo "script $sketch has been update"
fi

rm -rf src/__MACOSX

cd /home/pi/Documents/teensyUpdate
platformio run -t upload

echo "project has been compiled"