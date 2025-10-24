#pragma once

#include "base_node.h"
#include <vector>

class FunctionExitNode : public BaseNode
{
public:
    struct ReturnValue {
        std::string name;
        std::string type;  // "int", "string", "bool", "float"
        
        nlohmann::json ToJson() const {
            return {
                {"name", name},
                {"type", type}
            };
        }
        
        static ReturnValue FromJson(const nlohmann::json& j) {
            ReturnValue rv;
            rv.name = j.value("name", "");
            rv.type = j.value("type", "int");
            return rv;
        }
    };

    FunctionExitNode(const std::string &type)
        : BaseNode(type, "Function Exit Node")
        , m_exitLabel("Return")
    {
        SetWeight(900); // High weight, near the end
        SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
        SetupExecutionPorts(true, 0, true); // Has input, no output (it's the end)
    }

    void Initialize() override {
        // Load return values and exit label from internal data
        nlohmann::json j = GetInternalData();
        m_returnValues.clear();
        
        m_exitLabel = j.value("exitLabel", "Return");
        
        if (j.contains("returnValues") && j["returnValues"].is_array()) {
            for (const auto& rvJson : j["returnValues"]) {
                m_returnValues.push_back(ReturnValue::FromJson(rvJson));
            }
        }
        
        // Rebuild input ports for return values
        RebuildReturnValuePorts();
    }

    void SetExitLabel(const std::string& label) {
        m_exitLabel = label;
        SaveData();
    }
    
    std::string GetExitLabel() const {
        return m_exitLabel;
    }

    void AddReturnValue(const std::string& name, const std::string& type) {
        ReturnValue rv;
        rv.name = name;
        rv.type = type;
        m_returnValues.push_back(rv);
        
        SaveData();
        RebuildReturnValuePorts();
    }
    
    void RemoveReturnValue(size_t index) {
        if (index < m_returnValues.size()) {
            m_returnValues.erase(m_returnValues.begin() + index);
            SaveData();
            RebuildReturnValuePorts();
        }
    }
    
    void UpdateReturnValue(size_t index, const std::string& name, const std::string& type) {
        if (index < m_returnValues.size()) {
            m_returnValues[index].name = name;
            m_returnValues[index].type = type;
            SaveData();
            RebuildReturnValuePorts();
        }
    }
    
    const std::vector<ReturnValue>& GetReturnValues() const {
        return m_returnValues;
    }
    
    size_t GetReturnValueCount() const {
        return m_returnValues.size();
    }

private:
    std::string m_exitLabel;
    std::vector<ReturnValue> m_returnValues;
    
    void SaveData() {
        nlohmann::json j;
        j["exitLabel"] = m_exitLabel;
        j["returnValues"] = nlohmann::json::array();
        
        for (const auto& rv : m_returnValues) {
            j["returnValues"].push_back(rv.ToJson());
        }
        
        SetInternalData(j);
    }
    
    void RebuildReturnValuePorts() {
        // Clear all data input ports (keep execution port)
        ClearDataInputPorts();
        
        // Add a data input port for each return value
        for (const auto& rv : m_returnValues) {
            AddInputPort(Port::Type::DATA_PORT, rv.name, false);
        }
    }
};