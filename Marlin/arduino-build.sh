arroot=$(dirname `which arduino`)
buildpath=`pwd`/build
rm -r $buildpath 2> /dev/null
mkdir $buildpath

COMPILE="F"

while test $# -gt 0
do
    case "$1" in
        -g) COMPILE="T"
            ;;
        --bcn3d-dimensions) echo "BCN3D dimensions"
            FLAGS="$FLAGS -DBCN3D_DIMENSIONS"
            ;;
        --*) echo "bad option $1"
            ;;
        *) echo "argument $1"
            ;;
    esac
    shift
done

FLAGS=""
if [[ "@" == "--bcn3d-dimensions" ]]
then
    echo "BCN3D dimensions"
    FLAGS="$FLAGS -DBCN3D_DIMENSIONS"
fi

if [[ "$COMPILE" == "T" ]]
then
  echo "compile for debug optimization"
  arduino-builder -compile -tools $arroot/tools-builder/ \
   -tools $arroot/hardware/tools -hardware $arroot/hardware \
   -fqbn arduino:avr:mega:cpu=atmega2560 -build-path $buildpath \
   -prefs "compiler.c.elf.flags={compiler.warning_flags} -Og -g -flto -fuse-linker-plugin -Wl,--gc-sections -DDEBUG_COMPRESS" \
   -prefs "compiler.c.flags=-c -g -Og {compiler.warning_flags} -std=gnu11 -ffunction-sections -fdata-sections -MMD -flto -fno-fat-lto-objects -DDEBUG_COMPRESS" \
   -prefs "compiler.cpp.flags=-c -g -Og {compiler.warning_flags} -std=gnu++11 -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -flto -DDEBUG_COMPRESS $FLAGS" \
    *.ino
else
    echo "standard compile"
    arduino-builder -compile -tools $arroot/tools-builder/ \
     -tools $arroot/hardware/tools -hardware $arroot/hardware \
     -fqbn arduino:avr:mega:cpu=atmega2560 -build-path $buildpath \
      *.ino    
fi
