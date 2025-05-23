#pragma once

#include <string>
#include <memory>
#include <random>
#include <string>

#include "json.hpp"
#include "i_story_page.h"
#include "i_story_project.h"
#include "story_options.h"
#include "variable.h"

class IVariableVisitor {

public: 
    virtual void Visit(const std::shared_ptr<Variable> v) = 0;

};

class BaseNode
{
public:
    enum Behavior
    {
        BEHAVIOR_EXECUTION,
        BEHAVIOR_DATA,
    };

    struct Port
    {
        enum Type {
            EXECUTION_PORT,
            DATA_PORT
        };

        Port::Type type{EXECUTION_PORT};
        std::string label;
    };

    struct NodePosition
    {
        float x;
        float y;
    };


    struct ConstantValue
    {
        enum Type {
            TYPE_STRING,
            TYPE_INT,
        };
        std::string stringVal;
        int intVal{0};
        ConstantValue::Type type{TYPE_INT};
    };

    struct Constant
    {
        std::string label;
        int elementSize; // in bits: 8, 16 or 32
        std::vector<ConstantValue> values;
    };

    BaseNode(const std::string &type, const std::string &typeName, Behavior behavior = BEHAVIOR_EXECUTION);
    virtual ~BaseNode();

    static std::string GetEntryLabel(const std::string &id);

    virtual void Initialize() = 0;

    void SetPosition(float x, float y);

    // make this virtual so that graphical node override the behavior
    virtual float GetX() const;
    virtual float GetY() const;

    std::string GetMyEntryLabel() const {
        return GetEntryLabel(m_uuid);
    }

    // Coded type, internal use
    std::string GetType() const
    {
        return m_type;
    }

    // Human readable type
    std::string GetTypeName() const
    {
        return m_typeName;
    }


    void SetWeight(int w) {
        m_weight = w;
    }

    int GetWeight() const {
        return m_weight;
    }

    void SetId(const std::string &id) { m_uuid = id; }
    std::string GetId() const { return m_uuid; }

    void SetTitle(const std::string &title) { m_title = title; }
    std::string GetTitle() const { return m_title; }

    void FromJson(const nlohmann::json &);
    nlohmann::json ToJson() const;


    void SetInternalData(const nlohmann::json &j);
    nlohmann::json GetInternalData() const;

    void ClearPorts() {
        m_inputPorts.clear();
        m_outputPorts.clear();
    }

    // Port management
    void AddInputPort(Port::Type type, const std::string& label) {
        m_inputPorts.push_back({type, label});
    }

    void AddOutputPort(Port::Type type, const std::string& label) {
        m_outputPorts.push_back({type, label});
    }
    

    void SetBehavior(Behavior behavior) {
        m_behavior = behavior;
    }
    Behavior GetBehavior() const {
        return m_behavior;
    }

    void Accept(IVariableVisitor &visitor);


protected:
    // Easy access the variables for children nodes
    // Key is the variable name, or whatever the node use to identify the variable
    std::map<std::string, std::shared_ptr<Variable>> m_variables; 

private:
    std::string m_title{"Default title"};
    std::string m_type;
    std::string m_typeName;
    std::string m_uuid;
    NodePosition m_pos;
    int m_weight{0};
    Behavior m_behavior{BEHAVIOR_EXECUTION};

    std::vector<Port> m_inputPorts;
    std::vector<Port> m_outputPorts;

    nlohmann::json m_internal_data{{}};
};

