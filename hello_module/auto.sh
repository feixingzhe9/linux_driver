#!/bin/bash

echo "kaka" | sudo -S sh -c "rmmod hellomodule"

#sudo -S sh -c "insmod hellomodule.ko"
sudo insmod hellomodule.ko
sudo chmod 777 /dev/hello


