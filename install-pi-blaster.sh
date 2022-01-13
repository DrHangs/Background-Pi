#!/bin/bash
sudo apt-get install autoconf -y
git clone https://github.com/sarfata/pi-blaster.git
cd pi-blaster
./autogen.sh
./configure
make
sudo make install
sudo systemctl disable pi-blaster.service
sudo systemctl stop pi-blaster.service