#!/usr/bin/env python
# coding=utf-8

import sys
from fcntl import ioctl
import array
import ctypes
import time

GRA_ACC = 9.7964    # gravitational acceleration in Shanghai
ioctl_buf = array.array('B', [0])
fd = 0

def open_mpu6050():
    fd = open("/dev/mpu60500")
    return fd

class MPU6050_data(object):
    def __init__(self, _fd):
        self.fd = _fd
        ioctl_buf = array.array('B', [0])

    def build_value_by_reg_addr(self, addr_h, addr_l):
        ioctl_buf[0] = 0

        ioctl(fd, addr_h, ioctl_buf, 1)
        h = ioctl_buf[0]
        ioctl_buf[0] = 0

        ioctl(fd, addr_l, ioctl_buf, 1)
        l = ioctl_buf[0]

        return ctypes.c_int16(l).value + ctypes.c_int16(h<<8).value

    def get_temperature(self):
        temp = self.build_value_by_reg_addr(65, 66)
        temp = ctypes.c_float(temp).value/340 + ctypes.c_float(36.53).value
        return temp

    def get_acc_x_origin(self):
        return self.build_value_by_reg_addr(59, 60)

    def get_acc_y_origin(self):
        return self.build_value_by_reg_addr(61, 62)

    def get_acc_z_origin(self):
        return self.build_value_by_reg_addr(63, 64)

    def get_acc_x(self):
        global GRA_ACC
        return (ctypes.c_float(self.get_acc_x_origin()).value/16384) * GRA_ACC

    def get_acc_y(self):
        global GRA_ACC
        return (ctypes.c_float(self.get_acc_y_origin()).value/16384) * GRA_ACC

    def get_acc_z(self):
        global GRA_ACC
        return (ctypes.c_float(self.get_acc_z_origin()).value/16384) * GRA_ACC


def main():
    global fd
    fd = open_mpu6050()
    if fd < 0:
        print "can not open /dev/mpu6050 !"
    else:
        print "open mpu6050 sucessfully"
    acc = MPU6050_data(fd)
    while 1:
        temperature = acc.get_temperature()
        acc_x = acc.get_acc_x()
        acc_y = acc.get_acc_y()
        acc_z = acc.get_acc_z()
        print "get temperature  : ", temperature
        print "get acc_X        : ",acc_x ," m/s2"
        print "get acc_Y        : ",acc_y ," m/s2"
        print "get acc_Z        : ",acc_z ," m/s2"

        #print "gra_acc :",pow(pow(acc_x, 2) + pow(acc_y, 2) + pow(acc_z, 2), float(1)/2)
        time.sleep(0.3)

if __name__ == '__main__':
    try:
        main()
    except Exception:
        print sys.exc_info()
        exit(1)

