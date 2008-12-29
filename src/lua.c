/* 
 *
 *   Copyright (c) 2008-2009 Arthur Huillet 
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

/**
 * This file contains functions related to the Lua scripting interface of FreedroidRPG
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "savestruct.h"

#include "../lua/lua.h"
#include "../lua/lauxlib.h"
#include "../lua/lualib.h"


/* Our Lua state for event execution */
lua_State *global_lua_state;

static int lua_event_teleport(lua_State *L)
{
    const char *label = luaL_checkstring(L, 1);
    location TempLocation;
    ResolveMapLabelOnShip ( label, &TempLocation);
    Teleport(TempLocation . level, TempLocation . x + 0.5, TempLocation . y + 0.5, TRUE);
    return 0;
}

static int lua_event_display_big_message(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    SetNewBigScreenMessage (msg);
    return 0;
}

static int lua_event_display_console_message(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    append_new_game_message (msg);
    return 0;
}

static void event_modify_trigger(const char * name, int state)
{
    event_trigger * target_event = NULL;
    int i;
    for ( i = 0 ; i < MAX_EVENT_TRIGGERS ; i++ )
	{
	if (!strcmp (AllEventTriggers[i].name, name))
	    target_event = &AllEventTriggers[i];
	}

    target_event -> enabled = state;
}

static int lua_event_enable_trigger(lua_State *L)
{
    const char * name = luaL_checkstring(L, 1);
    event_modify_trigger(name, 1);
    return 0;
}

static int lua_event_disable_trigger(lua_State *L)
{
    const char * name = luaL_checkstring(L, 1);
    event_modify_trigger(name, 0);
    return 0;
}

static int event_change_obstacle_type(const char *obslabel, const char *state)
{
    obstacle *our_obstacle = give_pointer_to_obstacle_with_label (obslabel) ;
    int obstacle_level_num = give_level_of_obstacle_with_label (obslabel) ;
    level *obstacle_level = curShip . AllLevels [ obstacle_level_num ] ;

    if (state != NULL) {
	int j;
	int base  = obstacle_level -> obstacle_statelist_base  [ our_obstacle -> name_index ] ;
	int count = obstacle_level -> obstacle_statelist_count [ our_obstacle -> name_index ] ;
	for (j=0; j<count; j++)
	    if ( !strcmp ( obstacle_level->obstacle_states_names [ j+base ], state) ) {
		our_obstacle -> type = obstacle_level->obstacle_states_values [ j+base ];
		break;
	    }
    } else {
	action_remove_obstacle (obstacle_level, our_obstacle);
    }

    //--------------------
    // Now we make sure the door lists and that are all updated...
    GetAllAnimatedMapTiles (obstacle_level) ;

    //--------------------
    // Also make sure the other maps realize the change too, if it
    // maybe happend in the border area where two maps are glued together
    // only export if the obstacle falls within the interface zone

    if( our_obstacle->pos.x <= obstacle_level->jump_threshold_west ||
	    our_obstacle->pos.x >= obstacle_level->xlen - obstacle_level->jump_threshold_east ||
	    our_obstacle->pos.y <= obstacle_level->jump_threshold_north ||
	    our_obstacle->pos.y >= obstacle_level->ylen - obstacle_level->jump_threshold_south
      ) 
	ExportLevelInterface ( obstacle_level_num ) ;

    return 0;
}

static int lua_event_change_obstacle(lua_State *L) 
{
    const char *obslabel = luaL_checkstring(L, 1);
    const char *type = luaL_checkstring(L, 2);
    event_change_obstacle_type(obslabel, type);
    return 0;
}

static int lua_event_delete_obstacle(lua_State *L)
{
    const char *obslabel = luaL_checkstring(L, 1);
    event_change_obstacle_type(obslabel, NULL);
    return 0;
}

static int lua_event_heal_tux(lua_State *L)
{
    Me . energy = Me . maxenergy;
    return 0;
}

static int lua_event_kill_tux(lua_State *L)
{
    Me . energy = -100;
    return 0;
}

static int lua_event_hurt_tux(lua_State *L)
{
    int hp = luaL_checkinteger(L, 1);
    Me . energy -= hp;
    return 0;
}

static int lua_event_improve_skill(lua_State *L)
{
    const char *skilltype = luaL_checkstring(L, 1);
    int *skillptr = NULL;
    if(!strcmp(skilltype, "melee")) {
	skillptr = &Me . melee_weapon_skill;
	SetNewBigScreenMessage(_("Melee fighting ability improved!"));
    } else if (!strcmp(skilltype, "ranged")) {
	skillptr = &Me . ranged_weapon_skill;
	SetNewBigScreenMessage(_("Ranged combat ability improved!"));
    } else if (!strcmp(skilltype, "programming")) {
	skillptr = &Me . spellcasting_skill;
	SetNewBigScreenMessage(_("Programming ability improved!"));
    } else {
	ErrorMessage(__FUNCTION__, "Lua script called me with an incorrect parameter. Accepted values are \"melee\", \"ranged\", and \"programming\".\n", PLEASE_INFORM, IS_WARNING_ONLY);
    }

    if (skillptr) {
	ImproveSkill(skillptr);
    }
    return 0;
}

static int lua_event_improve_program(lua_State *L)
{
    const char *pname = luaL_checkstring(L, 1);
    Me . base_skill_level [ get_program_index_with_name(pname) ] ++;
    return 0;
}

static int lua_event_delete_item(lua_State *L)
{
    const char *itemname = luaL_checkstring(L, 1);
    int mult = luaL_checkinteger(L, 2);
    DeleteInventoryItemsOfType(GetItemIndexByName(itemname), mult);
    return 0;
}

static int lua_event_give_item(lua_State *L)
{
    const char *itemname = luaL_checkstring(L, 1);
    int mult = luaL_checkinteger(L, 2);

    item NewItem;
    NewItem.type = GetItemIndexByName(itemname);
    NewItem.prefix_code = (-1);
    NewItem.suffix_code = (-1);
    FillInItemProperties ( &NewItem , TRUE , 1);
    NewItem.multiplicity = mult;

    //--------------------
    // Either we put the new item directly into inventory or we issue a warning
    // that there is no room and then drop the item to the floor directly under 
    // the current Tux position.  That can't fail, right?
    //
    if ( !TryToIntegrateItemIntoInventory ( & NewItem , NewItem.multiplicity ) )
	{
	DropItemToTheFloor ( &NewItem , Me . pos . x , Me . pos . y , Me . pos. z ) ;
	SetNewBigScreenMessage( _("1 Item received (on floor)") );
	}
    else
	{
	SetNewBigScreenMessage( _("1 Item received!") );
	}
    return 0;
}

static int lua_event_open_diary_entry(lua_State *L)
{
    int mis_num = luaL_checkinteger(L, 1);
    int mis_diary_entry_num = luaL_checkinteger(L, 2);

    quest_browser_enable_new_diary_entry ( mis_num , mis_diary_entry_num );
    return 0;
}

static int lua_event_plant_cookie(lua_State *L)
{
    const char *cookie = luaL_checkstring(L, 1);
    PlantCookie(cookie);
    return 0;
}

static int lua_event_remove_cookie(lua_State *L)
{
    const char *cookie = luaL_checkstring(L, 1);
    DeleteCookie(cookie);
    return 0;
}

static int lua_event_assign_mission(lua_State *L)
{
    int misnum = luaL_checkinteger(L, 1);
    AssignMission(misnum);
    return 0;
}

static int lua_event_complete_mission(lua_State *L)
{
    int misnum = luaL_checkinteger(L, 1);
    Me . AllMissions[ misnum ] . MissionIsComplete = TRUE;
    return 0;
}

static int lua_event_give_xp(lua_State *L)
{
    int xp = luaL_checkinteger(L, 1);
    char tmpstr[150];
    Me . Experience += xp;
    sprintf( tmpstr , _("+%d Experience Points") , xp );
    SetNewBigScreenMessage ( tmpstr );
    return 0;
}

static int lua_event_eat_training_points(lua_State *L)
{
    int nb = luaL_checkinteger(L, 1);
    char tmpstr[150];
    Me . points_to_distribute -= nb;
    sprintf(tmpstr, _("%d training points spent!"), nb);
    SetNewBigScreenMessage(tmpstr);
    return 0;
}

static int lua_event_add_gold(lua_State *L)
{
    int nb = luaL_checkinteger(L, 1);
    char tmpstr[150];
    Me . Gold += nb;
    if (nb > 0)
	sprintf(tmpstr, _("Gained %d bucks!"), nb);
    else sprintf(tmpstr, _("Lost %d bucks!"), nb);

    SetNewBigScreenMessage(tmpstr);
    return 0;
}

static int lua_event_change_stat(lua_State *L)
{
    const char *characteristic = luaL_checkstring(L, 1);
    int nb = luaL_checkinteger(L, 2);
    int * statptr = NULL;

    if (!strcmp(characteristic, "strength")) {
	statptr = &Me.base_strength; 
    } else if (!strcmp(characteristic, "dexterity")) {
	statptr = &Me.base_dexterity; 
    } else if (!strcmp(characteristic, "CPU")) {
	statptr = &Me.base_magic; 
    } else if (!strcmp(characteristic, "vitality")) {
	statptr = &Me.base_vitality;
    } else {
	ErrorMessage(__FUNCTION__, "I was called with characteristic name %s - accepted values are \"strength\", \"dexterity\", \"CPU\", and \"vitality\".", PLEASE_INFORM, IS_WARNING_ONLY, characteristic);
	return 0;
    }

    *statptr += nb;
    return 0;
}

static int lua_event_respawn_level(lua_State *L)
{
    int lnb = luaL_checkinteger(L, 1);

    respawn_level(lnb);

    return 0;
}

static int lua_event_trade_with(lua_State *L)
{
    const char *cname = luaL_checkstring(L, 1);
 
    int id = ResolveDialogSectionToChatFlagsIndex(cname);
    InitTradeWithCharacter(id);
 
    return 0;
}

static int lua_chat_set_next_node(lua_State *L)
{
    int nodenb = luaL_checkint(L, 1);

    chat_control_next_node = nodenb;

    return 0;
}

static int lua_chat_end_dialog(lua_State *L)
{
    chat_control_end_dialog = 1;
    return 0;
}

static int lua_chat_break_off_and_attack(lua_State *L)
{
    chat_control_chat_droid->is_friendly = FALSE;
    chat_control_end_dialog = 1;
    return 0;
}

static int lua_chat_drop_dead(lua_State *L)
{
    hit_enemy(chat_control_chat_droid, chat_control_chat_droid->energy + 1, 0, -1, 0);
    chat_control_end_dialog = 1;
    return 0;
}

static int lua_chat_everybody_hostile(lua_State *L)
{
    enemy *erot;
    BROWSE_ALIVE_BOTS(erot)	
	{
	erot -> is_friendly = FALSE ;
	}
    SwitchBackgroundMusicTo(BIGFIGHT_BACKGROUND_MUSIC_SOUND);
    return 0;
}

static int lua_chat_set_bot_state(lua_State *L)
{
    const char *cmd = luaL_checkstring(L, 1);
    if (!strcmp(cmd, "follow_tux")) {
	chat_control_chat_droid->follow_tux = TRUE;
	chat_control_chat_droid->CompletelyFixed = FALSE;
    } else if (!strcmp(cmd, "fixed")) {
	chat_control_chat_droid->follow_tux = FALSE;
	chat_control_chat_droid->CompletelyFixed = TRUE;
    } else if (!strcmp(cmd, "free")) {
	chat_control_chat_droid->follow_tux = FALSE;
	chat_control_chat_droid->CompletelyFixed = FALSE;
    } else {
	ErrorMessage(__FUNCTION__, "I was called with an invalid state namei %s. Accepted values are \"follow_tux\", \"fixed\" and \"free\".\n", PLEASE_INFORM, IS_FATAL, cmd);
    }

    return 0;
}

static int lua_chat_make_tux_red_guard(lua_State *L)
{
    /*XXX get rid of this*/
    Me . is_town_guard_member = TRUE ;
    return 0;
}


luaL_reg lfuncs[] = {
    { "teleport", lua_event_teleport },
    { "display_big_message", lua_event_display_big_message },
    { "display_console_message", lua_event_display_console_message },
    { "enable_trigger", lua_event_enable_trigger },
    { "disable_trigger", lua_event_disable_trigger },
    { "change_obstacle", lua_event_change_obstacle },
    { "delete_obstacle", lua_event_delete_obstacle },
    { "kill_tux", lua_event_kill_tux },
    { "heal_tux", lua_event_heal_tux },
    { "hurt_tux", lua_event_hurt_tux },
    { "improve_skill", lua_event_improve_skill },
    { "improve_program", lua_event_improve_program },
    { "delete_item", lua_event_delete_item },
    { "give_item", lua_event_give_item },
    { "open_diary_entry", lua_event_open_diary_entry },
    { "plant_cookie", lua_event_plant_cookie },
    { "remove_cookie", lua_event_remove_cookie },
    { "assign_mission", lua_event_assign_mission },
    { "finish_mission", lua_event_complete_mission },
    { "give_xp", lua_event_give_xp },
    { "eat_training_points", lua_event_eat_training_points },
    { "add_gold", lua_event_add_gold },
    { "change_stat", lua_event_change_stat },
    { "respawn_level", lua_event_respawn_level },
    { "trade_with", lua_event_trade_with },

    { "set_next_node", lua_chat_set_next_node },
    { "end_dialog", lua_chat_end_dialog },
    { "break_off_and_attack", lua_chat_break_off_and_attack },
    { "drop_dead", lua_chat_drop_dead },
    { "everybody_hostile", lua_chat_everybody_hostile },
    { "set_bot_state", lua_chat_set_bot_state },
    { "make_tux_red_guard", lua_chat_make_tux_red_guard },

    { NULL, NULL },
};

void run_lua(const char * code) 
{
    if(luaL_dostring(global_lua_state, code)) {
	ErrorMessage(__FUNCTION__, "Error running Lua code {%s}: %s.\n", PLEASE_INFORM, IS_FATAL, code, lua_tostring(global_lua_state, -1));
    }
}

void init_lua()
{
    char fpath[2048];
    int i;

    global_lua_state = lua_open();
    luaL_openlibs(global_lua_state);

    if (!find_file("script_helpers.lua", MAP_DIR, fpath, 1))
	if (luaL_dofile(global_lua_state, fpath ))
	    ErrorMessage(__FUNCTION__, "Cannot open script helpers file script_helpers.lua: %s.\n", PLEASE_INFORM, IS_FATAL, lua_tostring(global_lua_state, -1));

    for (i=0; lfuncs[i].name != NULL; i++) {
	lua_pushcfunction(global_lua_state, lfuncs[i].func);
	lua_setglobal(global_lua_state, lfuncs[i].name);
    }
}


