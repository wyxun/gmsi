#ifndef GUI_LAYER_H
#define GUI_LAYER_H

#include "protocol_parser.h"
#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include <fstream>
#include <ctime>
#include <iomanip>

class GuiLayer {
public:
    GuiLayer();
    ~GuiLayer();

    // Renders the entire ImGui UI for a single frame
    void Render();

private:
    ProtocolParser parser_;

    // ImPlot Scrolling Buffer wrapper for continuous data
    struct ScrollingBuffer {
        int MaxSize;
        int Offset;
        std::vector<double> XData;
        std::vector<float> YData;
        
        ScrollingBuffer(int max_size = 500000) {
            MaxSize = max_size;
            Offset  = 0;
            XData.reserve(MaxSize);
            YData.reserve(MaxSize);
        }
        void AddPoint(double x, float y) {
            if ((int)XData.size() < MaxSize) {
                XData.push_back(x);
                YData.push_back(y);
            } else {
                XData[Offset] = x;
                YData[Offset] = y;
                Offset =  (Offset + 1) % MaxSize;
            }
        }
        void Erase() {
            if (XData.size() > 0) {
                XData.shrink_to_fit();
                YData.shrink_to_fit();
                XData.clear();
                YData.clear();
                Offset = 0;
            }
        }
    };

    std::map<int, ScrollingBuffer> ch_buffers_;
    std::map<int, bool> ch_visibility_;

    bool paused_ = false;
    double time_last_ = 0.0;
    float history_window_ = 10.0f; // View trailing 10 seconds

    // Recording state
    bool is_recording_ = false;
    std::ofstream record_file_;
    std::string recording_filename_;

    // Real-time measurement
    bool rt_measure_active = false;
    double rt_measure_x = 0;
    double rt_measure_y = 0;

    // Terminal data
    std::string term_log_;
    char term_input_buf_[256];
    bool auto_scroll_ = true;
    ImGuiTextFilter term_filter_;

    // UI Helpers
    void FetchNetworkData();
    void RenderWaveformWindow();
    void RenderTerminalSidebar();
    void RenderDashboard();
    
    // Offline Viewer
    struct OfflineData {
        std::vector<double> time;
        std::map<int, std::vector<double>> channels;
        std::map<int, std::string> channel_names;
    };
    
    struct OfflineSession {
        int id;
        bool open = true;
        bool first_frame = true;
        double x_min = 0, x_max = 10; 
        bool measure_active = false;
        double measure_x = 0;
        double measure_y = 0;
        char filepath[256];
        OfflineData data;
    };

    std::vector<OfflineSession> offline_sessions_;
    int next_session_id_ = 0;

    void RenderOfflineViewer(OfflineSession& session);
    void LoadCSV(OfflineSession& session);
    std::string OpenFileDialog();



    // Adaptive Smoothing State
    double m_virtualClock = 0.0;
    double m_smoothedPeriod = 0.001; // Initial guess: 1ms (1kHz)
    double m_lastWindowTime = 0.0;
    int m_pointsInWindow = 0;
};

#endif // GUI_LAYER_H
