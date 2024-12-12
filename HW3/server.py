import socket
from threading import Thread
import signal
import sys

class Server:
    Clients = []
    MAX_CLIENTS = 5

    def __init__(self, HOST, PORT):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind((HOST, PORT))
        self.socket.listen(self.MAX_CLIENTS)
        print(f"Server started on {HOST}:{PORT}")
        signal.signal(signal.SIGINT, self.shutdown)

    def shutdown(self, signum, frame):
        print("\nShutting down the server...")
        for client in Server.Clients:
            client["client_socket"].close()
        self.socket.close()
        sys.exit(0)

    def is_name_unique(self, name):
        return all(client["client_name"] != name for client in self.Clients)

    def listen(self):
        while True:
            client_socket, client_address = self.socket.accept()

            if len(self.Clients) >= self.MAX_CLIENTS:
                print(f"Connection attempt from {client_address} rejected: Maximum connections reached.")
                client_socket.send("Unable to connect: Maximum connections reached.".encode())
                client_socket.close()
                continue  # Keep listening for other connections

            print(f"New connection from {client_address}")
            client_socket.send("Welcome!".encode())

            # Validate username
            while True:
                try:
                    client_name = client_socket.recv(1024).decode()
                    if self.is_name_unique(client_name):
                        client_socket.send("Name accepted!".encode())
                        break
                    else:
                        client_socket.send("Name already in use. Please enter a different name.".encode())
                except socket.error:
                    print(f"Error receiving username from {client_address}.")
                    client_socket.close()
                    return

            client = {"client_name": client_name, "client_socket": client_socket}
            print(f"{client_name} has joined the chat!")
            self.broadcast_message(client_name, f"{client_name} has joined the chat!")
            self.Clients.append(client)
            Thread(target=self.handle_client, args=(client,)).start()


    def handle_client(self, client):
        client_name = client["client_name"]
        client_socket = client["client_socket"]

        while True:
            try:
                message = client_socket.recv(1024).decode()
                if not message:  # Handle case where the client disconnects without sending 'exit'
                    raise ConnectionResetError

                if message.strip().lower() == "exit":
                    self.broadcast_message(client_name, f"{client_name} has left the chat!")
                    print(f"{client_name} has left the chat.")
                    self.Clients.remove(client)
                    client_socket.close()
                    break  # Exit the loop and stop handling this client
                else:
                    # Prepend the username to the message
                    formatted_message = f"{client_name}: {message}"
                    # Alice sent: Hi everyone! 
                    print(f"{client_name} sent: {message}")
                    self.broadcast_message(client_name, formatted_message)
            except (ConnectionResetError, BrokenPipeError, OSError):
                # Handle cases where the connection is forcibly closed
                print(f"{client_name} has disconnected unexpectedly.")
                self.Clients.remove(client)
                client_socket.close()
                break  # Exit the loop and stop handling this client


    def broadcast_message(self, sender_name, message):
        for client in self.Clients:
            # if client["client_name"] != sender_name:
            client["client_socket"].send(message.encode())

if __name__ == "__main__":
    server = Server("127.0.0.1", 12345)
    server.listen()
