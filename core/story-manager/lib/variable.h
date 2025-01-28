#pragma once

#include <string>
#include <cstdint>

struct Variable {
    static const uint32_t NameMaxSize = 50;
    std::string name;      // Nom de la variable
    std::string type;      // Type de la variable (par exemple int32_t, int64_t)
    int64_t value;         // Valeur stockée en tant qu'entier en virgule fixe
    std::string valueText;
    int scalePower;    // Nombre de bits pour la partie fractionnaire

    Variable(const std::string &n, const std::string &t, int64_t v, int s) {
        name = n;
        type = t;
        value = v;
        scalePower = s;
    }
};
