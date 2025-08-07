// event_bus.h
#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include "events/event.h" // Inclure la classe de base Event

#include <functional> // Pour std::function
#include <map>        // Pour std::map (TypeId -> vector of handlers)
#include <vector>     // Pour std::vector (list of handlers)
#include <memory>     // Pour std::shared_ptr si on veut gérer la durée de vie des événements

// Définition du type de handler générique
// Un handler est une fonction qui prend une référence constante à un Event.
using EventCallback = std::function<void(const Event&)>;

class EventBus
{
public:
    EventBus() = default;
    ~EventBus() = default;

    // Supprime la copie pour éviter des comportements inattendus avec les callbacks
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // S'abonne à un type d'événement spécifique.
    // 'TEvent' est le type d'événement à écouter (ex: VmStateChangedEvent).
    // 'callback' est la fonction à appeler lorsque cet événement est émis.
    template <typename TEvent>
    void Subscribe(std::function<void(const TEvent&)> callback)
    {
        // On stocke une lambda qui convertit l'Event de base en TEvent
        // pour que la callback spécifique à TEvent puisse être appelée.
        std::type_index typeId = GetEventTypeId<TEvent>();
        m_subscribers[typeId].push_back([callback](const Event& event) {
            // S'assurer que le cast est sûr
            if (const TEvent* specificEvent = dynamic_cast<const TEvent*>(&event)) {
                callback(*specificEvent);
            }
        });
    }

    // Émet un événement.
    // L'EventBus prend la possession de l'événement (via std::shared_ptr).
    // Ceci est crucial car l'événement doit exister pendant que les callbacks sont appelées.
    void Emit(std::shared_ptr<Event> event);

private:
    // Mappe les TypeId des événements à une liste de callbacks.
    std::map<std::type_index, std::vector<EventCallback>> m_subscribers;
};

#endif // EVENT_BUS_H