#include <stddef.h>

#include "lua.h"
#include "lauxlib.h"

static const char *get_source_filename(lua_State *L, lua_Integer level) {
    lua_Debug ar;

    if (!lua_getstack(L, level - 1, &ar)) {
        return NULL;
    }

    lua_getinfo(L, "S", &ar);

    if (ar.source[0] == '@') {
        return ar.source + 1;
    } else {
        /*
        ** Ignore Lua code loaded from raw strings,
        ** unless runner.configuration.codefromstrings is true.
        */
        lua_getfield(L, lua_upvalueindex(1), "configuration");
        lua_getfield(L, -1, "codefromstrings");
        int codefromstrings = lua_toboolean(L, -1);
        lua_pop(L, 2);
        return codefromstrings ? ar.source : NULL;
    }
}

static int l_debug_hook(lua_State *L) {
    lua_Integer line_nr = luaL_checkinteger(L, 2);
    lua_Integer level = luaL_optinteger(L, 3, 2);
    lua_settop(L, 0);

    lua_getfield(L, lua_upvalueindex(1), "initialized");

    if (!lua_toboolean(L, -1)) {
        return 0;
    }

    lua_pop(L, 1);

    const char *filename = get_source_filename(L, level);

    if (filename == NULL) {
        return 0;
    }

    lua_getfield(L, lua_upvalueindex(1), "data");
    lua_getfield(L, -1, filename);

    if (!lua_istable(L, -1)) {
        /* New or ignored file. */
        lua_pop(L, 1);
        lua_getfield(L, lua_upvalueindex(2), filename);

        if (lua_toboolean(L, -1)) {
            /* Ignored file. */
            return 0;
        }

        lua_pop(L, 1);
        /* New file, have to call runner.file_included. */
        lua_getfield(L, lua_upvalueindex(1), "file_included");
        lua_pushstring(L, filename);
        lua_call(L, 1, 1);

        if (!lua_toboolean(L, -1)) {
            /* Remember this file as ignored. */
            lua_pushboolean(L, 1);
            lua_setfield(L, lua_upvalueindex(2), filename);
            return 0;
        }

        lua_pop(L, 1);

        /* Construct file data table for this new file. */
        lua_newtable(L);
        lua_pushinteger(L, 0);
        lua_setfield(L, -2, "max");
        lua_pushinteger(L, 0);
        lua_setfield(L, -2, "max_hits");

        /* Copy file data table and save it as data[filename]. */
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, filename);
    }

    /* Update max line number for this file if necessary. */
    lua_getfield(L, -1, "max");
    lua_Integer max_line_nr = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if (line_nr > max_line_nr) {
        lua_pushinteger(L, line_nr);
        lua_setfield(L, -2, "max");
    }

    /* Increment file_data[line_nr]. */
    lua_pushinteger(L, line_nr);
    lua_pushinteger(L, line_nr);
    lua_gettable(L, -3);
    /* Conveniently, lua_tointeger returns 0 on non-number. */
    lua_Integer hits = lua_tointeger(L, -1) + 1;
    lua_pop(L, 1);
    lua_pushinteger(L, hits);
    lua_settable(L, -3);

    /* Update max number of hits if necessary. */
    lua_getfield(L, -1, "max_hits");
    lua_Integer max_hits = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if (hits > max_hits) {
        lua_pushinteger(L, hits);
        lua_setfield(L, -2, "max_hits");
    }

    /* Handle runner.tick. */
    lua_getfield(L, lua_upvalueindex(1), "tick");

    if (!lua_toboolean(L, -1)) {
        return 0;
    }

    lua_getfield(L, lua_upvalueindex(1), "configuration");
    lua_getfield(L, -1, "savestepsize");
    lua_Integer save_step_size = lua_tointeger(L, -1);
    lua_pop(L, 2);

    lua_Integer steps_after_save = lua_tointeger(L, lua_upvalueindex(3)) + 1;

    if (steps_after_save == save_step_size) {
        steps_after_save = 0;

        /* Unless runner.paused, save data. */
        lua_getfield(L, lua_upvalueindex(1), "paused");
        int paused = lua_toboolean(L, -1);
        lua_pop(L, 1);

        if (!paused) {
            lua_getfield(L, lua_upvalueindex(1), "save_stats");
            lua_call(L, 0, 0);
        }
    }

    lua_pushinteger(L, steps_after_save);
    lua_replace(L, lua_upvalueindex(3));
    return 0;
}

int l_new_hook(lua_State *L) {
    /*
    ** Upvalues for the debug hook:
    ** runner module (already on the stack),
    ** ignored files set
    ** and steps counter.
    */
    lua_settop(L, 1);
    lua_newtable(L);
    lua_pushinteger(L, 0);
    lua_pushcclosure(L, l_debug_hook, 3);
    return 1;
}

int luaopen_cluacov_hook(lua_State *L) {
    lua_newtable(L);
    lua_pushcfunction(L, l_new_hook);
    lua_setfield(L, -2, "new");
    return 1;
}
