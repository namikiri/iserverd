#!/bin/sh

cd ../../../
make bin/iserverd

cd /usr/local/iserverd
killall iserverd
sleep 2
cp /home/project/IServerd/IServerd/bin/iserverd /usr/local/iserverd/bin/iserverd
./bin/iserverd -o
