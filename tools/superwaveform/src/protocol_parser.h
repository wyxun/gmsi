#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <vector>
#include <string>
#include <cstdint>
#include <map>

// Represents an identified channel mapping from the device
struct ChannelDesc {
    std::string name;
    float scale;
};

// Extracted frame data representing physics values
struct DataSample {
    double timestamp;  // Simulated local time inside host for rendering
    std::map<int, float> ch_values; // ch_idx to mapped physical value
};

class ProtocolParser {
public:
    ProtocolParser(int max_channels = 16);
    ~ProtocolParser() = default;

    // Feeds bytes into protocol parser. 
    // Populates ready_samples for 'data' frames. Returns true if 'desc' frame updated descriptions.
    bool Feed(const std::vector<uint8_t>& data, std::vector<DataSample>& ready_samples);

    const std::vector<ChannelDesc>& GetChannels() const { return channels_; }

private:
    int max_channels_;
    int mask_bytes_;
    std::vector<ChannelDesc> channels_;
    std::vector<uint8_t> buffer_;

    uint8_t CalcCRC(const uint8_t* payload, size_t len) const;
};

#endif // PROTOCOL_PARSER_H
