#pragma once

#include "base_node.h"
#include <sstream>
#include <stdexcept>
#include <unordered_map>

class OperatorNode : public BaseNode
{
public:
    enum class OperationType {
        ADD,            // Addition (+)
        SUBTRACT,       // Subtraction (-)
        MULTIPLY,       // Multiplication (*)
        DIVIDE,        // Division (/)
        MODULO,        // Modulo (%)
        AND,           // Logical AND (&&)
        OR,            // Logical OR (||)
        NOT,           // Logical NOT (!)
        XOR,           // Logical XOR (^)
        EQUAL,         // Equal to (==)
        NOT_EQUAL,     // Not equal to (!=)
        GREATER_THAN,  // Greater than (>)
        LESS_THAN,     // Less than (<)
        GREATER_EQUAL, // Greater than or equal (>=)
        LESS_EQUAL,    // Less than or equal (<=)
        BITWISE_AND,   // Bitwise AND (&)
        BITWISE_OR,    // Bitwise OR (|)
        BITWISE_XOR,   // Bitwise XOR (^)
        BITWISE_NOT,   // Bitwise NOT (~)
        LEFT_SHIFT,    // Left shift (<<)
        RIGHT_SHIFT    // Right shift (>>)
    };

    struct OperatorDesc {
        std::string name;
        std::string symbol;
    };

    const std::unordered_map<OperationType, OperatorDesc>& GetOperators() const
    {
        static const std::unordered_map<OperationType, OperatorDesc> Operators = {
            {OperationType::ADD, {"Addition", "+"}},
            {OperationType::SUBTRACT, {"Subtraction", "-"}},
            {OperationType::MULTIPLY, {"Multiplication", "*"}},
            {OperationType::DIVIDE, {"Division", "/"}},
            {OperationType::MODULO, {"Modulo", "%"}},
            {OperationType::AND, {"Logical AND", "&&"}},
            {OperationType::OR, {"Logical OR", "||"}},
            {OperationType::NOT, {"Logical NOT", "!"}},
            {OperationType::XOR, {"Logical XOR", "^"}},
            {OperationType::EQUAL, {"Equal to", "=="}},
            {OperationType::NOT_EQUAL, {"Not equal to", "!="}},
            {OperationType::GREATER_THAN, {"Greater than", ">"}},
            {OperationType::LESS_THAN, {"Less than", "<"}},
            {OperationType::GREATER_EQUAL, {"Greater than or equal to", ">="}},
            {OperationType::LESS_EQUAL, {"Less than or equal to", "<="}},
            {OperationType::BITWISE_AND, {"Bitwise AND", "&"}},
            {OperationType::BITWISE_OR, {"Bitwise OR", "|"}},
            {OperationType::BITWISE_XOR, {"Bitwise XOR", "^"}},
            {OperationType::BITWISE_NOT, {"Bitwise NOT", "~"}},
            {OperationType::LEFT_SHIFT, {"Left shift", "<<"}},
            {OperationType::RIGHT_SHIFT, {"Right shift", ">>"}}
        };
        return Operators;
    }

    OperatorNode(const std::string& type = "operator-node",
                 const std::string& typeName = "Operator")
        : BaseNode(type, typeName, BaseNode::Behavior::BEHAVIOR_DATA)
        , m_operationType(OperationType::ADD)
    {
        Initialize();
    }

    void Initialize() override {
        // Define input ports based on operator type
        UpdatePorts();
    }

    // Set the operator type and update ports accordingly
    void SetOperationType(OperationType type) {
        m_operationType = type;
        UpdatePorts();
    }

    OperationType GetOperationType() const {
        return m_operationType;
    }

    // Get the symbol representation of the operator
    std::string GetOperatorSymbol() const {
        auto symbolMap = GetOperators();
        auto it = symbolMap.find(m_operationType);
        return it != symbolMap.end() ? it->second.symbol : "?";
    }

private:
    OperationType m_operationType;

    bool IsUnaryOperator() const {
        return m_operationType == OperationType::NOT ||
               m_operationType == OperationType::BITWISE_NOT;
    }

    void UpdatePorts() {
        // Clear existing ports
        ClearPorts();

        // Add input ports based on operator type
        if (IsUnaryOperator()) {
            AddInputPort(Port::DATA_PORT, "in");
        } else {
            AddInputPort(Port::DATA_PORT, "in1");
            AddInputPort(Port::DATA_PORT, "in2");
        }

        // Add output port
        AddOutputPort(Port::DATA_PORT, "out");
    }

    std::string OperatorTypeName(OperationType type) const {
        auto symbolMap = GetOperators();
        auto it = symbolMap.find(type);
        return it != symbolMap.end() ? it->second.name : "Unknown";
    }

    OperationType SymbolToOperationType(const std::string& str) const {
        auto symbolMap = GetOperators();

        for (const auto& pair : symbolMap) {
            if (pair.second.symbol == str) {
                return pair.first;
            }
        }
        return OperationType::ADD; // Default to ADD if not found
    }
};
