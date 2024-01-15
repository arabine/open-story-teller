#ifndef TLV_H
#define TLV_H

#include <stdint.h>

#define TLV_ARRAY_TYPE  0xAB
#define TLV_OBJECT_TYPE  0xE7
#define TLV_STRING_TYPE  0x3D
#define TLV_INTEGER     0x77
#define TLV_REAL        0xB8

#include <iostream>
#include <fstream>

class Tlv
{
public:
    explicit Tlv(const std::string &filename)
    {
        m_file = std::ofstream(filename, std::ios::out | std::ios::binary);
    }

    ~Tlv() {
        m_file.close();
    }

    void add_array(uint16_t size)
    {
        m_file.write(reinterpret_cast<const char*>(&m_objectType), sizeof(m_objectType));
        m_file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    }

    void add_string(const char *s, uint16_t size)
    {
        m_file.write(reinterpret_cast<const char*>(&m_stringType), sizeof(m_stringType));

        m_file.write(s, size);
    }

    void add_integer(uint32_t value)
    {
        static const uint16_t size = 4;
        m_file.write(reinterpret_cast<const char*>(&m_integerType), sizeof(m_integerType));
        m_file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        m_file.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void add_string(const std::string &s)
    {
        add_string(s.c_str(), s.size());
    }

    void add_object(uint16_t entries)
    {
        m_file.write(reinterpret_cast<const char*>(&m_arrayType), sizeof(m_arrayType));
        m_file.write(reinterpret_cast<const char*>(&entries), sizeof(entries));
    }

private:
    std::ofstream m_file;

    uint8_t m_arrayType = TLV_ARRAY_TYPE;
    uint8_t m_objectType = TLV_OBJECT_TYPE;
    uint8_t m_stringType = TLV_STRING_TYPE;
    uint8_t m_integerType = TLV_INTEGER;

};


#endif // TLV_H
