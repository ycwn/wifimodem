#!/bin/bash


VERSION=$(grep libssh2 ../platformio.ini | cut -d '=' -f 2 | xargs)
TARBALL=libssh2-${VERSION}.tar.xz
LIBDIR=libssh2-${VERSION}/


rm -rf libssh2-*
wget https://libssh2.org/download/${TARBALL}
tar Jxvf ${TARBALL}

cp libssh2_config.h ${LIBDIR}/src/


rm -f ${TARBALL}


