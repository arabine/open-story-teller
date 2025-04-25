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

    enum RandomFlags
    {
        CHARSET_ALPHABET_LOWER = 0x1, // "abcdefghijklmnopqrstuvwxyz"
        CHARSET_ALPHABET_UPPER = 0x2, // "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        CHARSET_NUMBERS = 0x4, // "0123456789"
        CHARSET_SIGNS = 0x8, // "!@#$%^&*()_+-=[]{}|;:,.<>?";
        ALL_CHARSETS = CHARSET_ALPHABET_LOWER | CHARSET_ALPHABET_UPPER |CHARSET_NUMBERS | CHARSET_SIGNS
    };

    static const int NameMaxSize = 32; // Max size for the variable name

    Variable() {
        m_uuid = Uuid().String();
        m_label = Variable::GenerateRandomString(10, Variable::CHARSET_ALPHABET_LOWER | Variable::CHARSET_ALPHABET_UPPER );
    }

    Variable (const std::string &name)
        : Variable()
    {
        m_variableName = name;
    }

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

    void SetScalePower(int scalePower) {
        m_scalePower = scalePower;
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

    std::string GetLabel() const { 
        return m_label; 
    }

    template<typename T>
    T GetValue() const {
        try {
            return std::get<T>(m_value);
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error("[variable.h] GetValue(): Invalid value type requested");
        }
    }

    std::string GetStringValue() const {
        return GetValue<std::string>();
    }

    int GetIntegerValue() const {
        return GetValue<int>();
    }

    float GetFloatValue() const {
        return GetValue<float>();
    }

    bool GetBoolValue() const {
        return GetValue<bool>();
    }

    using VariableValue = std::variant<int, float, bool, std::string>;

    std::string GetUuid() const { 
        return m_uuid; 
    }

    bool IsString() const {
        return m_valueType == ValueType::STRING;
    }
    bool IsInteger() const {
        return m_valueType == ValueType::INTEGER;
    }
    bool IsFloat() const {
        return m_valueType == ValueType::FLOAT;
    }
    bool IsBool() const {
        return m_valueType == ValueType::BOOL;
    }

    int GetScalePower() const {
        return m_scalePower;
    }

    static std::string GenerateRandomString(size_t length, uint32_t flags) 
    {
        std::string charset = "";

        if (flags & CHARSET_ALPHABET_LOWER)
        {
            charset += "abcdefghijklmnopqrstuvwxyz";
        }

        if (flags & CHARSET_ALPHABET_UPPER)
        {
            charset += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        }

        if (flags & CHARSET_NUMBERS)
        {
            charset += "0123456789";
        }

        if (flags & CHARSET_SIGNS)
        {
            charset += "!@#$%^&*()_+-=[]{}|;:,.<>?";
        }

        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, charset.size() - 1);

        std::string result;
        result.reserve(length);

        for (size_t i = 0; i < length; ++i) {
            result += charset[distribution(generator)];
        }

        return result;
    }

    static std::string ValueTypeToString(ValueType type) {
        switch (type) {
            case ValueType::INTEGER: return "Integer";
            case ValueType::FLOAT: return "Float";
            case ValueType::BOOL: return "Bool";
            case ValueType::STRING: return "String";
            default: return "Unknown";
        }
    }

    static ValueType StringToValueType(const std::string& type) {
        if (type == "Integer") return ValueType::INTEGER;
        if (type == "Float") return ValueType::FLOAT;
        if (type == "Bool") return ValueType::BOOL;
        if (type == "String") return ValueType::STRING;
        throw std::runtime_error("[variable.h] StringToValueType(): Invalid value type string");
    }

private:
    std::string m_variableName; // nom humain
    ValueType m_valueType;
    VariableValue m_value;
    bool m_isConstant;
    std::string m_uuid; // pour identifier le variable dans le JSON du projet
    std::string m_label; // pour la génération assembleur
    int m_scalePower;    // Nombre de bits pour la partie fractionnaire

};
