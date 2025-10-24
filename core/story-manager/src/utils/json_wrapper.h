#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>

class JsonWrapper {
public:
    JsonWrapper();
    explicit JsonWrapper(const std::string& jsonString);
    ~JsonWrapper();

    // Ajout move constructor et move assign
    JsonWrapper(JsonWrapper&&) noexcept;
    JsonWrapper& operator=(JsonWrapper&&) noexcept;

    // Supprimer copie pour éviter erreur
    JsonWrapper(const JsonWrapper&) = delete;
    JsonWrapper& operator=(const JsonWrapper&) = delete;

    bool hasKey(const std::string& key) const;

    // Valeurs simples
    std::optional<std::string> getString(const std::string& key, std::string& errorKey) const;
    std::optional<int> getInt(const std::string& key, std::string& errorKey) const;
    std::optional<double> getDouble(const std::string& key, std::string& errorKey) const;
    std::optional<bool> getBool(const std::string& key, std::string& errorKey) const;

    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setDouble(const std::string& key, double value);
    void setBool(const std::string& key, bool value);

    // Array génériques
    template <typename T>
    void setArray(const std::string& key, const std::vector<T>& values) {
        setArrayImpl(key, reinterpret_cast<const void*>(&values), typeid(T).hash_code());
    }

    template <typename T>
    std::optional<std::vector<T>> getArray(const std::string& key, std::string& errorKey) const {
        return getArrayImpl<T>(key, errorKey);
    }

    // Sérialisation/Désérialisation générique
    template <typename T>
    T get(const std::string& key, std::string& errorKey) const {
        return getImpl<T>(key, errorKey);
    }

    template <typename T>
    T as(std::string& errorKey) const {
        return asImpl<T>(errorKey);
    }

    template <typename T>
    static JsonWrapper from(const T& value) {
        return fromImpl(reinterpret_cast<const void*>(&value), typeid(T).hash_code());
    }

    // Dump JSON
    std::string dump(int indent = -1) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;

    // Implémentations privées non template (définies dans .cpp)
    void setArrayImpl(const std::string& key, const void* values, size_t typeHash);
    
    template <typename T>
    std::optional<std::vector<T>> getArrayImpl(const std::string& key, std::string& errorKey) const;

    template <typename T>
    T getImpl(const std::string& key, std::string& errorKey) const;

    template <typename T>
    T asImpl(std::string& errorKey) const;

    static JsonWrapper fromImpl(const void* value, size_t typeHash);
};
