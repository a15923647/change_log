#!/bin/bash
set -x
cd src/daemon &&\
make clean
make && \
mv runrun ../../cl-daemon
cd ../cli && \
make clean
make && \
mv runrun ../../cl-cli
set +x
