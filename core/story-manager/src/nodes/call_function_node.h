// call_function_node.h
#pragma once

#include "base_node.h"

class CallFunctionNode : public BaseNode
{
public:
    CallFunctionNode(const std::string &type)
        : BaseNode(type, "Call Function Node")
        , m_functionName("")
        , m_functionUuid("")
    {
        SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
        SetupExecutionPorts(true, 1, true);  // 1 entrée, 1 sortie
    }

    void Initialize() override {
        // Charger le nom et l'UUID de la fonction depuis les données internes
        nlohmann::json j = GetInternalData();
        if (j.contains("functionName")) {
            m_functionName = j["functionName"].get<std::string>();
        }
        if (j.contains("functionUuid")) {
            m_functionUuid = j["functionUuid"].get<std::string>();
        }
    }

    std::string GetFunctionName() const { 
        return m_functionName; 
    }
    
    std::string GetFunctionUuid() const {
        return m_functionUuid;
    }
    
    void SetFunction(const std::string& uuid, const std::string& name) { 
        m_functionUuid = uuid;
        m_functionName = name;
        
        // Sauvegarder dans les données internes pour la persistance
        nlohmann::json j;
        j["functionName"] = m_functionName;
        j["functionUuid"] = m_functionUuid;
        SetInternalData(j);
    }

private:
    std::string m_functionName;
    std::string m_functionUuid;
};