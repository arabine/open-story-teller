#ifndef RESOURCE_H
#define RESOURCE_H

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <list>
#include <unordered_map>


struct Resource
{
    enum ImageFormat { IMG_SAME_FORMAT, IMG_FORMAT_QOIF, IMG_FORMAT_COUNT };
    enum SoundFormat { SND_SAME_FORMAT, SND_FORMAT_WAV, SND_FORMAT_QOAF, SND_FORMAT_COUNT };

    std::string file;
    std::string description;
    std::string format;
    std::string type;

    ~Resource() {
        // std::cout << "Res deleted" << std::endl;
    }

    static std::string ImageFormatToString(ImageFormat format);
    static std::string SoundFormatToString(SoundFormat format);
    static std::string ImageExtension(const std::string &filename, Resource::ImageFormat prefered_format);
    static std::string SoundExtension(const std::string &filename, Resource::SoundFormat prefered_format);
};

// Itérateur pour parcourir les éléments filtrés
class FilterIterator {
public:
    using Iterator = std::list<std::shared_ptr<Resource>>::const_iterator;

public:
    FilterIterator(Iterator start, Iterator end, const std::string &type)
        : current(start), end(end), filterType(type) {
        searchNext();
    }

    // Surcharge de l'opérateur de déréférencement
    const std::shared_ptr<Resource>& operator*() const {
        return *current;
    }

    const Iterator Current() const {
        return current;
    }

    // Surcharge de l'opérateur d'incrémentation
    FilterIterator& operator++() {
        ++current;
        searchNext();
        return *this;
    }

    // Surcharge de l'opérateur d'égalité
    bool operator==(const FilterIterator& other) const {
        return current == other.current;
    }

    // Surcharge de l'opérateur de différence
    bool operator!=(const FilterIterator& other) const {
        return !(*this == other);
    }

private:
    Iterator current;
    Iterator end;
    std::string filterType;

    // Fonction pour trouver le prochain élément qui correspond au filtre
    void searchNext() {

        if (filterType != "")
        {
            while (current != end && (*current)->type != filterType) {
                ++current;
            }
        }
    }
};


#endif // RESOURCE_H
