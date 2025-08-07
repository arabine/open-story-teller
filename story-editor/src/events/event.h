// events/event.h
#ifndef EVENT_H
#define EVENT_H

#include <typeindex> // Pour std::type_index

// Chaque événement aura un TypeId unique.
// Ceci permet de stocker et de récupérer des pointeurs vers des handlers spécifiques.
struct Event
{
    virtual ~Event() = default;
};

// Fonction utilitaire pour obtenir le TypeId unique d'une classe d'événement.
// Utilise std::type_index pour l'identification au runtime.
template <typename T>
std::type_index GetEventTypeId()
{
    return std::type_index(typeid(T));
}

#endif // EVENT_H