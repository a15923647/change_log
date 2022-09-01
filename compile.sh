#!/bin/bash
set -x
cd src/daemon &&\
make && \
mv runrun ../../cl-daemon
cd ../cli && \
make && \
mv runrun ../../cl-cli
set +x
