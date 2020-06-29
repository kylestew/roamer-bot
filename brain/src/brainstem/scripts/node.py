#!/usr/bin/env python

# subscribes:
# cmd_vel [Twist]
#
# publishes: 
# odom [Odometry]
# battery_state [BatteryState]

import rospy

from romi32U4_driver import Romi32U4Driver

from std_msgs.msg import String
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from sensor_msgs.msg import BatteryState

class Brainstem():
    def __init__(self):
        self.romi = Romi32U4Driver()

        rospy.init_node('brainstem_node', anonymous=True)

        self.odom_state_pub = rospy.Publisher('odom', Odometry,
                queue_size = 2)
        self.battery_pub = rospy.Publisher('battery_state', BatteryState,
                queue_size = 2)

    def publish_odom_state_msg(self):
        """
        Current Pose and Twist state of robot

        TODO: complete odom data with IMU readings
        """
        odom_state_msg = Odometry()
        odom_state_msg.twist.twist.linear.x = self.pose_twist[0]
        odom_state_msg.twist.twist.angular.z = self.pose_twist[1]

        odom_state_msg.header.stamp = rospy.get_rostime()
        self.odom_state_pub.publish(odom_state_msg)

    def cmd_vel_callback(self, data):
        linear_x = data.linear.x
        angular_z = data.angular.z
        # rospy.loginfo(rospy.get_caller_id() + " forward %s, rotation %s", linear_x, angular_z)
        self.romi.twist(linear_x, angular_z)

    def publish_battery_state_msg(self):
        battery_state_msg = BatteryState()
        battery_state_msg.header.stamp = rospy.get_rostime()
        battery_state_msg.voltage = self.battery_mv / 1000.0
        # estimate remaining battery life
        # 1.2v * 6 batteries = 7.2 v
        # 6v is depletion point
        battery_state_msg.percentage = (battery_state_msg.voltage - 6.0) / 1.2
        battery_state_msg.present = True if self.battery_mv > 0 else False
        self.battery_pub.publish(battery_state_msg)

    def read_romi_state(self):
        try:
            # read output data from 32U4
            self.battery_mv = self.romi.read_battery_millivolts()
            self.pose_twist = self.romi.read_pose_twist()
        except:
            rospy.logfatal("reading from Romi I2C driver failed.")

    def start(self):
        rospy.Subscriber('cmd_vel', Twist, self.cmd_vel_callback)

        rate = rospy.Rate(10) # 10hz
        while not rospy.is_shutdown():
            self.read_romi_state()
            self.publish_battery_state_msg()
            self.publish_odom_state_msg()

            rate.sleep()


if __name__ == '__main__':
    try:
        brainstem = Brainstem()
        brainstem.start()
    except rospy.ROSInterruptException:
        pass
