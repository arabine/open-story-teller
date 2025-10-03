#pragma once

#include "window_base.h"
#include "IconsFontAwesome5_c.h"
#include <vector>
#include <string>
#include <algorithm>

struct CompilationError {
    enum Type {
        ERROR,
        WARNING,
        INFO
    };
    
    Type type;
    std::string message;
    std::string nodeId;  // UUID du nœud concerné
    int line;
    
    std::string GetTypeIcon() const {
        switch(type) {
            case ERROR: return ICON_FA_TIMES_CIRCLE;
            case WARNING: return ICON_FA_EXCLAMATION_TRIANGLE;
            case INFO: return ICON_FA_INFO_CIRCLE;
        }
        return "";
    }
    
    ImVec4 GetTypeColor() const {
        switch(type) {
            case ERROR: return ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
            case WARNING: return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
            case INFO: return ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
        }
        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
};

class ErrorListDock : public WindowBase {
public:
    ErrorListDock() : WindowBase("Error List") {}
    
    void Draw() override;
    
    void Clear() { 
        m_errors.clear(); 
        m_shouldShow = false; // Hide when cleared
    }
    
    void AddError(const CompilationError& error) { 
        m_errors.push_back(error);
        m_shouldShow = true; // Show when error added
    }
    
    void AddError(const std::string& message, const std::string& nodeId = "", int line = 0) {
        m_errors.push_back({CompilationError::ERROR, message, nodeId, line});
        m_shouldShow = true;
    }
    
    void AddWarning(const std::string& message, const std::string& nodeId = "", int line = 0) {
        m_errors.push_back({CompilationError::WARNING, message, nodeId, line});
        m_shouldShow = true;
    }
    
    void AddInfo(const std::string& message, const std::string& nodeId = "", int line = 0) {
        m_errors.push_back({CompilationError::INFO, message, nodeId, line});
    }
    
    bool HasErrors() const { 
        return std::any_of(m_errors.begin(), m_errors.end(),
            [](const auto& e) { return e.type == CompilationError::ERROR; });
    }
    
    size_t GetErrorCount() const { 
        return std::count_if(m_errors.begin(), m_errors.end(), 
            [](const auto& e) { return e.type == CompilationError::ERROR; });
    }
    
    size_t GetWarningCount() const {
        return std::count_if(m_errors.begin(), m_errors.end(),
            [](const auto& e) { return e.type == CompilationError::WARNING; });
    }
    
    // Force the window to be visible (used when errors occur)
    void Show() { 
        m_shouldShow = true;
        Enable();
    }
    
private:
    std::vector<CompilationError> m_errors;
    bool m_shouldShow = false;
};