import socket

size = 8192

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))
sequenceNumber = 1
try:
    while True:
        data, address = sock.recvfrom(size)
        sock.sendto((str(sequenceNumber) + " " + data.decode()).encode(), address)
        sequenceNumber += 1
finally:
    sock.close()
