#include "connection.h"

void to_json(nlohmann::json &j, const Connection &p) {
    j = nlohmann::json{
                       {"outNodeId", p.outNodeId },
                       {"outPortIndex", static_cast<int64_t>(p.outPortIndex)},
                       {"inNodeId", p.inNodeId},
                       {"inPortIndex", static_cast<int64_t>(p.inPortIndex)},
                       };
}

void from_json(const nlohmann::json &j, Connection &p) {

    p.outNodeId = j["outNodeId"].get<std::string>();
    p.inNodeId = j["inNodeId"].get<std::string>();
    p.outPortIndex = j["outPortIndex"].get<int>();
    p.inPortIndex = j["inPortIndex"].get<int>();
}
