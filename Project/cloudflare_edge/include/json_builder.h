#pragma once
#include <string>

// ─────────────────────────────────────────────
// Lightweight JSON builder
// ─────────────────────────────────────────────
class JsonBuilder {
public:
    JsonBuilder() { buf_ = "{"; }

    JsonBuilder& add(const std::string& key, const std::string& val) {
        comma(); buf_ += "\"" + key + "\":\"" + escape(val) + "\"";
        return *this;
    }
    JsonBuilder& add(const std::string& key, int val) {
        comma(); buf_ += "\"" + key + "\":" + std::to_string(val);
        return *this;
    }
    JsonBuilder& add(const std::string& key, bool val) {
        comma(); buf_ += "\"" + key + "\":" + (val ? "true" : "false");
        return *this;
    }
    JsonBuilder& addRaw(const std::string& key, const std::string& raw) {
        comma(); buf_ += "\"" + key + "\":" + raw;
        return *this;
    }

    std::string build() { return buf_ + "}"; }

private:
    std::string buf_;
    bool first_ = true;

    void comma() { if (!first_) buf_ += ","; first_ = false; }

    std::string escape(const std::string& s) {
        std::string out;
        for (char c : s) {
            if (c == '"')  out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else out += c;
        }
        return out;
    }
};
