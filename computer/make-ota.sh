#!/usr/bin/env bash

#pushd ../../ArduinoIoTCloud/extras/tools
#./lzss.py --encode ../../../CharBook/computer/.pio/build/T5-ePaper-S3/firmware.bin /tmp/firmware.lzss
#./bin2ota.py ESP32 /tmp/firmware.lzss ../../../CharBook/computer/firmware.ota
#rm /tmp/firmware.lzss
#popd

ver=$1;
echo "Making flash for version $ver"
cp -a sd/${ver} ${ver}
cp .pio/build/T5-ePaper-S3/firmware.bin ${ver}/Flash.bin
gzip ${ver}/Flash.bin
mv ${ver}/Flash.bin.gz ${ver}/Flash.gz
tar -zcf ${ver}.tgz ${ver}
rm -rf ${ver}
