import socket
 
size = 8192
server_address = "localhost"
server_port = 9876
message = []
for i in range(51):
    message.append(str(i))
try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    for i in range(51):
        sock.sendto(message[i].encode(), (server_address, server_port))
        recv, server_addr = sock.recvfrom(size)
        print(str(recv, "utf-8"))
    sock.close()
except:
    print("cannot reach the server")
