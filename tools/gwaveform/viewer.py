import sys
import socket
import threading
import time
import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtWidgets
from protocol import GWaveformProtocol

class WaveformViewer(QtWidgets.QMainWindow):
    def __init__(self, host='localhost', port=9091):
        super().__init__()
        self.host = host
        self.port = port
        self.protocol = GWaveformProtocol(max_channels=16) # Support up to 16
        
        self.data_store = {} # {name: [values]}
        self.curves = {}     # {name: plot_item}
        self.start_time = time.time()
        
        self.init_ui()
        
        self.running = True
        self.thread = threading.Thread(target=self.receive_loop, daemon=True)
        self.thread.start()
        
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(30) # 33 fps

    def init_ui(self):
        self.setWindowTitle("GMSI Waveform Viewer")
        self.central_widget = QtWidgets.QWidget()
        self.setCentralWidget(self.central_widget)
        self.layout = QtWidgets.QVBoxLayout(self.central_widget)
        
        self.plot_widget = pg.PlotWidget(title="Real-time Waveforms")
        self.plot_widget.addLegend()
        self.plot_widget.showGrid(x=True, y=True)
        self.layout.addWidget(self.plot_widget)
        
        self.status_bar = QtWidgets.QStatusBar()
        self.setStatusBar(self.status_bar)
        self.status_bar.showMessage(f"Connecting to {self.host}:{self.port}...")

    def receive_loop(self):
        while self.running:
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    s.connect((self.host, self.port))
                    self.status_bar.showMessage(f"Connected to {self.host}:{self.port}")
                    while self.running:
                        data = s.recv(1024)
                        if not data:
                            break
                        for msg_type, content in self.protocol.feed(data):
                            if msg_type == 'desc':
                                self.handle_desc(content)
                            elif msg_type == 'data':
                                self.handle_data(content)
            except Exception as e:
                self.status_bar.showMessage(f"Error: {e}. Retrying in 2s...")
                time.sleep(2)

    def handle_desc(self, channels):
        print(f"Received Descriptor: {channels}")
        # Initialize store for new channels
        for ch in channels:
            name = ch['name']
            if name not in self.data_store:
                self.data_store[name] = []
                pen = pg.mkPen(color=pg.intColor(len(self.curves)), width=2)
                self.curves[name] = self.plot_widget.plot(name=name, pen=pen)

    def handle_data(self, data):
        ts = time.time() - self.start_time
        for name, val in data['values'].items():
            if name in self.data_store:
                self.data_store[name].append((ts, val))
                # Keep last 1000 points
                if len(self.data_store[name]) > 1000:
                    self.data_store[name].pop(0)

    def update_plot(self):
        for name, curve in self.curves.items():
            if name in self.data_store and self.data_store[name]:
                data = np.array(self.data_store[name])
                curve.setData(data[:, 0], data[:, 1])

    def closeEvent(self, event):
        self.running = False
        super().closeEvent(event)

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=9091)
    args = parser.parse_args()
    
    viewer = WaveformViewer(host=args.host, port=args.port)
    viewer.resize(1024, 600)
    viewer.show()
    sys.exit(app.exec_())
