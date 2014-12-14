#include "systems/lua-system.hpp"
#include "trillek-game.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "components/component.hpp"
#include "components/shared-component.hpp"
#include "components/system-component.hpp"
#include "components/system-component-value.hpp"

using namespace trillek;

static int LuaEnt_Interactive(lua_State* L) {
    uint32_t id = (uint32_t)luaL_checkint(L, 1);
    lua_pushboolean(L, component::Has<component::Component::Interactable>(id)? 1 : 0);
    return 1;
}

int luaopen_Entity(lua_State* L) {
    lua_createtable(L, 0, 0);
    int t = lua_gettop(L);
    lua_pushcclosure(L, LuaEnt_Interactive, 0); lua_setfield(L, t, "is_interactive");
    lua_setglobal(L, "Entity");
    lua_pushboolean(L, 1);
    return 1;
}
