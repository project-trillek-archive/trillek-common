#ifndef EVENT_QUEUE_HPP_INCLUDED
#define EVENT_QUEUE_HPP_INCLUDED

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include "atomic-queue.hpp"

namespace trillek {
namespace event {

template <class T>
class Handler {
public:
    virtual void OnEvent(const T& data) { }
};

template <class T>
class EventQueue final {
private:
    EventQueue() {}
    ~EventQueue() {}
    EventQueue(const EventQueue&) = delete;
    EventQueue(EventQueue&&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;
    EventQueue& operator=(EventQueue&&) = delete;

    static EventQueue instance;

    mutable AtomicQueue<T> event_list;
    std::list<Handler<T>*> event_handlers;
public:

    static void Subscribe(Handler<T>* subscriber) {
        EventQueue<T>::instance.event_handlers.push_back(subscriber);
    }
    static void Unsubscribe(const Handler<T>* subscriber) {
        EventQueue<T>::instance.event_handlers.remove(subscriber);
    }
    static void QueueEvent(T&& ev) {
        EventQueue<T>::instance.event_list.Push(std::forward<T>(ev));
    }
    static void ProcessEvents() {
        EventQueue<T>::instance.SendEvents();
    }

private:
    void SendEvents() {
        for(auto& ev : event_list.Poll()) {
            for(auto handle : event_handlers) {
                handle->OnEvent(ev);
            }
        }
    }
};

template<class T> EventQueue<T> EventQueue<T>::instance;

template <class T>
void ProcessEvents() {
    EventQueue<T>::ProcessEvents();
}

template<typename T>
void QueueEvent(T&& ev) {
    EventQueue<T>::QueueEvent(std::forward<T>(ev));
}

} // namespace event
} // namespace trillek

#endif
