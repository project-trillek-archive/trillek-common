#ifndef LUA_INTERACTION_HPP_INCLUDED
#define LUA_INTERACTION_HPP_INCLUDED

#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "interaction.hpp"

template<>
struct luaU_Impl<trillek::InteractEvent> {
    static trillek::InteractEvent luaU_check(lua_State* L, int index) {
        return trillek::InteractEvent(
            (trillek::Action)luaU_getfield<uint32_t>(L, index, "action"),
            luaU_getfield<uint32_t>(L, index, "entity"),
            luaU_getfield<uint32_t>(L, index, "num"));
    }
    static trillek::InteractEvent luaU_to(lua_State* L, int index) {
        return trillek::InteractEvent(
            (trillek::Action)luaU_getfield<uint32_t>(L, index, "action"),
            luaU_getfield<uint32_t>(L, index, "entity"),
            luaU_getfield<uint32_t>(L, index, "num"));
    }
    static void luaU_push(lua_State* L, const trillek::InteractEvent& val) {
        lua_newtable(L);
        luaU_setfield(L, -1, "action", (uint32_t)val.act);
        luaU_setfield(L, -1, "text", trillek::ActionText::Get(val.act, 0));
        luaU_setfield(L, -1, "entity", val.entity);
        luaU_setfield(L, -1, "num", val.num);
    }
    static void luaU_push(lua_State* L, trillek::InteractEvent& val) {
        lua_newtable(L);
        luaU_setfield(L, -1, "action", (uint32_t)val.act);
        luaU_setfield(L, -1, "text", trillek::ActionText::Get(val.act, 0));
        luaU_setfield(L, -1, "entity", val.entity);
        luaU_setfield(L, -1, "num", val.num);
    }
};


#endif
