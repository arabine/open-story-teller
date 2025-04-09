#pragma once

#include <string>
#include <variant>

#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class VariableNode : public BaseNode
{
public:

    enum class ValueType {
        INTEGER,
        FLOAT,
        BOOL,
        STRING
    };

    using VariableValue = std::variant<int, float, bool, std::string>;

    VariableNode(const std::string &type = "variable-node");

    virtual void Initialize() override;
    virtual std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override;
    virtual std::string GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns) override;

    virtual std::string GenerateAssembly() const { return ""; }

    void StoreInternalData();


    // Setters
    void SetVariableName(const std::string& name) { 
        m_variableName = name; 
    }

    void SetConstant(bool isConstant) { 
        m_isConstant = isConstant; 
    }

    void SetValueType(ValueType type) { 
        m_valueType = type;
        // Reset value to default for new type
        switch (type) {
            case ValueType::INTEGER:
                m_value = 0;
                break;
            case ValueType::FLOAT:
                m_value = 0.0f;
                break;
            case ValueType::BOOL:
                m_value = false;
                break;
            case ValueType::STRING:
                m_value = "";
                break;
        }
    }

    template<typename T>
    void SetValue(const T& value) {
        try {
            m_value = value;
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error("Invalid value type for variable");
        }
    }

    // Getters
    std::string GetVariableName() const { 
        return m_variableName; 
    }

    bool IsConstant() const { 
        return m_isConstant; 
    }

    ValueType GetValueType() const { 
        return m_valueType; 
    }

    template<typename T>
    T GetValue() const {
        try {
            return std::get<T>(m_value);
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error("Invalid value type requested");
        }
    }



private:
    std::string m_variableName;
    ValueType m_valueType;
    VariableValue m_value;
    bool m_isConstant;

};

