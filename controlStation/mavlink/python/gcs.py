#Script to receive via MAVLink (GCS side)
import time
from pymavlink import mavutil

#If through WiFi
mavlink_connection = mavutil.mavlink_connection(device='udpin:192.168.178.127:14550', baudrate=57600)

# https://www.ardusub.com/developers/pymavlink.html#send-rc-joystick
# https://medium.com/@tonyjacob_/using-mavlink-for-custom-applications-466e1d632f84
#https://github.com/cmb87/ESP32-MAVLink-Arduino-Example

def set_rc_channel_pwm(channel_id, pwm=1500):
    """ Set RC channel pwm value
    Args:
        channel_id (TYPE): Channel ID
        pwm (int, optional): Channel pwm value 1100-1900
    """
    if channel_id < 1 or channel_id > 18:
        print("Channel does not exist.")
        return

    # Mavlink 2 supports up to 18 channels:
    # https://mavlink.io/en/messages/common.html#RC_CHANNELS_OVERRIDE
    rc_channel_values = [65535 for _ in range(18)]
    rc_channel_values[channel_id - 1] = pwm

    mavlink_connection.mav.rc_channels_override_send(
        mavlink_connection.target_system,                # target_system
        mavlink_connection.target_component,             # target_component
        *rc_channel_values)                  # RC channel list, in microseconds.

def wait_conn():
    """
    Sends a ping to stabilish the UDP communication and awaits for a response
    """
    msg = None
    while not msg:
        mavlink_connection.mav.ping_send(
            int(time.time() * 1e6), # Unix time in microseconds
            0, # Ping number
            0, # Request ping of all systems
            0 # Request ping of all components
        )
        msg = mavlink_connection.recv_match()
        time.sleep(0.5)

#Else through a radio antenna,
# mavlink_connection = mavutil.mavserial(device='/dev/ttyUSB0', baud=57600)


#mavlink_connection.wait_heartbeat()

mavlink_connection.wait_heartbeat()
lastHearbeatReceived = time.time()


while 1:

    if ( time.time()-lastHearbeatReceived >= 1.0):
        
        mavlink_connection.wait_heartbeat()
        lastHearbeatReceived = time.time()
        print("-----------------------------------")

    #print(mavlink_connection.target_system)
    #print("[INFO] Heartbeat Received")

   # for i in range(4):
    
    msg = mavlink_connection.recv_msg()
    if msg is not None:
        print(msg.to_dict())



    
  #  print(msg)



    # Works! :)
    # mavlink_connection.mav.manual_control_send(
    #     mavlink_connection.target_system,
    #     500,
    #     -500,
    #     250,
    #     500,
    #     0
    # )


    mavlink_connection.mav.rc_channels_override_send(
        mavlink_connection.target_system,
        255,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
        1500,
    )
    time.sleep(0.05)