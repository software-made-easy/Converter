cd "${0%/*}"
# cd ../../build-Converter-Desktop-Release/
sudo cmake --install ../../build-Converter-Desktop-Release/ --prefix ~/qtprojegt/Converter/packaging/Converter/usr/
# mkdir -p Converter/usr/bin
# cp ../../build-Converter-Desktop-Release/converter Converter/usr/bin/smeconverter
# chmod +x Converter/usr/bin/smeconverter
pwd
dpkg-deb --build Converter
sudo dpkg -i Converter.deb
