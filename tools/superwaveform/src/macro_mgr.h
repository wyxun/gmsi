#ifndef MACRO_MGR_H
#define MACRO_MGR_H

#include <string>
#include <vector>

struct MacroEntry {
    std::string label;   // Button display text
    std::string command; // Command to send to Ch0 (without trailing \n)
};

class MacroManager {
public:
    // Load macros from INI file, returns true on success
    bool LoadFromFile(const std::string& path);

    // Get current macro list (read-only)
    const std::vector<MacroEntry>& GetMacros() const { return macros_; }

    // Get current loaded file path (empty string = not loaded)
    const std::string& GetCurrentPath() const { return current_path_; }

    // Restore last session from superwaveform.cfg, silently skip on failure
    void RestoreLastSession();

    // Persist current path to superwaveform.cfg
    void PersistCurrentPath() const;

private:
    std::vector<MacroEntry> macros_;
    std::string current_path_;

    static constexpr const char* kConfigFile = "superwaveform.cfg";
    static constexpr const char* kConfigKey  = "last_macro_file";

    // Trim leading/trailing whitespace
    static std::string Trim(const std::string& s);
};

#endif // MACRO_MGR_H
