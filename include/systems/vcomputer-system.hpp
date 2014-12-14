#ifndef VCOMPUTER_SYSTEM_HPP_INCLUDED
#define VCOMPUTER_SYSTEM_HPP_INCLUDED

#include "trillek-scheduler.hpp"
#include "system-base.hpp"
#include <memory>
#include "systems/dispatcher.hpp"
#include "os-event.hpp"

namespace trillek {

class ComponentBase;

class VComputerSystem final : public SystemBase,
    public event::Subscriber<KeyboardEvent> {
public:

    VComputerSystem();
    ~VComputerSystem();

    /** \brief This function is executed when a thread is attached to the system
     */
    virtual void ThreadInit() {};

    /** \brief Handle incoming events to update data
     *
     * This function is called once every frame. It is the only
     * function that can write data. This function is in the critical
     * path, job done here must be simple.
     *
     * If event handling need some batch processing, a task list must be
     * prepared and stored temporarily to be retrieved by RunBatch().
     */
    virtual void HandleEvents(frame_tp timepoint);

    /** \brief Make all pre-update or post-update work.
     *
     * Pre-update work is prepared by HandleEvents() and executed here.
     * Updates must be done through events to be taken into account at
     * next frame.
     * Pre-update work is needed when data retrieved from event must be
     * processed (i.e make computation, reassemble, etc.) before being stored.
     *
     * Post-update work is the transformation of data to render entities or
     * other operations.
     *
     * Note that this function is const, i.e the system is in read-only mode.
     */
    virtual void RunBatch() const { }

    /** \brief Save the data and terminate the system
     *
     * This function is called when the program is closing
     */
    virtual void Terminate() { }

    void Notify(const KeyboardEvent* key_event);
private:

    frame_unit delta; // The time since the last HandleEvents was called.
};

} // namespace trillek

#endif // VCOMPUTER_SYSTEM_HPP_INCLUDED
