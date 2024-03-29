#!/usr/bin/env python

import rospy
from led_strip_pkg.msg import LedStripCmdConf

def led_strip_cmd_conf():

    rospy.init_node('led_strip_cmd_conf', anonymous=True)

    pub = rospy.Publisher('led_strip/cmd_conf', LedStripCmdConf, queue_size=10)

    rate = rospy.Rate(1) 

    while not rospy.is_shutdown():
        num_front_left = int(input("Enter num_front_left (0-255): "))
        num_front_right = int(input("Enter num_front_right (0-255): "))
        num_right = int(input("Enter num_right (0-255): "))
        num_back_right = int(input("Enter num_back_right (0-255): "))
        num_back = int(input("Enter num_back (0-255): "))
        num_back_left = int(input("Enter num_back_left (0-255): "))        
        num_left = int(input("Enter num_left (0-255): "))
        num_flag = int(input("Enter num_flag (0-255): "))
              
        msg = LedStripCmdConf()
        msg.num_front_left = num_front_left
        msg.num_front_right = num_front_right
        msg.num_right = num_right
        msg.num_back_right = num_back_right
        msg.num_back = num_back
        msg.num_back_left = num_back_left       
        msg.num_left = num_left
        msg.num_flag = num_flag

        # Publish message
        pub.publish(msg)

        rate.sleep()

if __name__ == '__main__':
    try:
        led_strip_cmd_conf()
    except rospy.ROSInterruptException:
        pass
