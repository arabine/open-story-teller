#pragma once

#include <algorithm>
#include "json.hpp"

struct Connection
{

    enum Type
    {
        EXECUTION_LINK,
        DATA_LINK
    };

    Connection()
        : outPortIndex(0)
        , inPortIndex(0)
    {

    }

    ~Connection() {

    }

    Connection::Type type{Connection::EXECUTION_LINK};
    std::string outNodeId;
    unsigned int outPortIndex{0};
    std::string inNodeId;
    unsigned int inPortIndex{0};

    Connection(const Connection &other)
        : type(other.type)
        , outNodeId(other.outNodeId)
        , outPortIndex(other.outPortIndex)
        , inNodeId(other.inNodeId)
        , inPortIndex(other.inPortIndex)
    {

    }

    Connection& operator=(const Connection& other) {
        if (this != &other) { 
            this->outNodeId = other.outNodeId;
            this->outPortIndex = other.outPortIndex;
            this->inNodeId = other.inNodeId;
            this->inPortIndex = other.inPortIndex;
            this->type = other.type;
        }
        return *this;
    } 
};

inline bool operator==(Connection const &a, Connection const &b)
{
    return a.outNodeId == b.outNodeId && a.outPortIndex == b.outPortIndex
           && a.inNodeId == b.inNodeId && a.inPortIndex == b.inPortIndex && a.type == b.type;
}

inline bool operator!=(Connection const &a, Connection const &b)
{
    return !(a == b);
}


inline void invertConnection(Connection &id)
{
    std::swap(id.outNodeId, id.inNodeId);
    std::swap(id.outPortIndex, id.inPortIndex);
}

void to_json(nlohmann::json& j, const Connection& p);

void from_json(const nlohmann::json& j, Connection& p);



