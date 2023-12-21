#include "connection.h"

void to_json(nlohmann::json &j, const Connection &p) {
    j = nlohmann::json{
                       {"outNodeId", static_cast<int64_t>(p.outNodeId)},
                       {"outPortIndex", static_cast<int64_t>(p.outPortIndex)},
                       {"inNodeId", static_cast<int64_t>(p.inNodeId)},
                       {"inPortIndex", static_cast<int64_t>(p.inPortIndex)},
                       };
}

void from_json(const nlohmann::json &j, Connection &p) {
    j.at("outNodeId").get_to(p.outNodeId);
    j.at("outPortIndex").get_to(p.outPortIndex);
    j.at("inNodeId").get_to(p.inNodeId);
    j.at("inPortIndex").get_to(p.inPortIndex);
}
