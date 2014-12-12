#ifndef IORDER_HPP_INCLUDED
#define IORDER_HPP_INCLUDED

#include "components/component.hpp"
#include "trillek-game.hpp"

namespace trillek {
namespace usercommand {

    using namespace trillek::component;

    void Execute(id_t id, std::shared_ptr<component::Container>&& usercommand) {
        switch(static_cast<Component>(usercommand->GetTypeId())) {
        case Component::Velocity : {
            Update<Component::Velocity>(id, std::move(usercommand));
        }
        break;
        case Component::Movable : {
            Update<Component::Movable>(id, *Get<Component::Movable>(usercommand).get());
        }
        break;
        case Component::Moving : {
            Update<Component::Moving>(id, *Get<Component::Moving>(usercommand).get());
        }
        break;
        }
    }

} // namespace usercommand
} // namespace trillek

#endif // IORDER_HPP_INCLUDED
