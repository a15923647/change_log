#!/bin/bash
ls cl-cli cl-daemon 2> /dev/null || ./compile.sh
mv cl-daemon /usr/bin/
mkdir /etc/change_log
mv config.json /etc/change_log/
mv change_log.service /etc/systemd/system/
systemctl daemon-reload
# start script on boot
systemctl enable change_log.service
mv cl-cli /usr/bin/
