#!/usr/bin/env python

import rospy
import can
import struct
from led_strip_pkg.msg import LedStripCmd, LedStripCmdConf, LedStripStatus, LedStripConfig

ID_Device = 0x2A

ID_PKG_cmd = 0x01
ID_PKG_cmd_conf = 0x02
ID_PKG_status = 0x10
ID_PKG_config = 0x11

CAN_ID = ID_PKG_cmd << 8 | ID_Device
CAN_ID_Conf = ID_PKG_cmd_conf << 8 | ID_Device
CAN_INTERFACE = 'can0'

def id_decode(arbitration_id):
    id_device = arbitration_id & 0xFF
    id_pkg = (arbitration_id >> 8) & 0xFF
    return id_device, id_pkg
        
def pack_data(drive_mode, signal, effect):
    mode = ((drive_mode & 0x03) << 2) | (signal & 0x03)
    effect = effect & 0xFF
    return bytearray(struct.pack('BB', mode, effect))

def led_strip_cmd_callback(msg):
    drive_mode = msg.drive_mode
    signal = msg.signal
    effect = msg.effect
    data_msg = pack_data(drive_mode, signal, effect)
    msg = can.Message(arbitration_id=CAN_ID, is_extended_id=True, data=data_msg, dlc=2)
    try:
        bus.send(msg)
        print("Message sent successfully.")
    except can.CanError:
        print("Error sending message.")
        
def led_strip_cmd_conf_callback(msg):
    msg = can.Message(arbitration_id=CAN_ID_Conf, is_extended_id=True, data=[msg.num_front_left, msg.num_front_right, msg.num_right, msg.num_back_right, msg.num_back, msg.num_back_left, msg.num_left, msg.num_flag], dlc=8)
    try:
        bus.send(msg)
        print("Message sent successfully.")
    except can.CanError:
        print("Error sending message.")
        
def led_strip():
    rospy.init_node('led_strip', anonymous=True)
    
    pub_status = rospy.Publisher('led_strip/status', LedStripStatus, queue_size=10)
    pub_config = rospy.Publisher('led_strip/config', LedStripConfig, queue_size=10)
    
    rospy.Subscriber("led_strip/cmd", LedStripCmd, led_strip_cmd_callback)
    rospy.Subscriber("led_strip/cmd_conf", LedStripCmdConf, led_strip_cmd_conf_callback)

    while not rospy.is_shutdown():
        msg = bus.recv(timeout=0.5)
        if msg is not None and msg.arbitration_id == 0x0000102A:
            signal = msg.data[0] & 0x03
            drive_mode = (msg.data[0] & 0x0C) >> 2
            effect = msg.data[1] & 0xFF

            status_msg = LedStripStatus()
            status_msg.header.stamp = rospy.Time.now()
            status_msg.drive_mode = drive_mode
            status_msg.signal = signal
            status_msg.effect = effect

            pub_status.publish(status_msg)

        elif msg is not None and msg.arbitration_id == 0x0000112A:
            num_front_left = msg.data[0] 
            num_front_right = msg.data[1]
            num_right = msg.data[2]
            num_back_right = msg.data[3]
            num_back = msg.data[4]
            num_back_left = msg.data[5]
            num_left = msg.data[6]
            num_flag = msg.data[7]

            config_msg = LedStripConfig()
            config_msg.header.stamp = rospy.Time.now()
            config_msg.num_front_left = num_front_left
            config_msg.num_front_right = num_front_right
            config_msg.num_right= num_right
            config_msg.num_back_right = num_back_right
            config_msg.num_back = num_back
            config_msg.num_back_left = num_back_left
            config_msg.num_left = num_left
            config_msg.num_flag = num_flag

            pub_config.publish(config_msg)
    bus.shutdown()

if __name__ == "__main__":
    # Initialize CAN bus
    try:
    	bus = can.interface.Bus(channel=CAN_INTERFACE, bustype='socketcan')
    	led_strip()
    except Exception as e:
    	rospy.logerr("CAN error")
    	rospy.signal_shutdown("CAN bus init error")   
