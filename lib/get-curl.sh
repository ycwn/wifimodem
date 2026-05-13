#!/bin/bash


VERSION=$(grep libcurl ../platformio.ini | cut -d '=' -f 2 | xargs)
TARBALL=curl-${VERSION}.tar.xz
LIBDIR=curl-${VERSION}/


rm -rf curl-*
wget https://curl.se/download/${TARBALL}
tar Jxvf ${TARBALL}

rm -rf ${LIBDIR}/src
mv ${LIBDIR}/lib      ${LIBDIR}/src
cp curl_config.h      ${LIBDIR}/src/
cp curl_setup_once.h  ${LIBDIR}/src/

patch -d ${LIBDIR} -Np1 < curl-limit-buffers.patch

rm -f ${TARBALL}


