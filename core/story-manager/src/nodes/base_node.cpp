#include "base_node.h"
#include "uuid.h"
#include <iostream>

BaseNode::BaseNode(const std::string &type, const std::string &typeName, Behavior behavior)
    : m_behavior(behavior)
{
    m_type = type;
    m_typeName = typeName; 
    m_uuid = Uuid().String();

    nlohmann::json obj{};
    m_internal_data = obj;
}


BaseNode::~BaseNode()
{
    std::cout << "Deleted base node" << std::endl;
}


std::string BaseNode::GetEntryLabel(const std::string &id)
{
    std::stringstream ss;
    ss << ".nodeEntry" << std::setw(4) << std::setfill('0') << id;
    return ss.str();
}


void BaseNode::FromJson(const nlohmann::json &j)
{
    try
    {
        m_uuid = j["uuid"].get<std::string>();
        m_internal_data = j["internal-data"];
        m_type = j["type"].get<std::string>();
        m_title = j.value("title", "Default node");
        nlohmann::json posJson = j["position"];

        SetPosition(posJson["x"].get<double>(), posJson["y"].get<double>());
    }
    catch (std::exception&  e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
    }

}


nlohmann::json BaseNode::ToJson() const
{
    nlohmann::json node;
    node["uuid"] = GetId();
    node["type"] = GetType();

    nlohmann::json position;
    position["x"] = GetX();
    position["y"] = GetY();

    node["position"] = position;
    node["internal-data"] = m_internal_data;
    
    return node;
}


void BaseNode::SetInternalData(const nlohmann::json &j)
{
    m_internal_data = j;
}

nlohmann::json  BaseNode::GetInternalData() const
{
    return m_internal_data;
}

void BaseNode::SetPosition(float x, float y)
{
   m_pos.x = x;
   m_pos.y = y;
}

float BaseNode::GetX() const
{
    return m_pos.x;
}

float BaseNode::GetY() const
{
    return m_pos.y;
}

void BaseNode::Accept(IVariableVisitor &visitor)
{

    for (auto &v : m_variables)
    {
        visitor.Visit(v.second);
    }

}
