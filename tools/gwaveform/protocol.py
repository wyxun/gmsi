import struct

class GWaveformProtocol:
    SYNC_H = 0xAA
    SYNC_L = 0x55
    TYPE_DATA = 0x00
    TYPE_DESC = 0xFD

    def __init__(self, max_channels=8):
        self.max_channels = max_channels
        self.mask_bytes = (max_channels + 7) // 8
        self.channels = []
        self._buffer = bytearray()

    def feed(self, data):
        """Append raw bytes and yield parsed packets."""
        self._buffer.extend(data)
        
        while len(self._buffer) >= 2:
            # Sync search
            if self._buffer[0] != self.SYNC_H or self._buffer[1] != self.SYNC_L:
                self._buffer.pop(0)
                continue
            
            if len(self._buffer) < 4:
                break
                
            frame_type = self._buffer[2]
            
            if frame_type == self.TYPE_DESC:
                # Descriptor Frame
                ch_count = self._buffer[3]
                frame_len = 4 + 12 * ch_count + 1
                if len(self._buffer) < frame_len:
                    break
                
                payload = self._buffer[2:frame_len-1]
                crc = self._buffer[frame_len-1]
                if self.calc_crc(payload) == crc:
                    self.channels = []
                    for i in range(ch_count):
                        off = 4 + i * 12
                        name = self._buffer[off:off+8].split(b'\x00')[0].decode('ascii')
                        scale = struct.unpack('<f', self._buffer[off+8:off+12])[0]
                        self.channels.append({'name': name, 'scale': scale})
                    yield ('desc', self.channels)
                
                self._buffer = self._buffer[frame_len:]

            else:
                # Data Frame (frame_type is seq number)
                seq = frame_type
                mask = list(self._buffer[3:3+self.mask_bytes])
                
                # Count active channels in mask
                active_count = 0
                for b in mask:
                    active_count += bin(b).count('1')
                
                frame_len = 3 + self.mask_bytes + active_count * 2 + 1
                if len(self._buffer) < frame_len:
                    break
                
                payload = self._buffer[2:frame_len-1]
                crc = self._buffer[frame_len-1]
                
                if self.calc_crc(payload) == crc:
                    data_vals = {}
                    val_idx = 3 + self.mask_bytes
                    active_idx = 0
                    for ch_idx in range(len(self.channels)):
                        byte_idx = ch_idx // 8
                        bit_idx = ch_idx % 8
                        if mask[byte_idx] & (1 << bit_idx):
                            val = struct.unpack('<h', self._buffer[val_idx:val_idx+2])[0]
                            real_val = val / self.channels[ch_idx]['scale']
                            data_vals[self.channels[ch_idx]['name']] = real_val
                            val_idx += 2
                    yield ('data', {'seq': seq, 'values': data_vals})
                
                self._buffer = self._buffer[frame_len:]

    @staticmethod
    def calc_crc(data):
        crc = 0xFF
        for b in data:
            crc ^= b
        return crc
