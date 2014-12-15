#include "systems/lua-system.hpp"
#include "trillek-game.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "components/component.hpp"
#include "components/shared-component.hpp"
#include "components/system-component.hpp"
#include "components/system-component-value.hpp"

#include "interaction.hpp"
#include "lua/lua_interaction.hpp"

namespace trillek {
namespace script {

static int LuaEnt_Interactive(lua_State* L) {
    uint32_t id = (uint32_t)luaL_checkint(L, 1);
    lua_pushboolean(L, component::Has<component::Component::Interactable>(id)? 1 : 0);
    return 1;
}

static int LuaEnt_GetActions(lua_State* L) {
    uint32_t id = (uint32_t)luaL_checkint(L, 1);
    auto& inter = component::Get<component::Component::Interactable>(id);
    int ct = inter.ActionCount();
    lua_createtable(L, ct, 0);
    int t = lua_gettop(L);
    for(int i = 0; i < ct; i++) {
        lua_pushinteger(L,1+i);
        luaU_push(L, inter.GetAction(i));
        lua_settable(L, t);
    }
    return 1;
}

static int LuaEnt_Trigger(lua_State* L) {
    uint32_t id = (uint32_t)luaL_checkint(L, 1);
    if(!component::Has<component::Component::Interactable>(id)) return 0;
    auto& inter = component::Get<component::Component::Interactable>(id);
    uint32_t index = (uint32_t)luaL_checkint(L, 2);
    int ct = inter.ActionCount();
    if(index > 0 && index <= ct) {
        inter.TriggerAction(index - 1);
    }
    return 0;
}

int luaopen_Entity(lua_State* L) {
    lua_createtable(L, 0, 0);
    int t = lua_gettop(L);
    lua_pushcclosure(L, LuaEnt_Interactive, 0); lua_setfield(L, t, "is_interactive");
    lua_pushcclosure(L, LuaEnt_GetActions, 0); lua_setfield(L, t, "get_actions");
    lua_pushcclosure(L, LuaEnt_Trigger, 0); lua_setfield(L, t, "trigger");
    lua_setglobal(L, "Entity");
    lua_pushboolean(L, 1);
    return 1;
}

} // namespace script
}
