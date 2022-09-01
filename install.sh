#!/bin/bash
set -x
ls cl-cli cl-daemon 2> /dev/null || ./compile.sh
cp cl-daemon /usr/bin/
mkdir /etc/change_log
cp config.json /etc/change_log/
cp change_log.service /etc/systemd/system/
systemctl daemon-reload
# start script on boot
systemctl enable change_log.service
mv cl-cli /usr/bin/
set +x
