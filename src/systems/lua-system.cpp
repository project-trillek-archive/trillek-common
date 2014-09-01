#include "systems/lua-system.hpp"
#include "logging.hpp"
#include <iostream>

namespace trillek {
namespace script {

std::string lua_tostdstring(lua_State* L, int p) {
    size_t strl;
    const char * strc;
    if(!lua_isstring(L, p)) {
        return std::string();
    }
    strc = lua_tolstring(L, p, &strl);
    return std::string(strc, strl);
}

int luaopen_Transform(lua_State*);
int luaopen_LuaSys(lua_State*);

LuaSystem::LuaSystem() {
    event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);
    this->event_handlers[reflection::GetTypeID<KeyboardEvent>()];
    event::Dispatcher<MouseBtnEvent>::GetInstance()->Subscribe(this);
    this->event_handlers[reflection::GetTypeID<MouseBtnEvent>()];
    event::Dispatcher<MouseMoveEvent>::GetInstance()->Subscribe(this);
    this->event_handlers[reflection::GetTypeID<MouseMoveEvent>()];
}
LuaSystem::~LuaSystem() { }

void LuaSystem::Start() {
    this->L = luaL_newstate();
    luaL_openlibs(L);
    RegisterTypes();
}

void LuaSystem::AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component) { }

void LuaSystem::RegisterSystem(LuaRegisterFunction func) {
    if (this->L) {
        func(this->L);
    }
}

bool LuaSystem::LoadFile(const std::string fname) {
    if (!this->L) {
        return false;
    }
    if (luaL_dofile(L, fname.c_str())) {
        // TODO: Ommit error string about the filename that failed to load.
        std::cerr << lua_tostring(L, -1) << std::endl;
        return false;
    }
    return true;
}

void LuaSystem::HandleEvents(const frame_tp& timepoint) {
    static frame_tp last_tp;
    this->delta = timepoint - last_tp;
    last_tp = timepoint;
    if (this->L) {
        // Call the registered event handlers in Lua.
        auto evh_itr = this->event_handlers.find(reflection::GetTypeID<KeyboardEvent>());
        if(evh_itr != this->event_handlers.end()) {
            this->Lm.lock();
            auto key_event = this->event_key.begin();
            for( ;key_event != this->event_key.end(); key_event++) {
                for(auto& handler : evh_itr->second) {
                    lua_getglobal(L, handler.c_str());
                    if(key_event->action == KeyboardEvent::KEY_DOWN) {
                        lua_pushstring(L, "Down");
                    }
                    else if(key_event->action == KeyboardEvent::KEY_UP) {
                        lua_pushstring(L, "Up");
                    }
                    else if(key_event->action == KeyboardEvent::KEY_REPEAT) {
                        lua_pushstring(L, "Repeat");
                    }
                    else {
                        lua_pushnil(L);
                    }
                    lua_pushnumber(L, key_event->key);
                    if(lua_pcall(L, 2, 0, 0)) {
                        size_t sl = 0;
                        const char *cs = lua_tolstring(L, -1, &sl);
                        LOGMSG(ERROR) << cs;
                    }
                }
            }
            this->event_key.clear();
            this->Lm.unlock();
        }
        evh_itr = this->event_handlers.find(reflection::GetTypeID<MouseMoveEvent>());
        if(evh_itr != this->event_handlers.end()) {
            this->Lm.lock();
            auto mousemove_event = this->event_mmove.begin();
            for( ;mousemove_event != this->event_mmove.end(); mousemove_event++) {
                for(auto& handler : evh_itr->second) {
                    lua_getglobal(L, handler.c_str());
                    lua_pushnumber(L, mousemove_event->new_x);
                    lua_pushnumber(L, mousemove_event->new_y);
                    lua_pushnumber(L, mousemove_event->old_x);
                    lua_pushnumber(L, mousemove_event->old_y);
                    lua_pushnumber(L, mousemove_event->norm_x);
                    lua_pushnumber(L, mousemove_event->norm_y);
                    if(lua_pcall(L, 6, 0, 0)) {
                        size_t sl = 0;
                        const char *cs = lua_tolstring(L, -1, &sl);
                        LOGMSG(ERROR) << cs;
                    }
                }
            }
            this->event_mmove.clear();
            this->Lm.unlock();
        }
        evh_itr = this->event_handlers.find(reflection::GetTypeID<MouseBtnEvent>());
        if(evh_itr != this->event_handlers.end()) {
            this->Lm.lock();
            auto mousebtn_event = event_mbtn.begin();
            for( ;mousebtn_event != event_mbtn.end(); mousebtn_event++) {
                for (auto& handler : evh_itr->second) {
                    lua_getglobal(L, handler.c_str());
                    if(mousebtn_event->action == MouseBtnEvent::DOWN) {
                        lua_pushstring(L, "Down");
                    }
                    else if(mousebtn_event->action == MouseBtnEvent::UP) {
                        lua_pushstring(L, "Up");
                    }
                    else {
                        lua_pushnil(L);
                    }
                    if(mousebtn_event->button == MouseBtnEvent::LEFT) {
                        lua_pushstring(L, "Left");
                    }
                    else if(mousebtn_event->button == MouseBtnEvent::RIGHT) {
                        lua_pushstring(L, "Right");
                    }
                    else if(mousebtn_event->button == MouseBtnEvent::MIDDLE) {
                        lua_pushstring(L, "Middle");
                    }
                    else {
                        lua_pushnil(L);
                    }
                    if(lua_pcall(L, 2, 0, 0)) {
                        size_t sl = 0;
                        const char *cs = lua_tolstring(L, -1, &sl);
                        LOGMSG(ERROR) << cs;
                    }
                }
            }
            this->event_mbtn.clear();
            this->Lm.unlock();
        }
        // Call the registered update functions in Lua.
        evh_itr = this->event_handlers.find(1);
        if (evh_itr != this->event_handlers.end()) {
            this->Lm.lock();
            for(auto& handler : evh_itr->second) {
                lua_getglobal(L, handler.c_str());
                lua_pushnumber(L, delta.count() * 1.0E-9);
                if(lua_pcall(L, 1, 0, 0)) {
                    size_t sl = 0;
                    const char *cs = lua_tolstring(L, -1, &sl);
                    LOGMSG(ERROR) << cs;
                }
            }
            this->Lm.unlock();
        }
    }
}

void LuaSystem::Terminate() {
    lua_close(L);
}

void LuaSystem::AddUIEventType(uint32_t event_id, const std::string& event_class, const std::string& event_value) {
    Lm.lock();
    lua_getglobal(L, "_UI");
    int s = lua_gettop(L);
    lua_pushinteger(L, event_id);
    luaL_loadstring(L, event_value.c_str());
    lua_settable(L, s);
    lua_pop(L, 1);
    Lm.unlock();
}

void LuaSystem::UINotify(uint32_t event_id, const std::string& element_id) {
    Lm.lock();
    lua_getglobal(L, "_UI");
    int s = lua_gettop(L);
    lua_pushinteger(L, event_id);
    lua_gettable(L, s);
    if(lua_isfunction(L, lua_gettop(L))) {
        lua_pushstring(L, element_id.c_str());
        lua_pcall(L, 1, 0, 0);
    }
    lua_pop(L, 1);
    Lm.unlock();
}

void LuaSystem::Notify(const KeyboardEvent* key_event) {
    Lm.lock();
    event_key.push_back(*key_event);
    Lm.unlock();
}

void LuaSystem::Notify(const MouseBtnEvent* mousebtn_event) {
    Lm.lock();
    event_mbtn.push_back(*mousebtn_event);
    Lm.unlock();
}

void LuaSystem::Notify(const MouseMoveEvent* mousemove_event) {
    Lm.lock();
    event_mmove.push_back(*mousemove_event);
    Lm.unlock();
}

} // End of script
} // End of trillek
