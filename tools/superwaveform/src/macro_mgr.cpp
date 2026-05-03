#include "macro_mgr.h"
#include <fstream>
#include <cctype>

// Trim leading/trailing whitespace (space, \t, \r)
std::string MacroManager::Trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace((unsigned char)*start)) ++start;
    auto end = s.end();
    while (end != start && std::isspace((unsigned char)*(end - 1))) --end;
    return std::string(start, end);
}

bool MacroManager::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    macros_.clear();
    std::string line;
    while (std::getline(file, line)) {
        // Strip Windows \r
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::string trimmed = Trim(line);

        // Skip empty lines and comments
        if (trimmed.empty() || trimmed[0] == '#') continue;

        // Split on first '=' only (command may contain '=')
        auto eq_pos = trimmed.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string label   = Trim(trimmed.substr(0, eq_pos));
        std::string command = Trim(trimmed.substr(eq_pos + 1));

        if (label.empty() || command.empty()) continue;

        macros_.push_back({label, command});
    }

    current_path_ = path;
    return true;
}

void MacroManager::RestoreLastSession() {
    std::ifstream cfg(kConfigFile);
    if (!cfg.is_open()) return;

    std::string line;
    while (std::getline(cfg, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        auto eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = Trim(line.substr(0, eq_pos));
        std::string val = Trim(line.substr(eq_pos + 1));

        if (key == kConfigKey && !val.empty()) {
            LoadFromFile(val); // Silently skip if file no longer exists
            return;
        }
    }
}

void MacroManager::PersistCurrentPath() const {
    if (current_path_.empty()) return;
    std::ofstream cfg(kConfigFile);
    if (!cfg.is_open()) return;
    cfg << kConfigKey << "=" << current_path_ << "\n";
}
