#!/usr/bin/python2
#
# Notes:
# read_unpack, write_pack types quick reference
# https://docs.python.org/3.6/library/struct.html#format-characters
# ? - bool           - 1
# B - unsigned char  - 1
# h - short          - 2
# H - unsigned short - 2
# f - float          - 4
#
# Copyright Pololu Corporation.  For more information, see https://www.pololu.com/

from smbus import SMBus
import struct
import time

class Romi32U4Driver:
    def __init__(self):
        # open I2C port
        self.bus = SMBus(1)

    def close(self):
        self.bus.close()

    def read_raw(self, size):
        try:
            byte_list = [self.bus.read_byte(20) for _ in range(size)]
        except IOError:
            print("IOError Detected: read_raw")
            return None
        return byte_list


    def read_unpack(self, address, size, format):
        # Ideally we could do this:
        #    byte_list = self.bus.read_i2c_block_data(20, address, size)
        # But the AVR's TWI module can't handle a quick write->read transition,
        # since the STOP interrupt will occasionally happen after the START
        # condition, and the TWI module is disabled until the interrupt can
        # be processed.
        #
        # A delay of 0.0001 (100 us) after each write is enough to account
        # for the worst-case situation in our example code.
        self.bus.write_byte(20, address)
        time.sleep(0.0002)
        byte_list = [self.bus.read_byte(20) for _ in range(size)]
        return struct.unpack(format, bytes(bytearray(byte_list)))

    def write_pack(self, address, format, *data):
        for i in range(2):
            try:
                data_array = list(struct.pack(format, *data))
                # NOTE: had to hack these strings into ints for some reason
                data_array = map(lambda x: int(ord(x)), data_array)
                self.bus.write_i2c_block_data(20, address, data_array)
            except IOError:
                write_fail_flag = True
                print("IOError Detected: write_pack")
                continue
            break
        time.sleep(0.0001) 

    """
    OUTPUT commands
    """

    def leds(self, red, green, yellow):
        self.write_pack(0, '???', red, green, yellow)

    def _new_twist(self):
        """
        Mark current Twist as a fresh value
        Resets command timeout on robot
        """
        self.write_pack(8, '?', True)

    def twist(self, linear_x_m_s, angular_z_rad_s):
        """
        Sets a new Twist command for the robot to follow
        """
        self.write_pack(9, 'ff', linear_x_m_s, angular_z_rad_s)
        self._new_twist()

    """
    INPUT robot state
    """

    def read_buttons(self):
        return self.read_unpack(3, 3, "???")

    def read_battery_millivolts(self):
        return self.read_unpack(6, 2, "H")[0]

    def read_linear_velocity(self): 
        """
        Linear and velocity of robot
        Hopefully as a result of the current twist command
        """
        return self.read_unpack(17, 4, 'f')
    
    def read_angular_velocity(self): 
        """
        Angular and velocity of robot
        """
        return self.read_unpack(21, 4, 'f')

