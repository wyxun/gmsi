#ifndef GUI_LAYER_H
#define GUI_LAYER_H

#include "protocol_parser.h"
#include <vector>
#include <string>

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

    // Terminal data
    std::string term_log_;
    char term_input_buf_[256];
    bool auto_scroll_ = true;

    // UI Helpers
    void FetchNetworkData();
    void RenderWaveformWindow();
    void RenderTerminalSidebar();
    void RenderDashboard();

    // Adaptive Smoothing State
    double m_virtualClock = 0.0;
    double m_smoothedPeriod = 0.001; // Initial guess: 1ms (1kHz)
    double m_lastWindowTime = 0.0;
    int m_pointsInWindow = 0;
};

#endif // GUI_LAYER_H
