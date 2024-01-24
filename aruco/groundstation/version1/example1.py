import pygame
import sys
import socket
import struct
import time

# Wemos D1 Mini (ESP8266) IP address and port
wemos_ip = "192.168.178.59"  # Replace with the actual IP address
wemos_port = 8889

# Define the struct format
struct_format_send = "ii"  # 
struct_format_receive = "if"  # 


def send_data(sock, motorRightPwm, motorLeftPwm):
    # Pack the data into bytes
    data = struct.pack(struct_format_send, motorRightPwm, motorLeftPwm)    
    # Send the data to the Wemos D1 Mini
    sock.sendto(data, (wemos_ip, wemos_port))


def receive_data(sock):
    # Receive data from the Wemos D1 Mini
    data, addr = sock.recvfrom(1024)
    # Unpack the received bytes into the struct
    received_data = struct.unpack(struct_format_receive, data)

    return received_data



def joystick_example():

    pygame.init()

    # Initialize the joystick
    pygame.joystick.init()

    if pygame.joystick.get_count() == 0:
        print("No joystick found. Exiting.")
        pygame.quit()
        sys.exit()

    joystick = pygame.joystick.Joystick(0)
    joystick.init()

    print(f"Joystick Name: {joystick.get_name()}")
    print(f"Number of Axes: {joystick.get_numaxes()}")
    print(f"Number of Buttons: {joystick.get_numbuttons()}")


    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        while True:
            pygame.event.pump()

            # Get and print the state of each axis
            axes = [joystick.get_axis(i) for i in range(joystick.get_numaxes())]
           # print("Axes:", axes)

            # Get and print the state of each button
            buttons = [joystick.get_button(i) for i in range(joystick.get_numbuttons())]
           # print("Buttons:", buttons)

            #pygame.time.delay(100)  # Add a small delay to avoid high CPU usage

            try:
                # Dummy data to send to Wemos D1 Mini
                motorRightPwm = int(125*(-axes[1] - 0.1*axes[0]))
                motorLeftPwm =  int(125*(-axes[1] + 0.1*axes[0]))

                # Send data to Wemos D1 Mini
                send_data(client_socket, motorRightPwm, motorLeftPwm)
                #print(f"motorRightPwm: {motorRightPwm}, motorLeftPwm: {motorLeftPwm}")

                # Receive data from Wemos D1 Mini
                distance, heading = receive_data(client_socket)
                print(f"Received distance: {distance}, heading: {heading}")

            except KeyboardInterrupt:
                print("Exiting...")
            
            pygame.time.delay(10)  # Add a small delay to avoid high CPU usage




    finally:
        client_socket.close() 

        pygame.quit()

if __name__ == "__main__":
    joystick_example()