#!/usr/bin/env python

import rospy
from led_strip_pkg.msg import LedStripCmd

def led_strip_cmd():

    rospy.init_node('led_strip_cmd', anonymous=True)

    pub = rospy.Publisher('led_strip/cmd', LedStripCmd, queue_size=10)

    rate = rospy.Rate(1) 

    while not rospy.is_shutdown():
        drive_mode = int(input("Enter drive mode (0-3): "))
        signal = int(input("Enter signal (0-3): "))
        effect = int(input("Enter effect (0-5): "))
        
        msg = LedStripCmd()
        msg.drive_mode = drive_mode
        msg.signal = signal
        msg.effect = effect

        # Publish message
        pub.publish(msg)

        rate.sleep()

if __name__ == '__main__':
    try:
        led_strip_cmd()
    except rospy.ROSInterruptException:
        pass

