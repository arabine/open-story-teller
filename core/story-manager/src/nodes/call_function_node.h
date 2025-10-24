// call_function_node.h
#pragma once

#include "base_node.h"
#include <map>

class CallFunctionNode : public BaseNode
{
public:
    enum InputBindingMode {
        MODE_CONNECTED,  // Value comes from a connected pin
        MODE_CONSTANT    // Value is a constant set in the UI
    };

    struct InputBinding {
        std::string paramName;
        InputBindingMode mode;
        std::string constantValue;
        
        nlohmann::json ToJson() const {
            return {
                {"paramName", paramName},
                {"mode", mode == MODE_CONNECTED ? "connected" : "constant"},
                {"constantValue", constantValue}
            };
        }
        
        static InputBinding FromJson(const nlohmann::json& j) {
            InputBinding ib;
            ib.paramName = j.value("paramName", "");
            std::string modeStr = j.value("mode", "connected");
            ib.mode = (modeStr == "constant") ? MODE_CONSTANT : MODE_CONNECTED;
            ib.constantValue = j.value("constantValue", "");
            return ib;
        }
    };

    struct OutputMapping {
        std::string returnValueName;
        std::string targetVariable;  // Optional: map to a global variable
        
        nlohmann::json ToJson() const {
            return {
                {"returnValueName", returnValueName},
                {"targetVariable", targetVariable}
            };
        }
        
        static OutputMapping FromJson(const nlohmann::json& j) {
            OutputMapping om;
            om.returnValueName = j.value("returnValueName", "");
            om.targetVariable = j.value("targetVariable", "");
            return om;
        }
    };

    CallFunctionNode(const std::string &type)
        : BaseNode(type, "Call Function Node")
        , m_functionName("")
        , m_functionUuid("")
    {
        SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
        SetupExecutionPorts(true, 1, true);  // 1 entrée, sorties dynamiques
    }

    void Initialize() override {
        nlohmann::json j = GetInternalData();
        
        if (j.contains("functionName")) {
            m_functionName = j["functionName"].get<std::string>();
        }
        if (j.contains("functionUuid")) {
            m_functionUuid = j["functionUuid"].get<std::string>();
        }
        
        // Load input bindings
        m_inputBindings.clear();
        if (j.contains("inputBindings") && j["inputBindings"].is_array()) {
            for (const auto& ibJson : j["inputBindings"]) {
                m_inputBindings.push_back(InputBinding::FromJson(ibJson));
            }
        }
        
        // Load output mappings
        m_outputMappings.clear();
        if (j.contains("outputMappings") && j["outputMappings"].is_array()) {
            for (const auto& omJson : j["outputMappings"]) {
                m_outputMappings.push_back(OutputMapping::FromJson(omJson));
            }
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
        SaveData();
    }
    
    // Input bindings management
    const std::vector<InputBinding>& GetInputBindings() const {
        return m_inputBindings;
    }
    
    void SetInputBindingMode(const std::string& paramName, InputBindingMode mode, const std::string& constantValue = "") {
        // Find or create binding
        auto it = std::find_if(m_inputBindings.begin(), m_inputBindings.end(),
            [&paramName](const InputBinding& ib) { return ib.paramName == paramName; });
        
        if (it != m_inputBindings.end()) {
            it->mode = mode;
            it->constantValue = constantValue;
        } else {
            InputBinding ib;
            ib.paramName = paramName;
            ib.mode = mode;
            ib.constantValue = constantValue;
            m_inputBindings.push_back(ib);
        }
        
        SaveData();
    }
    
    InputBinding* GetInputBinding(const std::string& paramName) {
        auto it = std::find_if(m_inputBindings.begin(), m_inputBindings.end(),
            [&paramName](const InputBinding& ib) { return ib.paramName == paramName; });
        
        return (it != m_inputBindings.end()) ? &(*it) : nullptr;
    }
    
    // Output mappings management
    const std::vector<OutputMapping>& GetOutputMappings() const {
        return m_outputMappings;
    }
    
    void SetOutputMapping(const std::string& returnValueName, const std::string& targetVariable) {
        auto it = std::find_if(m_outputMappings.begin(), m_outputMappings.end(),
            [&returnValueName](const OutputMapping& om) { return om.returnValueName == returnValueName; });
        
        if (it != m_outputMappings.end()) {
            it->targetVariable = targetVariable;
        } else {
            OutputMapping om;
            om.returnValueName = returnValueName;
            om.targetVariable = targetVariable;
            m_outputMappings.push_back(om);
        }
        
        SaveData();
    }
    
    // Rebuild ports based on the module's interface
    void RebuildPortsFromModule(const std::vector<std::pair<std::string, std::string>>& parameters,
                                 const std::vector<std::pair<std::string, std::string>>& exitLabels,
                                 const std::map<std::string, std::vector<std::pair<std::string, std::string>>>& returnValuesByExit) {
        // Clear all ports except the main execution input
        ClearDataInputPorts();
        ClearAllOutputPorts();
        
        // Add data input ports for each parameter
        for (const auto& param : parameters) {
            AddInputPort(Port::Type::DATA_PORT, param.first, false);
        }
        
        // Add execution output ports for each exit
        for (const auto& exitLabel : exitLabels) {
            AddOutputPort(Port::Type::EXECUTION_PORT, exitLabel.first, true);
        }
        
        // Add data output ports for all unique return values across all exits
        std::set<std::string> allReturnValues;
        for (const auto& exitPair : returnValuesByExit) {
            for (const auto& rv : exitPair.second) {
                allReturnValues.insert(rv.first);
            }
        }
        
        for (const auto& rvName : allReturnValues) {
            AddOutputPort(Port::Type::DATA_PORT, rvName, false);
        }
    }

private:
    std::string m_functionName;
    std::string m_functionUuid;
    std::vector<InputBinding> m_inputBindings;
    std::vector<OutputMapping> m_outputMappings;
    
    void SaveData() {
        nlohmann::json j;
        j["functionName"] = m_functionName;
        j["functionUuid"] = m_functionUuid;
        
        j["inputBindings"] = nlohmann::json::array();
        for (const auto& ib : m_inputBindings) {
            j["inputBindings"].push_back(ib.ToJson());
        }
        
        j["outputMappings"] = nlohmann::json::array();
        for (const auto& om : m_outputMappings) {
            j["outputMappings"].push_back(om.ToJson());
        }
        
        SetInternalData(j);
    }
};