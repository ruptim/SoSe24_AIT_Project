#!/bin/sh

make BOARD=feather-nrf52840-sense
stty -F /dev/tty$1 raw ispeed 1200 ospeed 1200 cs8 -cstopb ignpar eol 255 eof 255
echo sleep 3
sleep 3
udisksctl mount -b /dev/sdg 
/home/timon/docs/uni/projects/mi_ait/SoSe24_AIT_Project/RIOT_AIT/dist/tools/uf2/uf2conv.py -f 0xADA52840 /home/timon/docs/uni/projects/mi_ait/SoSe24_AIT_Project/buzzer_system/bin/feather-nrf52840-sense/buzzer_system.hex --base 0x1000
if [ -n "$2" ]
    then
    make BOARD=feather-nrf52840-sense term PORT=/dev/tty$1
fi
