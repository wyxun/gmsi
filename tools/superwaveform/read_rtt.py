import socket
import sys
import time

def read_rtt(port=9090, duration=10):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5.0)
        s.connect(('localhost', port))
        print(f"Connected to RTT server on port {port}")
        
        start_time = time.time()
        while time.time() - start_time < duration:
            try:
                data = s.recv(1024)
                if data:
                    print(data.decode('utf-8', errors='ignore'), end='')
            except socket.timeout:
                continue
    except Exception as e:
        print(f"Error: {e}")
    finally:
        s.close()

if __name__ == "__main__":
    duration = 15
    if len(sys.argv) > 1:
        duration = int(sys.argv[1])
    read_rtt(duration=duration)
