cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd
mkdir -p Converter/usr/bin
cp ../../build-Converter-Desktop-Release/converter Converter/usr/bin/smeconverter
chmod +x Converter/usr/bin/smeconverter
dpkg-deb --build Converter
sudo dpkg -i Converter.deb
