#!/bin/bash


VERSION=$(grep libyxml ../platformio.ini | cut -d '=' -f 2 | xargs)


wget https://github.com/JulStrat/yxml/archive/refs/heads/${VERSION}.zip -O yxml.zip

rm -rf yxml
unzip yxml.zip
mv yxml-${VERSION} yxml

rm -rf yxml/bench
rm -rf yxml/test

rm -f yxml.zip


