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


def main():
    # Create a UDP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        while True:
            # Dummy data to send to Wemos D1 Mini
            motorRightPwm = 255
            motorLeftPwm = -50

            # Send data to Wemos D1 Mini
            send_data(client_socket, motorRightPwm, motorLeftPwm)
          #  print(f"motorRightPwm: {motorRightPwm}, motorLeftPwm: {motorLeftPwm}")

            # Receive data from Wemos D1 Mini
            distance, heading = receive_data(client_socket)
            print(f"Received distance: {distance}, heading: {heading}")

            time.sleep(0.01)  # Wait for 5 seconds before sending again

    except KeyboardInterrupt:
        print("Exiting...")

    finally:
        # Close the socket when done
        client_socket.close()

if __name__ == "__main__":
    main()