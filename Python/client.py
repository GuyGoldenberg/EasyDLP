from network import network_base

sock = network_base.NetworkBase()
sock.connect("127.0.0.1", 5050)
sock.send("Hello from client" * 1000000 + "\r\n")
print sock.recv(1024)
