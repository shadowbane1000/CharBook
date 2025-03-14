#pushd ../../ArduinoIoTCloud/extras/tools
#./lzss.py --encode ../../../CharBook/computer/.pio/build/T5-ePaper-S3/firmware.bin /tmp/firmware.lzss
#./bin2ota.py ESP32 /tmp/firmware.lzss ../../../CharBook/computer/firmware.ota
#rm /tmp/firmware.lzss
#popd
cp .pio/build/T5-ePaper-S3/firmware.bin firmware.ota

