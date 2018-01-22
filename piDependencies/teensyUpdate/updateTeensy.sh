#!/bin/bash
# Update teensy code for the propolis project

sketch=$1

##uncomment to fetch precompile hex file
#curl -H 'Authorization: token 6723862080f55bb07f31dcb5796f200b88fd4b38' -o sourceToUpload/AlveoleArduino.ino.TEENSY31.HEX https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/HEX/AlveoleArduino.ino.TEENSY31.hex

rm -r src/*

if [$1 == ""] 
	then
		curl -H 'Authorization: token 6723862080f55bb07f31dcb5796f200b88fd4b38' -o src/AlveoleArduino.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/AlveoleArduino.ino
		echo "script AlveoleArduino has been update"
else
	curl -H 'Authorization: token 6723862080f55bb07f31dcb5796f200b88fd4b38' -o src/$sketch.ino https://raw.githubusercontent.com/ProjetPropolis/AlveoleArduino/migrationSensor/sketchTest/$sketch/$sketch.ino
	echo "script $sketch has been update"
fi

rm -rf src/__MACOSX

platformio run -t upload

echo "project has been compiled"