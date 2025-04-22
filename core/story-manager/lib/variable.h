#pragma once

#include <string>
#include <cstdint>
#include <variant>

#include "uuid.h"

class Variable
{

public:
    enum class ValueType {
        INTEGER,
        FLOAT,
        BOOL,
        STRING
    };


    // Setters

    void SetUuid(const std::string& uuid) { 
        m_uuid = uuid; 
    }

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
            throw std::runtime_error("[variable.h] SetValue(): Invalid value type for variable");
        }
    }

    void SetTextValue(const std::string& value) {
        SetValue<std::string>(value);
        m_valueType = ValueType::STRING;
    }

    void SetIntegerValue(int value) {
        SetValue<int>(value);
        m_valueType = ValueType::INTEGER;
    }

    void SetFloatValue(float value) {
        SetValue<float>(value);
        m_valueType = ValueType::FLOAT;
    }

    void SetBoolValue(bool value) {
        SetValue<bool>(value);
        m_valueType = ValueType::BOOL;
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
            throw std::runtime_error("[variable.h] GetValue(): Invalid value type requested");
        }
    }

    using VariableValue = std::variant<int, float, bool, std::string>;

    std::string GetUuid() const { 
        return m_uuid; 
    }

    Variable() {
        m_uuid = Uuid().String();
    }

    Variable (const std::string &name)
        : Variable()
    {
        m_variableName = name;
    }

private:
    std::string m_variableName;
    ValueType m_valueType;
    VariableValue m_value;
    bool m_isConstant;
    std::string m_uuid;
    int m_scalePower;    // Nombre de bits pour la partie fractionnaire

};
