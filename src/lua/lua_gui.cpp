#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "systems/gui.hpp"
#include "trillek-game.hpp"

namespace trillek {
namespace script {

int Gui_Get(lua_State* L) {
    luaW_push<gui::GuiSystem>(L, &TrillekGame::GetGUISystem());
    return 1;
}
int Gui_LoadDoc(lua_State* L) {
    if(!luaW_is<gui::GuiSystem>(L, 1)) {
        return 0;
    }
    gui::GuiSystem *gui = luaW_to<gui::GuiSystem>(L, 1);
    auto doc = lua_tostdstring(L, 2);
    uint32_t docid = gui->AsyncLoadDocument(doc);
    lua_pushunsigned(L, docid);
    return 1;
}
int Gui_CloseDoc(lua_State* L) {
    if(!luaW_is<gui::GuiSystem>(L, 1)) {
        return 0;
    }
    gui::GuiSystem *gui = luaW_to<gui::GuiSystem>(L, 1);
    int isnum;
    uint32_t doc = lua_tounsignedx(L, 2, &isnum);
    if(isnum) {
        gui->AsyncCloseDocument(doc);
    }
    return 0;
}
int Gui_LoadFont(lua_State* L) {
    if(!luaW_is<gui::GuiSystem>(L, 1)) {
        return 0;
    }
    gui::GuiSystem *gui = luaW_to<gui::GuiSystem>(L, 1);
    auto font = lua_tostdstring(L, 2);
    gui->LoadFont(font);
    return 0;
}
int Gui_Subscribe(lua_State* L) {
    if(!luaW_is<gui::GuiSystem>(L, 1)) {
        return 0;
    }
    gui::GuiSystem *gui = luaW_to<gui::GuiSystem>(L, 1);
    return 0;
}

static luaL_Reg GuiSys_table[] =
{
    { "Get", Gui_Get },
    { nullptr, nullptr } // table end marker
};

static luaL_Reg GuiSys_metatable[] =
{
    { "LoadDocument", Gui_LoadDoc },
    { "LoadDoc", Gui_LoadDoc },
    { "CloseDoc", Gui_CloseDoc },
    { "LoadFont", Gui_LoadFont },
    { "Subscribe", Gui_Subscribe },
    { nullptr, nullptr } // table end marker
};

int luaopen_GuiSys(lua_State* L) {
    luaW_register<gui::GuiSystem>(L,
        "GUI",
        GuiSys_table,
        GuiSys_metatable,
        nullptr // no constructor
        );
    lua_newtable(L);
    lua_setglobal(L, "_UI");
    return 1;
}

} // script
} // trillek
