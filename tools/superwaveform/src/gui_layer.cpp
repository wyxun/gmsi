#include "gui_layer.h"
#include "network_mgr.h"
#include "imgui.h"
#include "implot.h"
#include <iostream>
#include <cstring>

GuiLayer::GuiLayer() : parser_(8) {
    std::memset(term_input_buf_, 0, sizeof(term_input_buf_));
}

GuiLayer::~GuiLayer() {
}

void GuiLayer::FetchNetworkData() {
    auto& net = NetworkMgr::GetInstance();

    // Fetch Ch0 Log lines
    std::string ch0_str;
    if (net.FetchCh0Data(ch0_str)) {
        term_log_ += ch0_str;
        if (term_log_.size() > 500000) { // Limit log size ~500kb
            term_log_.erase(0, term_log_.size() - 250000);
        }
    }

    // Fetch Ch1 Binary Stream
    std::vector<uint8_t> ch1_bytes;
    if (net.FetchCh1Data(ch1_bytes)) {
        std::vector<DataSample> samples;
        bool desc_changed = parser_.Feed(ch1_bytes, samples);
        
        if (desc_changed) {
            // New channels map received, could reset buffer or handle dynamically
            // Currently, just let it append to existing buffers if index matches
        }

        if (!paused_) {
            double now = ImGui::GetTime();
            time_last_ = now;
            
            // Initialize timing on first data arrival
            if (m_lastWindowTime == 0.0) {
                m_lastWindowTime = now;
                m_virtualClock = now;
            }
            
            // 1. Adaptive period estimation (every 500ms)
            m_pointsInWindow += (int)samples.size();
            if (now - m_lastWindowTime > 0.5) {
                if (m_pointsInWindow > 0) {
                    double estimated = (now - m_lastWindowTime) / m_pointsInWindow;
                    // If it's the first real estimate, take it directly; otherwise LPF
                    if (m_smoothedPeriod == 0.001) m_smoothedPeriod = estimated;
                    else m_smoothedPeriod = m_smoothedPeriod * 0.8 + estimated * 0.2;
                }
                m_lastWindowTime = now;
                m_pointsInWindow = 0;
            }

            for (const auto& s : samples) {
                // 2. Virtual Clock Distribution with Safety Bounds
                m_virtualClock += m_smoothedPeriod;

                // Relaxed Sync Check: Allow up to 500ms lead or 2.0s lag before hard sync
                if (m_virtualClock > now + 0.5 || m_virtualClock < now - 2.0) {
                    m_virtualClock = now;
                }
                
                time_last_ = m_virtualClock;

                for (const auto& kv : s.ch_values) {
                    int ch_idx = kv.first;
                    float val = kv.second;
                    // Initialize lazily
                    if (ch_buffers_.find(ch_idx) == ch_buffers_.end()) {
                        ch_buffers_[ch_idx] = ScrollingBuffer(1000000); 
                        ch_visibility_[ch_idx] = true;
                    }
                    ch_buffers_[ch_idx].AddPoint(m_virtualClock, val);
                }
            }
        }
    }
}

void GuiLayer::Render() {
    FetchNetworkData();

    // Setup Main DockSpace
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
                                    ImGuiWindowFlags_NoNavFocus;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("MainDockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    RenderDashboard();
    RenderWaveformWindow();
    RenderTerminalSidebar();

    ImGui::End(); // MainDockSpace
}

void GuiLayer::RenderDashboard() {
    ImGui::Begin("Dashboard");
    auto& net = NetworkMgr::GetInstance();

    ImGui::Text("Connections:");
    ImGui::SameLine();
    ImGui::TextColored(net.IsCh0Connected() ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), "[Ch0 Shell: %s]", net.IsCh0Connected() ? "ON" : "OFF");
    ImGui::SameLine();
    ImGui::TextColored(net.IsCh1Connected() ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), "[Ch1 Wave: %s]", net.IsCh1Connected() ? "ON" : "OFF");
    ImGui::SameLine();
    ImGui::Text("| Sample Rate: %.1f Hz", 1.0 / m_smoothedPeriod);

    ImGui::Separator();
    
    // Wave Control
    if (ImGui::Button(paused_ ? "Resume Waveform" : "Pause Waveform")) {
        paused_ = !paused_;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Waveform Buffer")) {
        for (auto& pair : ch_buffers_) {
            pair.second.Erase();
        }
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::SliderFloat("Time Window", &history_window_, 1.0f, 60.0f, "%.1f s");

    ImGui::Separator();
    ImGui::Text("Quick User Macros (Ch0):");
    if (ImGui::Button("wave start")) { net.SendToCh0("wave start\n"); } ImGui::SameLine();
    if (ImGui::Button("wave stop")) { net.SendToCh0("wave stop\n"); } ImGui::SameLine();
    if (ImGui::Button("log -I (Hide Info)")) { net.SendToCh0("log -I\n"); } ImGui::SameLine();
    if (ImGui::Button("log -A (Show All)")) { net.SendToCh0("log -A\n"); }

    ImGui::End();
}

void GuiLayer::RenderWaveformWindow() {
    ImGui::Begin("Oscilloscope");

    if (ImPlot::BeginPlot("##WaveformPlot", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Time(s)", "Value");
        
        // If not paused, stick the X-axis right to the newest time
        if (!paused_) {
            ImPlot::SetupAxisLimits(ImAxis_X1, time_last_ - history_window_, time_last_, ImGuiCond_Always);
        } else {
            // Allows user dragging and scrolling freely
        }

        const auto& channels = parser_.GetChannels();
        
        for (auto& pair : ch_buffers_) {
            int ch_idx = pair.first;
            std::string name = (ch_idx < (int)channels.size()) ? channels[ch_idx].name : "CH" + std::to_string(ch_idx);

            if (ch_buffers_[ch_idx].XData.size() > 0) {
                ImPlot::PlotLineG(name.c_str(), [](int idx, void* data) {
                    ScrollingBuffer* buf = (ScrollingBuffer*)data;
                    int real_idx = (buf->Offset + idx) % (int)buf->XData.size();
                    return ImPlotPoint(buf->XData[real_idx], buf->YData[real_idx]);
                }, &ch_buffers_[ch_idx], (int)ch_buffers_[ch_idx].XData.size());
            }
        }
        ImPlot::EndPlot();
    }

    ImGui::End();
}

void GuiLayer::RenderTerminalSidebar() {
    ImGui::Begin("GShell Terminal");

    ImGui::Checkbox("Auto-scroll", &auto_scroll_);
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        term_log_.clear();
    }

    ImGui::Separator();

    // Log viewer
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::TextUnformatted(term_log_.c_str());
    if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();

    ImGui::Separator();

    // Command Input
    bool reclaim_focus = false;
    ImGui::PushItemWidth(-ImGui::GetStyle().ItemSpacing.x * 7); // Leave space for send btn
    if (ImGui::InputText("##Input", term_input_buf_, IM_ARRAYSIZE(term_input_buf_), ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (term_input_buf_[0]) {
            std::string cmd = std::string(term_input_buf_) + "\r\n";
            NetworkMgr::GetInstance().SendToCh0(cmd);
            std::memset(term_input_buf_, 0, sizeof(term_input_buf_));
        }
        reclaim_focus = true;
    }
    ImGui::PopItemWidth();
    
    ImGui::SetItemDefaultFocus();
    if (reclaim_focus)
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

    ImGui::End();
}
