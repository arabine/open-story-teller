#pragma once

#include <algorithm>
#include "json.hpp"

struct Connection
{
    Connection()
        : outPortIndex(0)
        , inPortIndex(0)
    {

    }

    ~Connection() {

    }

    std::string outNodeId;
    unsigned int outPortIndex{0};
    std::string inNodeId;
    unsigned int inPortIndex{0};

    Connection(const Connection &other){
        *this = other;
    }

    Connection& operator=(const Connection& other) {
        this->outNodeId = other.outNodeId;
        this->outPortIndex = other.outPortIndex;
        this->inNodeId = other.inNodeId;
        this->inPortIndex = other.inPortIndex;
        return *this;
    }
};

inline bool operator==(Connection const &a, Connection const &b)
{
    return a.outNodeId == b.outNodeId && a.outPortIndex == b.outPortIndex
           && a.inNodeId == b.inNodeId && a.inPortIndex == b.inPortIndex;
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



