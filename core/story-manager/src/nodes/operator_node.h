#pragma once

#include "base_node.h"

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
        static const std::unordered_map<OperationType, std::string> symbolMap = {
            {OperationType::ADD, "+"},
            {OperationType::SUBTRACT, "-"},
            {OperationType::MULTIPLY, "*"},
            {OperationType::DIVIDE, "/"},
            {OperationType::MODULO, "%"},
            {OperationType::AND, "&&"},
            {OperationType::OR, "||"},
            {OperationType::NOT, "!"},
            {OperationType::XOR, "^"},
            {OperationType::EQUAL, "=="},
            {OperationType::NOT_EQUAL, "!="},
            {OperationType::GREATER_THAN, ">"},
            {OperationType::LESS_THAN, "<"},
            {OperationType::GREATER_EQUAL, ">="},
            {OperationType::LESS_EQUAL, "<="},
            {OperationType::BITWISE_AND, "&"},
            {OperationType::BITWISE_OR, "|"},
            {OperationType::BITWISE_XOR, "^"},
            {OperationType::BITWISE_NOT, "~"},
            {OperationType::LEFT_SHIFT, "<<"},
            {OperationType::RIGHT_SHIFT, ">>"}
        };

        auto it = symbolMap.find(m_operationType);
        return it != symbolMap.end() ? it->second : "?";
    }

    std::string Build(IStoryPage& page, const StoryOptions& options, int nb_out_conns) override {
        std::stringstream ss;
        ss << "// Operator: " << GetOperatorSymbol() << "\n";

        // For unary operators
        if (IsUnaryOperator()) {
            ss << GetOperatorSymbol() << "operand";
        }
        // For binary operators
        else {
            ss << "operand1 " << GetOperatorSymbol() << " operand2";
        }

        return ss.str();
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

    static std::string OperatorTypeToString(OperationType type) {
        static const std::unordered_map<OperationType, std::string> typeMap = {
            {OperationType::ADD, "add"},
            {OperationType::SUBTRACT, "subtract"},
            {OperationType::MULTIPLY, "multiply"},
            {OperationType::DIVIDE, "divide"},
            {OperationType::MODULO, "modulo"},
            {OperationType::AND, "and"},
            {OperationType::OR, "or"},
            {OperationType::NOT, "not"},
            {OperationType::XOR, "xor"},
            {OperationType::EQUAL, "equal"},
            {OperationType::NOT_EQUAL, "not_equal"},
            {OperationType::GREATER_THAN, "greater_than"},
            {OperationType::LESS_THAN, "less_than"},
            {OperationType::GREATER_EQUAL, "greater_equal"},
            {OperationType::LESS_EQUAL, "less_equal"},
            {OperationType::BITWISE_AND, "bitwise_and"},
            {OperationType::BITWISE_OR, "bitwise_or"},
            {OperationType::BITWISE_XOR, "bitwise_xor"},
            {OperationType::BITWISE_NOT, "bitwise_not"},
            {OperationType::LEFT_SHIFT, "left_shift"},
            {OperationType::RIGHT_SHIFT, "right_shift"}
        };

        auto it = typeMap.find(type);
        return it != typeMap.end() ? it->second : "unknown";
    }

    static OperationType StringToOperationType(const std::string& str) {
        static const std::unordered_map<std::string, OperationType> typeMap = {
            {"add", OperationType::ADD},
            {"subtract", OperationType::SUBTRACT},
            {"multiply", OperationType::MULTIPLY},
            {"divide", OperationType::DIVIDE},
            {"modulo", OperationType::MODULO},
            {"and", OperationType::AND},
            {"or", OperationType::OR},
            {"not", OperationType::NOT},
            {"xor", OperationType::XOR},
            {"equal", OperationType::EQUAL},
            {"not_equal", OperationType::NOT_EQUAL},
            {"greater_than", OperationType::GREATER_THAN},
            {"less_than", OperationType::LESS_THAN},
            {"greater_equal", OperationType::GREATER_EQUAL},
            {"less_equal", OperationType::LESS_EQUAL},
            {"bitwise_and", OperationType::BITWISE_AND},
            {"bitwise_or", OperationType::BITWISE_OR},
            {"bitwise_xor", OperationType::BITWISE_XOR},
            {"bitwise_not", OperationType::BITWISE_NOT},
            {"left_shift", OperationType::LEFT_SHIFT},
            {"right_shift", OperationType::RIGHT_SHIFT}
        };

        auto it = typeMap.find(str);
        return it != typeMap.end() ? it->second : OperationType::ADD;
    }
};
