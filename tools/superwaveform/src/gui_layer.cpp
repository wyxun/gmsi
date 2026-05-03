#include "gui_layer.h"
#include "network_mgr.h"
#include "imgui.h"
#include "implot.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

GuiLayer::GuiLayer() : parser_(8) {
    std::memset(term_input_buf_, 0, sizeof(term_input_buf_));
    macro_mgr_.RestoreLastSession();
}

GuiLayer::~GuiLayer() {
    if (record_file_.is_open()) record_file_.close();
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
                    // Sanity check: 0.1Hz to 1MHz
                    if (estimated > 10.0) estimated = 10.0;
                    if (estimated < 0.000001) estimated = 0.000001;

                    // If it's the first real estimate, take it directly; otherwise LPF
                    if (m_smoothedPeriod == 0.001) m_smoothedPeriod = estimated;
                    else m_smoothedPeriod = m_smoothedPeriod * 0.9 + estimated * 0.1; // Slower adaptation
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
                
                // Record to CSV
                if (is_recording_ && record_file_.is_open()) {
                    record_file_ << m_virtualClock;
                    const auto& channels = parser_.GetChannels();
                    for (size_t i = 0; i < channels.size(); i++) {
                        auto it = s.ch_values.find(i);
                        if (it != s.ch_values.end()) {
                            record_file_ << "," << it->second;
                        } else {
                            record_file_ << ",";
                        }
                    }
                    record_file_ << "\n";
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

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.07f, 0.09f, 0.94f);
    colors[ImGuiCol_Header]                 = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.15f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.18f, 0.30f, 0.45f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.10f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.15f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.35f, 0.50f, 1.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.10f, 0.15f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.GrabRounding = 4.0f;

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

    for (auto it = offline_sessions_.begin(); it != offline_sessions_.end(); ) {
        if (!it->open) {
            it = offline_sessions_.erase(it);
        } else {
            RenderOfflineViewer(*it);
            ++it;
        }
    }

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
    if (ImGui::Button(paused_ ? "Resume" : "Pause")) paused_ = !paused_;
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("Clear")) {
        for (auto& pair : ch_buffers_) {
            pair.second.XData.clear();
            pair.second.YData.clear();
        }
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::SliderFloat("Window", &history_window_, 0.1f, 60.0f, "%.1fs");

    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 120.0f); // Push to the right
    
    if (is_recording_) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Stop CSV")) {
            is_recording_ = false;
            if (record_file_.is_open()) record_file_.close();
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.4f, 0.2f, 1.0f));
        if (ImGui::Button("Start CSV")) {
            std::time_t t = std::time(nullptr);
            std::tm tm = *std::localtime(&t);
            std::stringstream ss;
            ss << "rec_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".csv";
            recording_filename_ = ss.str();
            
            record_file_.open(recording_filename_);
            if (record_file_.is_open()) {
                is_recording_ = true;
                // Write header
                record_file_ << "Time";
                const auto& channels = parser_.GetChannels();
                for (const auto& ch : channels) record_file_ << "," << ch.name;
                record_file_ << "\n";
            }
        }
        ImGui::PopStyleColor();
    }
    
    ImGui::Separator();
    
    // Offline Analysis
    ImGui::Text("Data Analysis:");
    ImGui::SameLine();
    if (ImGui::Button("Open CSV Viewer")) {
        std::string path = OpenFileDialog();
        if (!path.empty()) {
            // Remove closed sessions to keep ID space small
            offline_sessions_.erase(
                std::remove_if(offline_sessions_.begin(), offline_sessions_.end(), 
                [](const OfflineSession& s) { return !s.open; }), 
                offline_sessions_.end());

            OfflineSession session;
            session.id = next_session_id_++;
            session.open = true;
            strncpy(session.filepath, path.c_str(), sizeof(session.filepath)-1);
            LoadCSV(session);
            offline_sessions_.push_back(session);
        }
    }

    ImGui::Separator();

    // --- Macro Manager UI ---
    ImGui::Text("Quick User Macros (Ch0):");
    ImGui::SameLine();
    if (ImGui::Button("Load Macros...")) {
        std::string path = OpenIniFileDialog();
        if (!path.empty()) {
            if (macro_mgr_.LoadFromFile(path)) {
                macro_mgr_.PersistCurrentPath();
            }
        }
    }
    ImGui::SameLine();
    const std::string& cur_path = macro_mgr_.GetCurrentPath();
    if (cur_path.empty()) {
        ImGui::TextDisabled("(No macros loaded)");
    } else {
        size_t slash = cur_path.find_last_of("/\\");
        std::string fname = (slash != std::string::npos) ? cur_path.substr(slash + 1) : cur_path;
        ImGui::TextDisabled("%s", fname.c_str());
    }

    // Render dynamic macro buttons
    const auto& macros = macro_mgr_.GetMacros();
    if (!macros.empty()) {
        ImGui::Separator();
        for (size_t i = 0; i < macros.size(); ++i) {
            char btn_id[128];
            snprintf(btn_id, sizeof(btn_id), "%s##macro%zu", macros[i].label.c_str(), i);
            if (ImGui::Button(btn_id)) {
                net.SendToCh0(macros[i].command + "\n");
            }
            if (i + 1 < macros.size()) ImGui::SameLine();
        }
    }
    // --- End Macro Manager UI ---

    ImGui::End();
}

void GuiLayer::RenderWaveformWindow() {
    ImGui::Begin("Oscilloscope");

    if (ImPlot::BeginPlot("##WaveformPlot", ImVec2(-1, -1))) {
        // 1. Setup ALL axis properties FIRST
        ImPlot::SetupAxes("Time(s)", "Value", 0, 0);
        ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 0.02, 60.0);
        if (!paused_) {
            ImPlot::SetupAxisLimits(ImAxis_X1, time_last_ - history_window_, time_last_, ImGuiCond_Always);
        }

        // 2. Interaction logic (Safe after Setup, but before/during Plotting)
        if (ImPlot::IsPlotHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f) {
            history_window_ *= (1.0f - ImGui::GetIO().MouseWheel * 0.1f);
            if (history_window_ < 0.02f) history_window_ = 0.02f;
            if (history_window_ > 60.0f) history_window_ = 60.0f;
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
        
        // --- NEW: Interaction & Measurement Logic ---
        if (ImPlot::IsPlotHovered()) {
            ImPlotPoint mouse = ImPlot::GetPlotMousePos();
            ImVec2 mouse_pixel = ImPlot::PlotToPixels(mouse);
            ImDrawList* draw_list = ImPlot::GetPlotDrawList();
            ImVec2 plot_min = ImPlot::GetPlotPos();
            ImVec2 plot_max = ImVec2(plot_min.x + ImPlot::GetPlotSize().x, plot_min.y + ImPlot::GetPlotSize().y);
            
            // Toggle measurement on Space
            if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
                rt_measure_active = !rt_measure_active;
                rt_measure_x = mouse.x;
                rt_measure_y = mouse.y;
            }

            // Helper for dashed lines
            auto draw_dashed_h = [&](float x1, float x2, float y, ImU32 col) {
                for (float x = x1; x < x2; x += 10.0f) draw_list->AddLine(ImVec2(x, y), ImVec2((x + 5.0f < x2 ? x + 5.0f : x2), y), col, 1.0f);
            };
            auto draw_dashed_v = [&](float x, float y1, float y2, ImU32 col) {
                for (float y = y1; y < y2; y += 10.0f) draw_list->AddLine(ImVec2(x, y), ImVec2(x, (y + 5.0f < y2 ? y + 5.0f : y2)), col, 1.0f);
            };

            // Current cursor (Yellow Dashed)
            draw_dashed_v(mouse_pixel.x, plot_min.y, plot_max.y, IM_COL32(255, 255, 0, 100));
            draw_dashed_h(plot_min.x, plot_max.x, mouse_pixel.y, IM_COL32(255, 255, 0, 100));

            if (rt_measure_active) {
                ImVec2 ref_pixel = ImPlot::PlotToPixels(rt_measure_x, rt_measure_y);
                // Reference cursor (Red Dashed)
                draw_dashed_v(ref_pixel.x, plot_min.y, plot_max.y, IM_COL32(255, 0, 0, 150));
                draw_dashed_h(plot_min.x, plot_max.x, ref_pixel.y, IM_COL32(255, 0, 0, 150));
                draw_list->AddRectFilled(ref_pixel, mouse_pixel, IM_COL32(255, 0, 0, 30));
            }

            ImGui::BeginTooltip();
            ImGui::Text("Current X: %.4f s", mouse.x);
            ImGui::Text("Current Y: %.2f", mouse.y);
            if (rt_measure_active) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Ref X: %.4f s", rt_measure_x);
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Ref Y: %.2f", rt_measure_y);
                double dx = mouse.x - rt_measure_x;
                double abs_dx = (dx < 0 ? -dx : dx);
                ImGui::TextColored(ImVec4(0, 1, 1, 1), "dX: %.4f s (%.2f Hz)", dx, (abs_dx > 1e-6 ? 1.0 / abs_dx : 0));
                ImGui::TextColored(ImVec4(0, 1, 1, 1), "dY: %.2f", mouse.y - rt_measure_y);
            }
            ImGui::EndTooltip();
        }
        // ---------------------------

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
    term_filter_.Draw("Filter (\"incl,-excl\")", 180);
    ImGui::Separator();
    
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    if (term_filter_.IsActive()) {
        const char* line_start = term_log_.c_str();
        const char* log_end = line_start + term_log_.length();
        while (line_start < log_end) {
            const char* line_end = strchr(line_start, '\n');
            if (!line_end) line_end = log_end;
            if (term_filter_.PassFilter(line_start, line_end)) {
                ImGui::TextUnformatted(line_start, line_end);
            }
            line_start = line_end + 1;
        }
    } else {
        ImGui::TextUnformatted(term_log_.c_str());
    }
    
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

// --- End of Recording Helpers ---

std::string GuiLayer::OpenFileDialog() {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "CSV Files\0*.csv\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
#endif
    return "";
}

std::string GuiLayer::OpenIniFileDialog() {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "INI Files\0*.ini\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
#endif
    return "";
}

void GuiLayer::LoadCSV(OfflineSession& session) {
    std::ifstream file(session.filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV: " << session.filepath << std::endl;
        return;
    }

    session.data.time.clear();
    session.data.channels.clear();
    session.data.channel_names.clear();

    std::string line;
    // 1. Parse Header
    if (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::stringstream ss(line);
        std::string token;
        std::getline(ss, token, ','); // Time
        int ch_idx = 0;
        while (std::getline(ss, token, ',')) {
            session.data.channel_names[ch_idx] = token;
            session.data.channels[ch_idx] = std::vector<double>(); 
            ch_idx++;
        }
    }

    // 2. Parse Data
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        
        std::stringstream ss(line);
        std::string token;
        if (!std::getline(ss, token, ',')) continue;
        
        try {
            double t = std::stod(token);
            session.data.time.push_back(t);
            
            // Populate all channels for this time point
            for (auto& pair : session.data.channels) {
                if (std::getline(ss, token, ',') && !token.empty()) {
                    try {
                        pair.second.push_back(std::stod(token));
                    } catch (...) {
                        pair.second.push_back(0.0);
                    }
                } else {
                    pair.second.push_back(0.0); // Padding for missing data
                }
            }
        } catch (...) {}
    }
}

void GuiLayer::RenderOfflineViewer(OfflineSession& session) {
    char title[256];
    sprintf(title, "Offline Viewer [%d] - %s###Offline%d", session.id, session.filepath, session.id);
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    // Push unique color for each session to distinguish windows
    float hue = (float)(session.id % 5) * 0.1f; // Slight variations in blue-cyan range
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, (ImVec4)ImColor::HSV(0.6f + hue, 0.6f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, (ImVec4)ImColor::HSV(0.6f + hue, 0.4f, 0.2f));

    if (ImGui::Begin(title, &session.open)) {
        ImGui::PopStyleColor(2); // Pop early after Begin to not affect internal items
        if (session.data.time.size() > 0) {
            ImGui::Text("Points: %zu | Path: %s", session.data.time.size(), session.filepath);
            
            if (session.first_frame) {
                if (!session.data.time.empty()) {
                    session.x_min = session.data.time.front();
                    session.x_max = session.data.time.back();
                }
                session.first_frame = false;
            }

            if (ImPlot::BeginPlot("##OfflinePlot", ImVec2(-1, -1))) {
                // 1. Setup axis properties FIRST
                ImPlot::SetupAxes("Time(s)", "Value", 0, 0);
                ImPlot::SetupAxisLimits(ImAxis_X1, session.x_min, session.x_max, ImGuiCond_Always);
                
                // 2. Manual Zoom Logic (BEFORE plotting)
                if (ImPlot::IsPlotHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f) {
                    double mouse_x = ImPlot::GetPlotMousePos().x;
                    double zoom_factor = (ImGui::GetIO().MouseWheel > 0) ? 0.9 : 1.1;
                    
                    // Zoom centered at mouse_x
                    session.x_min = mouse_x - (mouse_x - session.x_min) * zoom_factor;
                    session.x_max = mouse_x + (session.x_max - mouse_x) * zoom_factor;
                }

                // 3. Pan Logic (Manual pan if needed, but ImPlot default pan is usually okay if we don't lock)
                // However, since we use ImGuiCond_Always, we need to capture dragging too:
                if (ImPlot::IsPlotHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    double drag_x = ImGui::GetIO().MouseDelta.x * (session.x_max - session.x_min) / ImPlot::GetPlotSize().x;
                    session.x_min -= drag_x;
                    session.x_max -= drag_x;
                }

                // 4. Plotting data
                for (const auto& kv : session.data.channels) {
                    int ch = kv.first;
                    std::string name = session.data.channel_names.count(ch) ? session.data.channel_names[ch] : "CH" + std::to_string(ch);
                    if (kv.second.size() == session.data.time.size()) {
                        ImPlot::PlotLine(name.c_str(), session.data.time.data(), kv.second.data(), (int)session.data.time.size());
                    }
                }

                // 5. Interaction & Measurement Logic
                if (ImPlot::IsPlotHovered()) {
                    ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                    ImVec2 mouse_pixel = ImPlot::PlotToPixels(mouse);
                    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
                    ImVec2 plot_min = ImPlot::GetPlotPos();
                    ImVec2 plot_max = ImVec2(plot_min.x + ImPlot::GetPlotSize().x, plot_min.y + ImPlot::GetPlotSize().y);
                    
                    // Toggle measurement on Space
                    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
                        session.measure_active = !session.measure_active;
                        session.measure_x = mouse.x;
                        session.measure_y = mouse.y;
                    }

                    // Helper for dashed lines
                    auto draw_dashed_h = [&](float x1, float x2, float y, ImU32 col) {
                        for (float x = x1; x < x2; x += 10.0f) draw_list->AddLine(ImVec2(x, y), ImVec2((x + 5.0f < x2 ? x + 5.0f : x2), y), col, 1.0f);
                    };
                    auto draw_dashed_v = [&](float x, float y1, float y2, ImU32 col) {
                        for (float y = y1; y < y2; y += 10.0f) draw_list->AddLine(ImVec2(x, y), ImVec2(x, (y + 5.0f < y2 ? y + 5.0f : y2)), col, 1.0f);
                    };

                    // Draw measurement reference (Red Dashed)
                    if (session.measure_active) {
                        ImVec2 ref_pixel = ImPlot::PlotToPixels(session.measure_x, session.measure_y);
                        draw_dashed_v(ref_pixel.x, plot_min.y, plot_max.y, IM_COL32(255, 0, 0, 150));
                        draw_dashed_h(plot_min.x, plot_max.x, ref_pixel.y, IM_COL32(255, 0, 0, 150));
                        draw_list->AddRectFilled(ref_pixel, mouse_pixel, IM_COL32(255, 0, 0, 30));
                    }

                    // Current cursor (Yellow Dashed)
                    draw_dashed_v(mouse_pixel.x, plot_min.y, plot_max.y, IM_COL32(255, 255, 0, 100));
                    draw_dashed_h(plot_min.x, plot_max.x, mouse_pixel.y, IM_COL32(255, 255, 0, 100));
                    
                    ImGui::BeginTooltip();
                    ImGui::Text("Current X: %.4f s", mouse.x);
                    ImGui::Text("Current Y: %.2f", mouse.y);
                    if (session.measure_active) {
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Ref X: %.4f s", session.measure_x);
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Ref Y: %.2f", session.measure_y);
                        ImGui::TextColored(ImVec4(0, 1, 1, 1), "dX: %.4f s (%.2f Hz)", mouse.x - session.measure_x, 1.0 / (mouse.x - session.measure_x));
                        ImGui::TextColored(ImVec4(0, 1, 1, 1), "dY: %.2f", mouse.y - session.measure_y);
                    } else {
                        ImGui::TextDisabled("(Press Space to measure)");
                    }
                    ImGui::EndTooltip();
                }

                ImPlot::EndPlot();
            }
        } else {
            ImGui::Text("Failed to load or file empty.");
        }
    }
    ImGui::End();
}
