#!/bin/bash

#echo "kaka" | sudo -S sh -c "rmmod hellomodule"

#sudo -S sh -c "insmod mpu6050_module.ko"

sudo rmmod mpu6050_module
sudo insmod mpu6050_module.ko
sudo chmod 777 /dev/mpu60500


