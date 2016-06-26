#include "lua.h"
#include "lauxlib.h"

#if LUA_VERSION_NUM > 501 || defined(LUAI_BITSINT)
#define PUCRIOLUA
#endif

#ifdef PUCRIOLUA
#if LUA_VERSION_NUM == 501
#include "lua51/lobject.h"
#elif LUA_VERSION_NUM == 502
#include "lua52/lobject.h"
#elif LUA_VERSION_NUM == 503
#include "lua53/lobject.h"
#else
#error unsupported Lua version
#endif
#else /* LuaJIT */
#include "lj2/lj_obj.h"
#endif

#ifdef PUCRIOLUA

static Proto *get_proto(lua_State *L) {
    return ((Closure *) lua_topointer(L, 1))->l.p;
}

static void add_activelines(lua_State *L, Proto *proto) {
    /*
    ** For standard Lua active lines and nested prototypes
    ** are simply members of Proto, see lobject.h.
    */
    int i;

    for (i = 0; i < proto->sizelineinfo; i++) {
        lua_pushinteger(L, proto->lineinfo[i]);
        lua_pushboolean(L, 1);
        lua_settable(L, -3);
    }

    for (i = 0; i < proto->sizep; i++) {
        add_activelines(L, proto->p[i]);
    }
}

#else /* LuaJIT */

static GCproto *get_proto(lua_State *L) {
    return funcproto(funcV(L->base));
}

static void add_activelines(lua_State *L, GCproto *proto) {
    /*
    ** LuaJIT packs active lines depending on function length.
    ** See implementation of lj_debug_getinfo in lj_debug.c.
    */
    ptrdiff_t idx;
    const void *lineinfo = proto_lineinfo(proto);

    if (lineinfo) {
        BCLine first = proto->firstline;
        int sz = proto->numline < 256 ? 1 : proto->numline < 65536 ? 2 : 4;
        MSize i, szl = proto->sizebc - 1;

        for (i = 0; i < szl; i++) {
            BCLine line = first +
                (sz == 1 ? (BCLine) ((const uint8_t *) lineinfo)[i] :
                 sz == 2 ? (BCLine) ((const uint16_t *) lineinfo)[i] :
                 (BCLine) ((const uint32_t *) lineinfo)[i]);
            lua_pushinteger(L, line);
            lua_pushboolean(L, 1);
            lua_settable(L, -3);
        }
    }

    /*
    ** LuaJIT stores nested prototypes as garbage-collectible constants,
    ** iterate over them. See implementation of jit_util_funck in lib_jit.c.
    */
    for (idx = -1; ~idx < (ptrdiff_t) proto->sizekgc; idx--) {
        GCobj *gc = proto_kgc(proto, idx);

        if (~gc->gch.gct == LJ_TPROTO) {
            add_activelines(L, (GCproto *) gc);
        }
    }
}

#endif

static int l_deepactivelines(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_argcheck(L, !lua_iscfunction(L, 1), 1,
        "Lua function expected, got C function");
    lua_settop(L, 1);
    lua_newtable(L);
    add_activelines(L, get_proto(L));
    return 1;
}

int luaopen_cluacov_deepactivelines(lua_State *L) {
    lua_newtable(L);
    lua_pushliteral(L, "0.1.0");
    lua_setfield(L, -2, "version");
    lua_pushcfunction(L, l_deepactivelines);
    lua_setfield(L, -2, "get");
    return 1;
}
