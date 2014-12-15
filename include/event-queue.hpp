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

template <class T> class EventQueue;

template <class T>
class LocalHandler {
    friend class EventQueue<T>;
public:
    virtual ~LocalHandler() {}

    virtual void OnEvent(const T& data) = 0;

private:
    mutable AtomicQueue<std::shared_ptr<T>> event_list;
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

    mutable AtomicQueue<std::shared_ptr<T>> event_list;
    std::list<Handler<T>*> event_handlers;
    mutable std::mutex queuer_lock; // protect against subscription changes
    mutable std::list<LocalHandler<T>*> event_queuers;
public:

    static void Subscribe(Handler<T>* subscriber) {
        EventQueue<T>::instance.QueueSubscribe(subscriber);
    }
    static void Subscribe(LocalHandler<T>* subscriber) {
        EventQueue<T>::instance.QueueSubscribe(subscriber);
    }
    static void Unsubscribe(const Handler<T>* subscriber) {
        EventQueue<T>::instance.QueueUnsubscribe(subscriber);
    }
    static void Unsubscribe(const LocalHandler<T>* subscriber) {
        EventQueue<T>::instance.QueueUnsubscribe(subscriber);
    }
    static void QueueEvent(T&& ev) {
        EventQueue<T>::instance.SendEvent(std::forward<T>(ev));
    }
    static void QueueEvent(const T& ev) {
        EventQueue<T>::instance.SendEvent(ev);
    }
    static void ProcessEvents() {
        EventQueue<T>::instance.SendEvents();
    }
    static void ProcessEvents(LocalHandler<T>* eh) {
    for(auto& ev : eh->event_list.Poll()) {
        eh->OnEvent(*ev.get());
    }
}

private:
    void QueueSubscribe(Handler<T>* subscriber) const {
        std::unique_lock<std::mutex> lock(queuer_lock);
        event_handlers.push_back(subscriber);
    }
    void QueueSubscribe(LocalHandler<T>* subscriber) const {
        std::unique_lock<std::mutex> lock(queuer_lock);
        event_queuers.push_back(subscriber);
    }
    void QueueUnsubscribe(const Handler<T>* subscriber) {
        std::unique_lock<std::mutex> lock(queuer_lock);
        event_handlers.remove(subscriber);
        if(event_handlers.empty()) {
            event_list.Poll();
        }
    }
    void QueueUnsubscribe(const LocalHandler<T>* subscriber) {
        std::unique_lock<std::mutex> lock(queuer_lock);
        event_queuers.remove(subscriber);
    }
    void SendEvent(T&& ev) const {
        auto ptr = std::shared_ptr<T>(new T(std::forward<T>(ev)));
        if(!event_handlers.empty()) {
            event_list.Push(ptr);
        }
        std::unique_lock<std::mutex> lock(queuer_lock);
        for(auto evh : event_queuers) {
            evh->event_list.Push(ptr);
        }
    }
    void SendEvent(const T& ev) const {
        auto ptr = std::shared_ptr<T>(new T(ev));
        std::unique_lock<std::mutex> lock(queuer_lock);
        if(!event_handlers.empty()) {
            event_list.Push(ptr);
        }
        for(auto evh : event_queuers) {
            evh->event_list.Push(ptr);
        }
    }
    void SendEvents() {
        for(auto& ev : event_list.Poll()) {
            for(auto handle : event_handlers) {
                handle->OnEvent(*ev.get());
            }
        }
    }
};

template<class T> EventQueue<T> EventQueue<T>::instance;

template<typename T>
void QueueEvent(T&& ev) {
    EventQueue<T>::QueueEvent(std::forward<T>(ev));
}
template<typename T>
void QueueEvent(const T& ev) {
    EventQueue<T>::QueueEvent(ev);
}

} // namespace event
} // namespace trillek

#endif
