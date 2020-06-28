#!/usr/bin/env python

# publishes: 
# ???
# battery_state
#
# subscribes:
# cmd_vel -> sends to 32U4 control board

import rospy

from romi32U4_driver import Romi32U4Driver

from std_msgs.msg import String
from geometry_msgs.msg import Twist
from sensor_msgs.msg import BatteryState

class Brainstem():
    def __init__(self):
        self.romi = Romi32U4Driver()

        rospy.init_node('brainstem_node', anonymous=True)

        self.battery_pub = rospy.Publisher('battery_state', BatteryState,
                queue_size = 2)

    def cmd_vel_callback(self, data):
        linear_x = data.linear.x
        angular_z = data.angular.z
        rospy.loginfo(rospy.get_caller_id() + " forward %s, rotation %s", linear_x, angular_z)

    def publish_battery_state_msg(self, publisher):
        pass

    def read_romi_state(self):
        try:
            self.battery_mv = self.romi.read_battery_millivolts()
        except:
            rospy.logfatal("reading from Romi I2C driver failed.")

    def start(self):
        rospy.Subscriber('cmd_vel', Twist, self.cmd_vel_callback)

        rate = rospy.Rate(10) # 10hz
        while not rospy.is_shutdown():
            # self.read_romi_state()
            # self.publish_battery_state_msg(self.battery_pub)

            rate.sleep()


if __name__ == '__main__':
    try:
        brainstem = Brainstem()
        brainstem.start()
    except rospy.ROSInterruptException:
        pass
