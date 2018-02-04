#!/bin/bash
# Update teensy code for the propolis project

gitToken=$GIT_KEY
defaultValue=$MODULE_TYPE
echo "defaultValue"
sketch=$1

rm -r src/*

if [ "$1" = "hexgroup" ]; then
	curl -H "Authorization: token $gitToken" -o src/AlveoleArduino.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/AlveoleArduino.ino
	echo "script AlveoleArduino has been update"

elif [ "$1" = "atomgroup" ]; then
	curl -H "Authorization: token $gitToken" -o src/AlveoleArduino.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/molecules/AlveoleArduino.ino
	echo "script MoleculeArduino has been update"

elif [ "$1" = "default" ]; then
	if [ defaultValue = "hexgroup" ]; then
		curl -H "Authorization: token $gitToken" -o src/AlveoleArduino.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/AlveoleArduino.ino
		echo "script AlveoleArduino has been update"
	elif [ defaultValue = "atomgroup" ]; then
		curl -H "Authorization: token $gitToken" -o src/AlveoleArduino.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/molecules/AlveoleArduino.ino
		echo "script MoleculeArduino has been update"
	fi
else
	curl -H "Authorization: token $gitToken" -o src/$sketch.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/sketchTest/$sketch/$sketch.ino
	echo "script $sketch has been update"
fi

rm -rf src/__MACOSX

screen -X -S "propolis" quit

cd /home/pi/Documents/teensyUpdate
sudo platformio run -t upload &&

screen -dmS propolis python3 /home/pi/Desktop/python_server/serialTestVF.py

echo "project has been compiled"

read