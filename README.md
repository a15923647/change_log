# change log
A automatic file tracking system

Currently support for inotify-supported platform.
Use sqlite database to store all changes.

# compile
```shell=
./compile.sh
```
# install
fill in all fields in config.json

run systemd service
```shell=
sudo ./install.sh
```
Alternatively, execute daemon directly
```shell=
sudo mv cl-cli /usr/bin/
./cl-daemon config.json
```
# cli usage
```
cl-cli /path/to/db (list|ll|back) [path_pattern]
cl-cli /path/to/db back path_pattern back_time
```
Value of back_time can be either unix timestamp or negative number representing how many times to patch back.
If your database is fixed, you can add a alias on the command.
```shell=
alias cl='cl-cli /path/to/db'
```
