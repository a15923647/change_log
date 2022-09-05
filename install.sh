#!/bin/bash
set -x
apt update
sudo apt install -y make g++ nlohmann-json3-dev sqlite3 libsqlite3-dev openssl libssl-dev
ls cl-cli cl-daemon 2> /dev/null || ./compile.sh
service change_log stop
cp cl-daemon /usr/bin/
mkdir /etc/change_log
cp ${1:-config.json} /etc/change_log/config.json
cp change_log.service /etc/systemd/system/
systemctl daemon-reload
# start script on boot
systemctl enable change_log.service
mv cl-cli /usr/bin/
service change_log restart
set +x
