#pragma once

#include <string>
#include "base_node.h"

class ForLoopNode : public BaseNode
{
public:
    ForLoopNode(const std::string &type = "for-loop-node");

    void Initialize() override;

    // Propriétés configurables (valeurs par défaut si non connectées)
    int GetStartIndex() const { return m_startIndex; }
    void SetStartIndex(int value) { 
        m_startIndex = value;
        nlohmann::json j = GetInternalData();
        j["start_index"] = m_startIndex;
        SetInternalData(j);
    }
    
    int GetEndIndex() const { return m_endIndex; }
    void SetEndIndex(int value) { 
        m_endIndex = value;
        nlohmann::json j = GetInternalData();
        j["end_index"] = m_endIndex;
        SetInternalData(j);
    }
    
    int GetStep() const { return m_step; }
    void SetStep(int value) { 
        m_step = value;
        nlohmann::json j = GetInternalData();
        j["step"] = m_step;
        SetInternalData(j);
    }

private:
    int m_startIndex{0};
    int m_endIndex{10};
    int m_step{1};
};