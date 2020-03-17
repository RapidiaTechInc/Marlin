cd Marlin/

arroot=$(dirname `which arduino`)
buildpath=`pwd`/build
if [ ! -d $buildpath ]
then
  mkdir $buildpath
fi

arduino-cli upload --fqbn arduino:avr:mega:cpu=atmega2560 --verbose --port /dev/ttyUSB* --verify Marlin.ino
