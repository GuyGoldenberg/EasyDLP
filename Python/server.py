from network import network_base
sock = network_base.NetworkBase()
sock.bind("5050")
sock.listen(10)
client = sock.accept()
r = sock.recv(1024, client=client)
s = r
while len(r) > 0 and not r.endswith("\r\n"):
    print r
    r = sock.recv(1024, client=client)
    s += r
print s
sock.send("Hello from server!", client=client)
