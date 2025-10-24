#include "json_wrapper.h"
#include "json.hpp"
#include <stdexcept>
#include <sstream>
#include <exception>
#include <iostream>


using json = nlohmann::json;

class JsonWrapper::Impl {
public:
    json j;
};

JsonWrapper::JsonWrapper() : impl(std::make_unique<Impl>()) {
    impl->j = json::object();
}

JsonWrapper::JsonWrapper(JsonWrapper&& other) noexcept : impl(std::move(other.impl)) {}

JsonWrapper& JsonWrapper::operator=(JsonWrapper&& other) noexcept {
    if (this != &other) {
        impl = std::move(other.impl);
    }
    return *this;
}

JsonWrapper::JsonWrapper(const std::string& jsonString) : impl(std::make_unique<Impl>()) {
    try {
        impl->j = json::parse(jsonString);
    } catch (...) {
        impl->j = json::object();
    }
}

JsonWrapper::~JsonWrapper() = default;

bool JsonWrapper::hasKey(const std::string& key) const {
    return impl->j.contains(key);
}

std::optional<std::string> JsonWrapper::getString(const std::string& key, std::string& errorKey) const {
    try {
        return impl->j.at(key).get<std::string>();
    } catch (...) {
        errorKey = key;
        return std::nullopt;
    }
}

std::optional<int> JsonWrapper::getInt(const std::string& key, std::string& errorKey) const {
    try {
        return impl->j.at(key).get<int>();
    } catch (...) {
        errorKey = key;
        return std::nullopt;
    }
}

std::optional<double> JsonWrapper::getDouble(const std::string& key, std::string& errorKey) const {
    try {
        return impl->j.at(key).get<double>();
    } catch (...) {
        errorKey = key;
        return std::nullopt;
    }
}

std::optional<bool> JsonWrapper::getBool(const std::string& key, std::string& errorKey) const {
    try {
        return impl->j.at(key).get<bool>();
    } catch (...) {
        errorKey = key;
        return std::nullopt;
    }
}

void JsonWrapper::setString(const std::string& key, const std::string& value) {
    impl->j[key] = value;
}

void JsonWrapper::setInt(const std::string& key, int value) {
    impl->j[key] = value;
}

void JsonWrapper::setDouble(const std::string& key, double value) {
    impl->j[key] = value;
}

void JsonWrapper::setBool(const std::string& key, bool value) {
    impl->j[key] = value;
}

void JsonWrapper::setArrayImpl(const std::string& key, const void* values, size_t typeHash) {
    // Ici on doit caster et stocker dans json, mais sans RTTI avancé on limite
    // Par exemple on peut specialiser cette méthode pour les types courants
    if (typeHash == typeid(int).hash_code()) {
        const auto& v = *reinterpret_cast<const std::vector<int>*>(values);
        impl->j[key] = v;
    } else if (typeHash == typeid(double).hash_code()) {
        const auto& v = *reinterpret_cast<const std::vector<double>*>(values);
        impl->j[key] = v;
    } else if (typeHash == typeid(std::string).hash_code()) {
        const auto& v = *reinterpret_cast<const std::vector<std::string>*>(values);
        impl->j[key] = v;
    } else if (typeHash == typeid(bool).hash_code()) {
        const auto& v = *reinterpret_cast<const std::vector<bool>*>(values);
        impl->j[key] = v;
    } else {
        // Pour les types non pris en charge, on peut lever une exception ou ignorer
        throw std::runtime_error("Type non supporté pour setArray");
    }
}

template <typename T>
std::optional<std::vector<T>> JsonWrapper::getArrayImpl(const std::string& key, std::string& errorKey) const {
    try {
        return impl->j.at(key).get<std::vector<T>>();
    } catch (...) {
        errorKey = key;
        return std::nullopt;
    }
}

template <typename T>
T JsonWrapper::getImpl(const std::string& key, std::string& errorKey) const {
    try {
        return impl->j.at(key).get<T>();
    } catch (...) {
        errorKey = key;
        throw;
    }
}

template <typename T>
T JsonWrapper::asImpl(std::string& errorKey) const {
    try {
        return impl->j.get<T>();
    } catch (...) {
        errorKey = "[root]";
        throw;
    }
}

JsonWrapper JsonWrapper::fromImpl(const void* value, size_t typeHash) {
    JsonWrapper wrapper;
    if (typeHash == typeid(int).hash_code()) {
        wrapper.impl->j = *reinterpret_cast<const int*>(value);
    } else if (typeHash == typeid(double).hash_code()) {
        wrapper.impl->j = *reinterpret_cast<const double*>(value);
    } else if (typeHash == typeid(std::string).hash_code()) {
        wrapper.impl->j = *reinterpret_cast<const std::string*>(value);
    } else if (typeHash == typeid(bool).hash_code()) {
        wrapper.impl->j = *reinterpret_cast<const bool*>(value);
    } else {
        // Si c’est un type complexe, il faut une surcharge ou un specialization dans le .cpp
        // Exemple: si T a to_json/from_json, on peut faire une conversion nlohmann::json value = T;
        // Ici on fait un cast "générique" (moins safe)
        // On peut faire un throw ici pour forcer l’utilisateur à spécialiser.
        throw std::runtime_error("Type non supporté pour from()");
    }
    return std::move(wrapper);
}

// Explicit instantiations for getArrayImpl, getImpl, asImpl for common types

template std::optional<std::vector<int>> JsonWrapper::getArrayImpl<int>(const std::string&, std::string&) const;
template std::optional<std::vector<double>> JsonWrapper::getArrayImpl<double>(const std::string&, std::string&) const;
template std::optional<std::vector<std::string>> JsonWrapper::getArrayImpl<std::string>(const std::string&, std::string&) const;
template std::optional<std::vector<bool>> JsonWrapper::getArrayImpl<bool>(const std::string&, std::string&) const;

template int JsonWrapper::getImpl<int>(const std::string&, std::string&) const;
template double JsonWrapper::getImpl<double>(const std::string&, std::string&) const;
template std::string JsonWrapper::getImpl<std::string>(const std::string&, std::string&) const;
template bool JsonWrapper::getImpl<bool>(const std::string&, std::string&) const;

template int JsonWrapper::asImpl<int>(std::string&) const;
template double JsonWrapper::asImpl<double>(std::string&) const;
template std::string JsonWrapper::asImpl<std::string>(std::string&) const;
template bool JsonWrapper::asImpl<bool>(std::string&) const;

std::string JsonWrapper::dump(int indent) const {
    if (indent < 0) return impl->j.dump();
    return impl->j.dump(indent);
}
