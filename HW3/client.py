import socket
from threading import Thread
import os
import sys

class Client:
    def __init__(self, HOST, PORT):
        self.socket = socket.socket()
        try:
            self.socket.connect((HOST, PORT))
            server_response = self.socket.recv(1024).decode()
            if "Unable to connect" in server_response:
                print(server_response)
                self.socket.close()
                os._exit(0)
            print(server_response)  # Server's welcome message
        except ConnectionRefusedError:
            print("Unable to connect to the server.")
            sys.exit(0)

        # Send and validate username
        while True:
            self.name = input("Please enter your username: ")
            self.socket.send(self.name.encode())
            response = self.socket.recv(1024).decode()
            if "Name accepted!" in response:
                print(response)
                break
            print(response)

        print(f"{self.name} has joined the chat!")
        Thread(target=self.receive_message).start()
        self.send_message()

    def send_message(self):
        while True:
            # Display the prompt dynamically
            sys.stdout.write(f"{self.name}: ")
            sys.stdout.flush()
            message = input()

            if message.strip().lower() == "exit":
                self.socket.send("exit".encode())
                print("You left the chat.\nDisconnected from server.")
                os._exit(0)

            # Send the plain message to the server
            self.socket.send(message.encode())

            # Clear the input line after sending the message
            sys.stdout.write("\033[F\033[K")  # Move up and clear line
            sys.stdout.flush()

    def receive_message(self):
        while True:
            try:
                server_message = self.socket.recv(1024).decode()
                if not server_message:
                    raise ConnectionResetError
                # Clear the current input line, display server message, and re-prompt
                sys.stdout.write("\r\033[K")  # Clear the current line
                sys.stdout.write(f"{server_message}\n")  # Print server message
                sys.stdout.write(f"{self.name}: ")  # Re-display prompt
                sys.stdout.flush()
            except (ConnectionResetError, OSError):
                print("Disconnected from server.")
                os._exit(0)

if __name__ == "__main__":
    Client("127.0.0.1", 12345)
