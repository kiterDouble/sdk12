#!/usr/bin/bash

make distclean
make PREFIX=${INSTALL_PREFIX} install
make distclean

