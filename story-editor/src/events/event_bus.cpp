// event_bus.cpp
#include "event_bus.h"
#include <iostream> // Pour des logs simples de débogage si nécessaire

void EventBus::Emit(std::shared_ptr<Event> event)
{
    if (!event) {
        // Gérer le cas où l'événement est nul
        std::cerr << "EventBus: Attempted to emit a null event." << std::endl;
        return;
    }

    std::type_index eventTypeId = GetEventTypeId<Event>(); // Default type for base Event

    // Tente de trouver le type exact de l'événement pour la distribution
    // C'est un peu tricky car dynamic_cast ne peut pas être utilisé avec typeid(Event).
    // On doit utiliser typeid(*event) pour obtenir le type réel de l'objet pointé.
    eventTypeId = std::type_index(typeid(*event));

    auto it = m_subscribers.find(eventTypeId);
    if (it != m_subscribers.end())
    {
        // Parcourt tous les abonnés pour ce type d'événement
        for (const auto& callback : it->second)
        {
            callback(*event); // Appelle la callback en passant l'événement de base
        }
    } else {
        // Optionnel: loguer si aucun abonné pour ce type d'événement
        // std::cout << "EventBus: No subscribers for event type: " << eventTypeId.name() << std::endl;
    }
}
