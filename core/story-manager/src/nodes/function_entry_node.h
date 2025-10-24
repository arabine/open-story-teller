#pragma once

#include "base_node.h"
#include <vector>

class FunctionEntryNode : public BaseNode
{
public:
    struct Parameter {
        std::string name;
        std::string type;  // "int", "string", "bool", "float"
        std::string defaultValue;
        
        nlohmann::json ToJson() const {
            return {
                {"name", name},
                {"type", type},
                {"defaultValue", defaultValue}
            };
        }
        
        static Parameter FromJson(const nlohmann::json& j) {
            Parameter p;
            p.name = j.value("name", "");
            p.type = j.value("type", "int");
            p.defaultValue = j.value("defaultValue", "");
            return p;
        }
    };

    FunctionEntryNode(const std::string &type)
        : BaseNode(type, "Function Entry Node")
    {
        SetWeight(100);
        SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
        SetupExecutionPorts(false, 1, true);
    }

    void Initialize() override {
        // Load parameters from internal data
        nlohmann::json j = GetInternalData();
        m_parameters.clear();
        
        if (j.contains("parameters") && j["parameters"].is_array()) {
            for (const auto& paramJson : j["parameters"]) {
                m_parameters.push_back(Parameter::FromJson(paramJson));
            }
        }
        
        // Rebuild output ports for parameters
        RebuildParameterPorts();
    }

    void AddParameter(const std::string& name, const std::string& type, const std::string& defaultValue = "") {
        Parameter param;
        param.name = name;
        param.type = type;
        param.defaultValue = defaultValue;
        m_parameters.push_back(param);
        
        SaveParameters();
        RebuildParameterPorts();
    }
    
    void RemoveParameter(size_t index) {
        if (index < m_parameters.size()) {
            m_parameters.erase(m_parameters.begin() + index);
            SaveParameters();
            RebuildParameterPorts();
        }
    }
    
    void UpdateParameter(size_t index, const std::string& name, const std::string& type, const std::string& defaultValue) {
        if (index < m_parameters.size()) {
            m_parameters[index].name = name;
            m_parameters[index].type = type;
            m_parameters[index].defaultValue = defaultValue;
            SaveParameters();
            RebuildParameterPorts();
        }
    }
    
    const std::vector<Parameter>& GetParameters() const {
        return m_parameters;
    }
    
    size_t GetParameterCount() const {
        return m_parameters.size();
    }

private:
    std::vector<Parameter> m_parameters;
    
    void SaveParameters() {
        nlohmann::json j;
        j["parameters"] = nlohmann::json::array();
        
        for (const auto& param : m_parameters) {
            j["parameters"].push_back(param.ToJson());
        }
        
        SetInternalData(j);
    }
    
    void RebuildParameterPorts() {
        // Clear all data output ports (keep execution port)
        ClearDataOutputPorts();
        
        // Add a data output port for each parameter
        for (const auto& param : m_parameters) {
            AddOutputPort(Port::Type::DATA_PORT, param.name, false);
        }
    }
};