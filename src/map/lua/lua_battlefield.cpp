/*
===========================================================================

  Copyright (c) 2010-2015 Darkstar Dev Teams

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/

  This file is part of DarkStar-server source code.

===========================================================================
*/

#include "../../common/showmsg.h"
#include "../../common/timer.h"

#include "lua_battlefield.h"
#include "lua_baseentity.h"
#include "../battlefield.h"
#include "../utils/mobutils.h"
#include "../utils/zoneutils.h"
#include "../status_effect_container.h"


/************************************************************************
*																		*
*  Constructor															*
*																		*
************************************************************************/

CLuaBattlefield::CLuaBattlefield(lua_State *L)
{
    if (!lua_isnil(L, -1))
    {
        m_PLuaBattlefield = (CBattlefield*)(lua_touserdata(L, -1));
        lua_pop(L, 1);
    }
    else
    {
        m_PLuaBattlefield = nullptr;
    }
}

/************************************************************************
*																		*
*  Constructor															*
*																		*
************************************************************************/

CLuaBattlefield::CLuaBattlefield(CBattlefield* PBattlefield)
{
    m_PLuaBattlefield = PBattlefield;
}

/************************************************************************
*                                                                       *
*						Get methods								        *
*                                                                       *
************************************************************************/

inline int32 CLuaBattlefield::getID(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_pushinteger(L, m_PLuaBattlefield->GetID());
    return 1;
}

inline int32 CLuaBattlefield::getArea(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_pushinteger(L, m_PLuaBattlefield->GetArea());
    return 1;
}

inline int32 CLuaBattlefield::getTimeLimit(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_pushinteger(L, std::chrono::duration_cast<std::chrono::seconds>(m_PLuaBattlefield->GetTimeLimit()).count());
    return 1;
}

inline int32 CLuaBattlefield::getTimeInside(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    uint32 duration = std::chrono::duration_cast<std::chrono::seconds>(m_PLuaBattlefield->GetTimeInside()).count();

    lua_pushinteger(L, duration);
    return 1;
}

inline int32 CLuaBattlefield::getRemainingTime(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_pushinteger(L, m_PLuaBattlefield->GetRemainingTime().count());
    return 1;
}

inline int32 CLuaBattlefield::getFightTick(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_pushinteger(L, std::chrono::duration_cast<std::chrono::seconds>(m_PLuaBattlefield->GetFightTime() - m_PLuaBattlefield->GetStartTime()).count());
    return 1;
}

inline int32 CLuaBattlefield::getWipeTime(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    auto count = std::chrono::duration_cast<std::chrono::milliseconds>(get_server_start_time() - m_PLuaBattlefield->GetWipeTime()).count();

    lua_pushinteger(L, count);
    return 1;
}

inline int32 CLuaBattlefield::getFightTime(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_pushinteger(L, std::chrono::duration_cast<std::chrono::milliseconds>(get_server_start_time() - m_PLuaBattlefield->GetFightTime()).count());
    return 1;
}

inline int32 CLuaBattlefield::getPlayers(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_createtable(L, m_PLuaBattlefield->m_PlayerList.size(), 0);
    int8 newTable = lua_gettop(L);
    int i = 1;

    for (auto player : m_PLuaBattlefield->m_PlayerList)
    {
        lua_getglobal(L, CLuaBaseEntity::className);
        lua_pushstring(L, "new");
        lua_gettable(L, -2);
        lua_insert(L, -2);
        lua_pushlightuserdata(L, (void*)m_PLuaBattlefield->GetZone()->GetEntity(player, TYPE_NPC));
        lua_pcall(L, 2, 1, 0);

        lua_rawseti(L, -2, i++);
    }
    return 1;
}

inline int32 CLuaBattlefield::getMobs(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    // do we want just required mobs, all mobs, or just mobs not needed to win
    auto required = lua_isnil(L, 1) ? true : lua_toboolean(L, 1);
    auto adds = lua_isnil(L, 2) ? false : lua_toboolean(L, 2);
    lua_createtable(L, m_PLuaBattlefield->m_EnemyList.size(), 0);   // yes this will create a larger table than results returned
    int8 newTable = lua_gettop(L);
    int i = 1;

    for (auto mob : m_PLuaBattlefield->m_EnemyList)
    {
        CBaseEntity* PMob = nullptr;

        if ((required && mob.condition & CONDITION_SPAWNED_AT_START) || adds)
            PMob = m_PLuaBattlefield->GetZone()->GetEntity(mob.targid, TYPE_MOB | TYPE_PET);

        if (PMob)
        {
            lua_getglobal(L, CLuaBaseEntity::className);
            lua_pushstring(L, "new");
            lua_gettable(L, -2);
            lua_insert(L, -2);
            lua_pushlightuserdata(L, (void*)PMob);
            lua_pcall(L, 2, 1, 0);

            lua_rawseti(L, -2, i++);
        }
    }
    return 1;
}

inline int32 CLuaBattlefield::getNPCs(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_createtable(L, m_PLuaBattlefield->m_NpcList.size(), 0);
    int8 newTable = lua_gettop(L);
    int i = 1;

    for (auto npc : m_PLuaBattlefield->m_NpcList)
    {
        lua_getglobal(L, CLuaBaseEntity::className);
        lua_pushstring(L, "new");
        lua_gettable(L, -2);
        lua_insert(L, -2);
        lua_pushlightuserdata(L, (void*)m_PLuaBattlefield->GetZone()->GetEntity(npc, TYPE_NPC));
        lua_pcall(L, 2, 1, 0);

        lua_rawseti(L, -2, i++);
    }
    return 1;
}

inline int32 CLuaBattlefield::getAllies(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);


    lua_createtable(L, m_PLuaBattlefield->m_AllyList.size(), 0);
    int8 newTable = lua_gettop(L);
    int i = 1;
    for (auto ally : m_PLuaBattlefield->m_AllyList)
    {
        lua_getglobal(L, CLuaBaseEntity::className);
        lua_pushstring(L, "new");
        lua_gettable(L, -2);
        lua_insert(L, -2);
        lua_pushlightuserdata(L, (void*)m_PLuaBattlefield->GetZone()->GetEntity(ally, TYPE_MOB | TYPE_PET));
        lua_pcall(L, 2, 1, 0);

        lua_rawseti(L, -2, i++);
    }

    return 1;
}

inline int32 CLuaBattlefield::getRecord(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    lua_pushstring(L, m_PLuaBattlefield->GetRecord().name.c_str());
    lua_pushinteger(L, std::chrono::duration_cast<std::chrono::milliseconds>(m_PLuaBattlefield->GetRecord().time).count());
    return 1;
}

inline int32 CLuaBattlefield::setWipeTime(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);
    DSP_DEBUG_BREAK_IF(lua_isnil(L, 1) || !lua_isnumber(L, 1));

    m_PLuaBattlefield->SetWipeTime(get_server_start_time() + std::chrono::milliseconds(lua_tointeger(L, 1)));
    return 0;
}

inline int32 CLuaBattlefield::setRecord(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);
    DSP_DEBUG_BREAK_IF(lua_isnil(L, 1) || !lua_isstring(L, 1));
    DSP_DEBUG_BREAK_IF(lua_isnil(L, 2) || !lua_isnumber(L, 2));

    auto name = lua_tostring(L, 1);
    auto time = lua_tointeger(L, 2);

    m_PLuaBattlefield->SetRecord((int8*)name, std::chrono::milliseconds(time));
    return 0;
}

inline int32 CLuaBattlefield::loadMobs(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);
    m_PLuaBattlefield->LoadMobs();
    return 0;
}

inline int32 CLuaBattlefield::loadNPCs(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);
    m_PLuaBattlefield->SpawnTreasureChest();
    return 0;
}

inline int32 CLuaBattlefield::insertEntity(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);
    DSP_DEBUG_BREAK_IF(lua_isnil(L, 1) || !lua_isnumber(L, 1));

    auto targid = lua_tointeger(L, 1);
    bool ally = !lua_isnil(L,2) ? lua_tointeger(L, 2) : false;
    bool inBattlefield = !lua_isnil(L, 3) ? lua_toboolean(L, 3) : false;
    BATTLEFIELDMOBCONDITION conditions = static_cast<BATTLEFIELDMOBCONDITION>(!lua_isnil(L, 4) ? lua_tointeger(L, 4) : 0);

    // entity type
    auto filter = 0x1F;

    auto PEntity = ally ? mobutils::InstantiateAlly(targid, m_PLuaBattlefield->GetZoneID()) : m_PLuaBattlefield->GetZone()->GetEntity(targid, filter);

    if (PEntity)
    {
        m_PLuaBattlefield->InsertEntity(PEntity, inBattlefield, conditions);

        lua_getglobal(L, CLuaBaseEntity::className);
        lua_pushstring(L, "new");
        lua_gettable(L, -2);
        lua_insert(L, -2);
        lua_pushlightuserdata(L, (void*)PEntity);
        lua_pcall(L, 2, 1, 0);
    }
    else
    {
        ShowError(CL_RED "CLuaBattlefield::insertEntity - targid ID %u not found!" CL_RESET, targid);
        lua_pushnil(L);
    }
    return 1;
}

inline int32 CLuaBattlefield::cleanup(lua_State* L)
{
    DSP_DEBUG_BREAK_IF(m_PLuaBattlefield == nullptr);

    auto cleanup = !lua_isnil(L, 1) ? lua_tointeger(L, 1) : false;

    lua_pushboolean(L, m_PLuaBattlefield->CanCleanup(cleanup));
    return 1;
}

/************************************************************************
*																		*
*  declare lua function													*
*																		*
************************************************************************/

const int8 CLuaBattlefield::className[] = "CBattlefield";
Lunar<CLuaBattlefield>::Register_t CLuaBattlefield::methods[] =
{
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getID),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getArea),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getTimeLimit),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getRemainingTime),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getTimeInside),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getFightTick),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getWipeTime),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getFightTime),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getPlayers),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getMobs),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getNPCs),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getAllies),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,getRecord),

    LUNAR_DECLARE_METHOD(CLuaBattlefield,setWipeTime),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,setRecord),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,loadMobs),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,loadNPCs),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,insertEntity),
    LUNAR_DECLARE_METHOD(CLuaBattlefield,cleanup),
    {nullptr,nullptr}
};