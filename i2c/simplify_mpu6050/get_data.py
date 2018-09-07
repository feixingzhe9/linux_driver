#!/usr/bin/env python
# coding=utf-8


from fcntl import ioctl
import array
import ctypes
import time


buf = array.array('B', [0])

fd = 0
def open_mpu6050():
    fd = open("/dev/mpu60500")
    return fd


fd = open_mpu6050() 
if fd < 0:
    print "can not open /dev/mpu6050 !"
else:
    print "open mpu6050 sucessfully"


def get_accelerometer():
    global fd
    global buf
    #ioctl(fd, 65, test, 1)
    #ioctl(fd, 65, buf, 1)
    #print 'buf ', buf


def get_temperature():
    global fd
    global buf
    ioctl(fd, 65, buf, 1)
    temperature_h = buf[0]
    temperature_h = ct = ctypes.c_int8(temperature_h).value
    #print "temp high: ", temperature_h

    ioctl(fd, 66, buf, 1)
    temperature_l = buf[0]
    #print "temp low: ", temperature_l

    temperature = ctypes.c_int16(temperature_l).value + ctypes.c_int16(temperature_h<<8).value
    #print "temperature : ", temperature

    #temp = ct = ctypes.c_short(temperature).value
    temp =  ctypes.c_short(temperature).value

    #print "temp : ", temp
    temp = ctypes.c_float(temp).value/340 + ctypes.c_float(36.53).value
    print "true temp : ", temp

#get_accelerometer()

while 1:
    get_temperature()
    time.sleep(1)

