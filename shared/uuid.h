#ifndef UUID_H
#define UUID_H

#include <string>
#include <random>
#include <regex>

// Encaasulate the genaeration of a Version 4 UUID object
// A Version 4 UUID is a universally unique identifier that is generated using random numbers.
class UUID
{
public:

    UUID() { New(); }

    // Factory method for creating UUID object.
    void New()
    {
        std::random_device rd;
        std::mt19937 engine{rd()};
        std::uniform_int_distribution<int> dist{0, 256}; //Limits of the interval

        for (int index = 0; index < 16; ++index)
        {
            _data[index] = (unsigned char)dist(engine);
        }

        _data[6] = ((_data[6] & 0x0f) | 0x40); // Version 4
        _data[8] = ((_data[8] & 0x3f) | 0x80); // Variant is 10
    }

    // Returns UUID as formatted string
    std::string String()
    {
        // Formats to "0065e7d7-418c-4da4-b4d6-b54b6cf7466a"
        char buffer[256] = {0};
        std::snprintf(buffer, 255,
                      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                      _data[0], _data[1], _data[2], _data[3],
                      _data[4], _data[5],
                      _data[6], _data[7],
                      _data[8], _data[9],
                      _data[10], _data[11], _data[12], _data[13], _data[14], _data[15]);

        std::string uuid = buffer;

        return uuid;
    }

    static bool IsValid(const std::string& input) {
        // Le motif regex pour un UUID V4
        std::regex uuidRegex("^[0-9A-F]{8}-[0-9A-F]{4}-4[0-9A-F]{3}-[89AB][0-9A-F]{3}-[0-9A-F]{12}$",std::regex_constants::icase);

        // Vérifier si la chaîne correspond au motif UUID V4
        return std::regex_match(input, uuidRegex);
    }

    unsigned char _data[16] = {0};
};


#endif // UUID_H
