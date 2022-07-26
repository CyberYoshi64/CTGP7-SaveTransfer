#!/bin/sh
clear
echo "+----------------------------------------------------------+"
echo "|                CY64's GUI building script                |"
echo "+----------------------------------------------------------+"
rm -r -f output/
echo ""
echo "\$> make $1 && make"
make $1 && make
echo "\$> make cia"
make cia
echo ""
echo "Script has finished."
echo ""
echo "Either use 3dslink or copy the contents of the output folder to the SD card of the 3DS."
