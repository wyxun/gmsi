#ifndef NETWORK_MGR_H
#define NETWORK_MGR_H

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>

class NetworkMgr {
public:
    static NetworkMgr& GetInstance() {
        static NetworkMgr instance;
        return instance;
    }

    void Init();
    void Shutdown();

    // Channel 0 (Terminal) interface
    bool IsCh0Connected() const { return ch0_connected_; }
    void SendToCh0(const std::string& data);
    bool FetchCh0Data(std::string& out_data); // Returns true if new data appended

    // Channel 1 (Binary Protocol) interface
    bool IsCh1Connected() const { return ch1_connected_; }
    bool FetchCh1Data(std::vector<uint8_t>& out_data); // Extracts raw stream bytes

private:
    NetworkMgr() = default;
    ~NetworkMgr() = default;

    void ThreadLoopCh0();
    void ThreadLoopCh1();

    std::atomic<bool> running_{false};
    
    std::thread ch0_thread_;
    std::atomic<bool> ch0_connected_{false};
    std::mutex ch0_mutex_;
    std::string ch0_buffer_recv_;
    std::string ch0_buffer_send_;

    std::thread ch1_thread_;
    std::atomic<bool> ch1_connected_{false};
    std::mutex ch1_mutex_;
    std::vector<uint8_t> ch1_buffer_recv_;
};

#endif // NETWORK_MGR_H
