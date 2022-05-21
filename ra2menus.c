#include "g_local.h"

extern arena_link_t *teams;
extern int num_arenas;
extern cvar_t *admincode;

/*******
StringForProtect
*******/
char *StringForProtect(int val)
{
	switch(val) {
	case 0:
		return "Damage all          ";
	case 1:
		return "Dont damage team    ";
	case 2:
		return "Damage self not team";
	}
	return "Damage all          ";
}

/*******
NumForProtect
*******/
int NumForProtect(char *val)
{
	if(!strcmp(val, "Damage all          "))
		return 0;
	if(!strcmp(val, "Dont damage team    "))
		return 1;
	if(!strcmp(val, "Damage self not team"))
		return 2;
	return 0;
}

/*******
menuDoNothing - hold place for something not implemented
*******/
int menuDoNothing(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	return 2; //dont even refresh
}

/*******
menuLeaveArena
******/
int menuLeaveArena(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	arena_state_t *curstate;
	
	curstate=&arenas[((arena_team_t *)(teams[ent->client->resp.team].it))->context].game_state;
	if (*curstate != GAME_COUNT && *curstate != COMP_WAIT && ent->takedamage != DAMAGE_NO) //let observers leave
	{
		menu_centerprint(ent, "Sorry, you cannot leave the arena\nduring a match");
		//		gi.centerprintf(ent, "Sorry, you cannot leave the arena\nduring a match");
		return 2;
	}
	remove_from_queue(&(((arena_team_t *)(teams[ent->client->resp.team].it))->ar), NULL); //take them out of whatever line their in
	SendTeamToArena(&teams[ent->client->resp.team], 0, true, true); //observe
	return 0;
}
/*******
menuAddtoArena - called when an arena is selected from the list
*******/
int menuAddtoArena(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	arena_link_t	*tempitem;
	int	arena=0;
	
	tempitem=menulink->it; //the menu
	while (tempitem->next)
	{
		arena++;
		tempitem=tempitem->next;
		if (tempitem==selected)
			break;
	}
	if (arena)
	{
		if (key==1) //enter
			return AddtoArena(ent, arena, false,false);//dont force, dont observeonly
		else //invdrop
			return AddtoArena(ent,arena,true,true); //force, observe only
	}
	
}


/*******
menuLeaveTeamAr -- leave a team still in the arena
*******/
int menuLeaveTeamAr(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	arena_state_t *curstate;
	
	curstate=&arenas[((arena_team_t *)(teams[ent->client->resp.team].it))->context].game_state;
	if (*curstate != GAME_COUNT && *curstate != COMP_WAIT && ent->takedamage != DAMAGE_NO) //let observers leave
	{
		menu_centerprint(ent, "Sorry, you cannot leave the arena\nduring a match");
		//		gi.centerprintf(ent, "Sorry, you cannot leave the arena\nduring a match");
		return 2;
	}
	remove_from_team(ent);
	move_to_arena(ent, 0, true);
	init_player(ent);
	return 0;
}
/*******
menuLeaveTeam
*******/
int menuLeaveTeam(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	remove_from_team(ent);
	init_player(ent);
	return 0;
}

/*******
menuStepInOutofLine - get in or out of line
*******/
int menuStepInOutofLine(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	int arena;
	qboolean observeonly;
	
	arena = ent->client->resp.context;
	observeonly = !((arena_team_t *)(teams[ent->client->resp.team].it))->observeonly;
	if (menuLeaveArena(ent,NULL,NULL,0)==0)
	{
		return AddtoArena(ent, arena, true,observeonly);// force, use observeonly
	} else 
		return 2;
	
}


/*******
getarenaname - looks for the name from the bsp
*******/
char *getarenaname(int arena)
{
	edict_t	*spot=NULL;
	
	while ((spot = G_Find (spot, FOFS(classname), "info_player_intermission")) != NULL)
		if (spot->arena == arena)
			return spot->message;
		
			/*	newname = gi.TagMalloc(MAXITEMSIZE, TAG_LEVEL);
		sprintf(newname, "Arena Number %d", arena);*/
		return va("Arena Number %d", arena); //since it will get strcpy'd anyways
}

/*******
menuChangeOMode - increment mode
********/
int menuChangeOMode(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
/*if (ent->client->resp.flag != ARENA_OBSERVER)
return 2;
if(ent->client->resp.omode != TRACKCAM &&
ent->client->resp.omode != EYECAM) {
ent->client->resp.old_omode = ent->client->resp.omode;
}
ent->client->resp.omode = (ent->client->resp.omode + 1) % NUM_OMODES;
gi.cprintf(ent, PRINT_HIGH, "Switched Observer Mode to: %s\n", omode_descriptions[ent->client->resp.omode]);
move_to_arena (ent,ent->client->resp.context, 1);
return 0; //clear the menu, since move_to.. puts it back up
	*/
	ChangeOMode(ent);
	//	menu_centerprint(ent,"Use ATTACK to change observer modes\nand JUMP/DUCK to change track target");
	//	gi.centerprintf(ent,"Use ATTACK to change observer modes\nand JUMP/DUCK to change track target");
	return 2;
}




/*******
menuShowSettingsPropose
*******/
int menuShowSettingsPropose(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	int timeleft;
	
	if (arenas[ent->client->resp.context].voteinfo.endtime > level.time)
	{
		timeleft = (int)(arenas[ent->client->resp.context].voteinfo.endtime - level.time);
		if (timeleft  < 30)
			menu_centerprint(ent, va("Voting is in progress.\nPlease wait %d seconds",(int)(arenas[ent->client->resp.context].voteinfo.endtime - level.time)));
		//			gi.centerprintf(ent, "Voting is in progress.\nPlease wait %d seconds",(int)(arenas[ent->client->resp.context].voteinfo.endtime - level.time));
		else
			menu_centerprint(ent, "Voting is in progress.\nPlease wait");
		//			gi.centerprintf(ent, "Voting is in progress.\nPlease wait");
		return 2;
	}
	if (!ent->client->resp.votetries) {
		menu_centerprint(ent, va("Sorry, you cannot propose any more changes.\nYou have already proposed %d times\n", votetries_setting));
		//		gi,centerprintf(ent, va("Sorry, you cannot propose any more changes.\nYou have already proposed %d times\n", votetries_setting));
		return 2;
	}
	ent->client->resp.votetries--;
	Cmd_arenaadmin_f (ent, PROPOSE);
	return 2;
}

/*******
menuShowSettingsVote
*******/
int menuShowSettingsVote(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (arenas[ent->client->resp.context].voteinfo.endtime < level.time)
	{
		menu_centerprint(ent, "No changes have been proposed");
		//		gi.centerprintf(ent, "No changes have been proposed");
		return 2;
	}
	if (ent->client->resp.voted)
	{
		menu_centerprint(ent,"You have already voted");
		//		gi.centerprintf(ent,"You have already voted");
		return 2;
		//		return 0;
	}
	Cmd_arenaadmin_f (ent, VOTE);
	return 2;
}


/*******
show_observer_menu - shows the list of arenas
*******/
void show_observer_menu(edict_t *ent)
{
	arena_link_t	*menul;
	
	menul=CreateQMenu(ent, "Observer Options");
	//AddMenuItem(menul, "Viewing Mode: " ,omode_descriptions[ent->client->resp.omode],-1, &menuChangeOMode);
	//AddMenuItem(menul, "Viewing Mode: " ,"Use ATTACK key to Change",-1, &menuChangeOMode);
	if (!((arena_team_t *)(teams[ent->client->resp.team].it))->observeonly)
	{
		AddMenuItem(menul, "Change Arena Settings" ,NULL,-1, &menuShowSettingsPropose);
		AddMenuItem(menul, "Vote on Changes" ,NULL,-1, &menuShowSettingsVote);
		AddMenuItem(menul,"",NULL,-1,NULL);
	}
	if (!arenas[ent->client->resp.context].pickup) //no team leaving in pickup arenas
	{
		AddMenuItem(menul,va("Step %s Line",((arena_team_t *)(teams[ent->client->resp.team].it))->observeonly ? "into" : "out of"),NULL,-1,&menuStepInOutofLine);
		AddMenuItem(menul,"",NULL,-1,NULL);
	}
	AddMenuItem(menul,"Leave Team",NULL,-1,&menuLeaveTeamAr);
	if (!arenas[ent->client->resp.context].pickup) //no team leaving in pickup arenas
		AddMenuItem(menul,"Leave Arena",NULL,-1,&menuLeaveArena);
	FinishMenu(ent, menul,false);
}
/*******
show_arena_menu - shows the list of arenas
*******/
void show_arena_menu(edict_t *ent)
{
	arena_link_t	*menul;
	int i;
	
	menul=CreateQMenu(ent, "Choose Your Arena");
	for (i=1; i<=num_arenas; i++)
	{
		if (arenas[i].pickup)
			AddMenuItem(menul, getarenaname(i) ," (PT)",-1, &menuAddtoArena);
		else
			AddMenuItem(menul, getarenaname(i) ," T:",count_queue(&arenas[i].unassigned) + count_queue(&arenas[i].players), &menuAddtoArena);
	}
	AddMenuItem(menul,"",NULL,-1,NULL);
	AddMenuItem(menul,"Leave Team",NULL,-1,&menuLeaveTeam);
	FinishMenu(ent, menul,true);
}
/*******
menuAddtoTeam - called when an entity selects a team
*******/
int menuAddtoTeam(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	
	if (add_to_team(ent, ((menu_item_t *)selected->it)->itemtext))
	{
		if (!((arena_team_t *)(teams[ent->client->resp.team].it))->context)
			show_arena_menu(ent); //not in an arena yet
		else {
			//			ent->client->resp.context=((arena_team_t *)(teams[ent->client->resp.team].it))->context;
			//moved to move to arena
			ent->client->resp.flag = ARENA_OBSERVER;
			ent->takedamage = DAMAGE_NO;
			move_to_arena(ent, ((arena_team_t *)(teams[ent->client->resp.team].it))->context, true);
		}
		return 0;
	} else	
	{
		menu_centerprint(ent, "That team is already in an arena\nand full or\nthe arena is locked");
		//		gi.centerprintf(ent, "That team is already in an arena\nand full");
		return 2;
	}
	
	
	
}

/*******
menuNewTeam - when a player selects to start a new team
*******/
int menuNewTeam(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	char *menustr;
	int i=0;
	arena_team_t *ateam;
	
	menustr=gi.TagMalloc(MAXITEMSIZE, TAG_LEVEL);
	
	Com_sprintf(menustr,MAXITEMSIZE, "%s's Team", ent->client->pers.netname);
	//search for other teams with that name
	while (i<MAXTEAMS)
	{
		if(teams[i].it) {
			ateam=teams[i].it;
			if( !strcmp(ateam->teamname, menustr)) 
			{
				strcat(menustr,"!"); //add something to the name if one already exists
				i=0;
			} else
				i++;
		}
		else
			i++;
	}
	
	
	add_to_team(ent, menustr);
	show_arena_menu(ent);
	
	return 0;
}

/*******
menuRefreshTeamList - displays team names from the team array
*******/
int menuRefreshTeamList(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	arena_link_t	*menul;
	int i;
	
	menul=CreateQMenu(ent, "Choose your team");
	//	check_teams(0); not needed since we check teams on leave
	
	AddMenuItem(menul, "Start New Team", NULL, -1, &menuNewTeam);
	for (i=0; i< MAXTEAMS; i++)
	{
		if (teams[i].it)
			AddMenuItem(menul, ((arena_team_t *)(teams[i].it))->teamname, " Players: ", count_queue(&teams[i]), &menuAddtoTeam);
	}
	AddMenuItem(menul, "Refresh List", NULL, -1, &menuRefreshTeamList);
	AddMenuItem(menul, "", NULL, -1, NULL);
	AddMenuItem(menul, "Confused? try /cmd menuhelp", NULL, -1, NULL);
	
	
	
	FinishMenu(ent, menul,true);
	
	return 2;//remove the current one
	
}		

/*******
menuChangeValue - increment or decrement value depending on key
********/
int menuChangeValue(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (key) //inc for enter, dec for '
		((menu_item_t *)selected->it)->itemvalue++;
	else
		((menu_item_t *)selected->it)->itemvalue--;
	
	if (!((menu_item_t *)selected->it)->itemvalue) //if we went to zero
		((menu_item_t *)selected->it)->itemvalue=1;
	return 1;
}

/*******
menuChangeValue10 - increment or decrement value depending on key by 10
********/
int menuChangeValue10(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (key) //inc for enter, dec for '
		((menu_item_t *)selected->it)->itemvalue+=10;
	else
		((menu_item_t *)selected->it)->itemvalue-=10;
	
	if (((menu_item_t *)selected->it)->itemvalue <= 0) //if we went to zero
		((menu_item_t *)selected->it)->itemvalue=10; //min of 10
	return 1;
}

/*******
menuChangeYesNo - switches between yes/no
********/
int menuChangeYesNo(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (((menu_item_t *)selected->it)->valuetext[0] == 'Y')
		strcpy(((menu_item_t *)selected->it)->valuetext,"NO ");
	else
		strcpy(((menu_item_t *)selected->it)->valuetext,"YES");
	return 1;
}

/*******
menuChangeProtect - increment or decrement value depending on key
********/
int menuChangeProtect(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	int val;
	char *str;
	
	val = NumForProtect(((menu_item_t *)selected->it)->valuetext);
	if(key)
		val++;
	else
		val--;
	
	if(val < 0) val = 2;
	if(val > 2) val = 0;
	
	str = StringForProtect(val);
	strcpy(((menu_item_t *)selected->it)->valuetext, str);
	return 1;
}

/*******
menuChangeMap - gets the next map from maploop
********/
int menuChangeMap(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	strcpy(((menu_item_t *)selected->it)->valuetext, get_next_map(((menu_item_t *)selected->it)->valuetext));
	return 1;
}

/*******
cvar_setvalue - sets a cvar to a value
********/
void cvar_setvalue(char *cvar, int value)
{
	char temp[256];
	sprintf(temp, "%d", value);
	gi.cvar_set(cvar,temp);
	
}

/*******
menuApplyAdmin - read in from menu, and sets new admin settings
********/
int menuApplyAdmin(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	edict_t *clent;
	arena_link_t *itemlink;
	menu_item_t *menuitem;
	char *adminmap;
	
	itemlink=(menulink->it);
	while (itemlink->next)
	{
		itemlink=itemlink->next;
		menuitem=itemlink->it;
		if (Q_stricmp(menuitem->itemtext, "Fraglimit:        ")==0)	{
			cvar_setvalue("fraglimit",menuitem->itemvalue);
		} 	else if (Q_stricmp(menuitem->itemtext, "Timelimit:        ")==0) {
			cvar_setvalue("timelimit",menuitem->itemvalue);
		}	else if (Q_stricmp(menuitem->itemtext, "Mapname:          ")==0) {
			adminmap=menuitem->valuetext;
		}
	}
	
	clent = G_Spawn();
	clent->classname = "target_changelevel";
	//clent->map = adminmap; //mem is freed, can't do that!
	clent->map = gi.TagMalloc(strlen(adminmap)+1,TAG_LEVEL);
	strcpy(clent->map,adminmap);
	BeginIntermission (clent);
	return 0;
}

/*******
menuCancel - gets rid of the admin menu
********/
int menuCancel(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	return 0;
}

/*******
Cmd_menuhelp_f - prints some help for the menus
********/
void Cmd_menuhelp_f(edict_t *ent)
{
	gi.cprintf(ent, PRINT_HIGH, "H| Use invprev and invnext ([ and ])\n"
		"H| to navigate the menu\n"
		"H| invuse (ENTER) selects\n"
		"H| inven (TAB) toggles it on/off\n");
	
	
	
}
/*******
Cmd_admin_f - parse admin command and display menu
********/
void Cmd_admin_f (edict_t *ent)
{
	int code;
	arena_link_t *menul,*iteml;
	
	if (admincode->value == 0)
		return;
	code = atoi(gi.argv(1));
	
	if (code == admincode->value)
	{
		//add - allowarenaadminvoting
		menul=CreateQMenu(ent, "Admin Menu");
		AddMenuItem(menul, "Fraglimit:        ",NULL, fraglimit->value, &menuChangeValue);
		AddMenuItem(menul, "Timelimit:        ",NULL, timelimit->value, &menuChangeValue);
		iteml=AddMenuItem(menul, "Mapname:          ","                                ", -1, &menuChangeMap);
		strcpy(((menu_item_t *)(iteml->it))->valuetext,level.mapname);
		AddMenuItem(menul, "",NULL,-1,NULL);
		AddMenuItem(menul, "Apply",NULL,-1,&menuApplyAdmin);
		AddMenuItem(menul, "Cancel", NULL, -1, &menuCancel);
		FinishMenu(ent,menul,true);
		//do admin code stuff
	} else
	{
		gi.cprintf(ent, PRINT_HIGH, "Sorry, incorrect admin code\n");
	}
	
	
	
}

/*******
menuApplyArenaAdmin - read in from menu, and sets new arena settings
********/
extern int weapon_vals[];
int menuApplyArenaAdmin(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	arena_link_t *itemlink;
	menu_item_t *menuitem;
	arena_settings_t *mysettings;
	int newweapons;
	
	int arena;
	
	
	itemlink=(menulink->it);
	while (itemlink->next)
	{
		itemlink=itemlink->next;
		menuitem=itemlink->it;
		if (Q_stricmp(menuitem->itemtext, "Arena:                 ")==0){
			arena = menuitem->itemvalue;
			if (((menu_item_t *)selected->it)->itemtext[0] == 'A') //force apply
				mysettings = &arenas[arena].settings;
			else
			{
				if (arenas[arena].voteinfo.endtime > level.time)
				{
					menu_centerprint(ent, va("Voting is in progress.\nPlease wait %d seconds",(int)(arenas[ent->client->resp.context].voteinfo.endtime - level.time)));
					//						gi.centerprintf(ent, "Voting is in progress.\nPlease wait %d seconds",(int)(arenas[ent->client->resp.context].voteinfo.endtime - level.time));
					return 2;
				}
				memcpy(&arenas[arena].voteinfo.propsettings,&arenas[arena].settings, sizeof(arena_settings_t));
				mysettings = &arenas[arena].voteinfo.propsettings; //propose
				start_voting(ent, arena);
				arenas[arena].voteinfo.ycount++;
				ent->client->resp.voted = true;
			}
			mysettings->changed = 1;
			newweapons = 0; //start with none
			//				mysettings->weapons = 0; //start with none
			if(!mysettings->allow_voting_shotgun )
				newweapons |= (mysettings->weapons & weapon_vals[0]) ? weapon_vals[0] : 0;
			if(!mysettings->allow_voting_supershotgun )
				newweapons |= (mysettings->weapons & weapon_vals[1]) ? weapon_vals[1] : 0;
			if(!mysettings->allow_voting_machinegun )
				newweapons |= (mysettings->weapons & weapon_vals[2]) ? weapon_vals[2] : 0;
			if(!mysettings->allow_voting_chaingun )
				newweapons |= (mysettings->weapons & weapon_vals[3]) ? weapon_vals[3] : 0;
			if(!mysettings->allow_voting_grenadelauncher )
				newweapons |= (mysettings->weapons & weapon_vals[4]) ? weapon_vals[4] : 0;
			if(!mysettings->allow_voting_rocketlauncher )
				newweapons |= (mysettings->weapons & weapon_vals[5]) ? weapon_vals[5] : 0;
			if(!mysettings->allow_voting_hyperblaster )
				newweapons |= (mysettings->weapons & weapon_vals[6]) ? weapon_vals[6] : 0;
			if(!mysettings->allow_voting_railgun )
				newweapons |= (mysettings->weapons & weapon_vals[7]) ? weapon_vals[7] : 0;
			if(!mysettings->allow_voting_bfg )
				newweapons |= (mysettings->weapons & weapon_vals[8]) ? weapon_vals[8] : 0;
			mysettings->weapons = newweapons;
		}   else if (Q_stricmp(menuitem->itemtext, "Players per team:      ")==0){
			mysettings->needed_players_team = menuitem->itemvalue;
		} 	else if (Q_stricmp(menuitem->itemtext, "Initial Health:        ")==0)	{
			mysettings->health = menuitem->itemvalue;
		}	else if (Q_stricmp(menuitem->itemtext, "Initial Armor:         ")==0)	{
			mysettings->armor = menuitem->itemvalue;
		} 	else if (Q_stricmp(menuitem->itemtext, "Minimum Ping:          ")==0)	{
			mysettings->minping = menuitem->itemvalue;
		}	else if (Q_stricmp(menuitem->itemtext, "Maximum Ping:          ")==0)	{
			mysettings->maxping = menuitem->itemvalue;
		}	else if (Q_stricmp(menuitem->itemtext, "Rounds:                ")==0)	{
			mysettings->rounds = (menuitem->itemvalue / 2)*2 + 1; //always odd!
		}	else if (Q_stricmp(menuitem->itemtext, "Allow Shotgun:         ")==0)	{
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[0] : 0;	
		}	else if (Q_stricmp(menuitem->itemtext, "Allow Super Shotgun:   ")==0)	{
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[1] : 0;
		}	else if (Q_stricmp(menuitem->itemtext, "Allow Machine gun:     ")==0) {	
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[2] : 0;
		}	else if (Q_stricmp(menuitem->itemtext, "Allow Chain gun:       ")==0)	{
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[3] : 0;
		}	else if (Q_stricmp(menuitem->itemtext, "Allow Grenade Launcher:")==0)	{
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[4] : 0;
		}	else if (Q_stricmp(menuitem->itemtext, "Allow Rocket Launcher: ")==0)	{
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[5] : 0;
		}	else if (Q_stricmp(menuitem->itemtext, "Allow Hyperblaster:    ")==0)	{
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[6] : 0;
		}	else if (Q_stricmp(menuitem->itemtext, "Allow Railgun:         ")==0)	{
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[7] : 0;
		}	else if (Q_stricmp(menuitem->itemtext, "Allow BFG10K:          ")==0)	{
			mysettings->weapons |= (menuitem->valuetext[0]=='Y') ? weapon_vals[8] : 0;
		}	else if (Q_stricmp(menuitem->itemtext, "Health: ")==0)	{
			mysettings->healthprotect = NumForProtect(menuitem->valuetext);
		}	else if (Q_stricmp(menuitem->itemtext, "Armor:  ")==0)	{
			mysettings->armorprotect = NumForProtect(menuitem->valuetext);
		}	else if (Q_stricmp(menuitem->itemtext, "Falling Damage:        ")==0)	{
			mysettings->fallingdamage = (menuitem->valuetext[0]=='Y');
		}	else if (Q_stricmp(menuitem->itemtext, "Lock Arena:            ")==0)	{
			mysettings->lock_arena = (menuitem->valuetext[0]=='Y');
		}
	}
	check_teams(arena);
	return 0;
}

/*******
menuVote Yes/No
********/
int menuVote(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	int arena;
	
	arena = ent->client->resp.context;
	if (arenas[arena].voteinfo.endtime < level.time)
	{
		menu_centerprint(ent,"Sorry, voting is over");
		//		gi.centerprintf(ent,"Sorry, voting is over");
		return 2;
		//		return 0;
	}
	if (ent->client->resp.voted)
	{
		menu_centerprint(ent,"You have already voted");
		//		gi.centerprintf(ent,"You have already voted");
		return 2;
		//		return 0;
	}
	if (((menu_item_t *)selected->it)->valuetext[0] == 'Y') //yes
		arenas[arena].voteinfo.ycount++;
	else
		arenas[arena].voteinfo.ncount++;
	ent->client->resp.voted = true;
	return 0;
	
}


/*******
Cmd_arenaadmin_f - parse admin command and display menu
********/
void Cmd_arenaadmin_f (edict_t *ent, adminmenu_mode_t mode)
{
	int code;
	arena_link_t *menul;
	int arena=0;
	int	(*mymenuChangeValue) (edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key); 
	int	(*mymenuChangeValue10) (edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key); 
	int	(*mymenuChangeYesNo) (edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key); 
	int	(*mymenuChangeProtect) (edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key); 
	arena_settings_t *mysettings, *cursettings;
	
	switch(mode) 
	{
	case FORCE:
		if (admincode->value == 0)
			return;
		code = atoi(gi.argv(1));
		arena = atoi(gi.argv(2));
		if (code != admincode->value)
			return;
	case PROPOSE:
		if (!arena)
			arena = ent->client->resp.context;
		if (arena < 1 || arena > num_arenas)
			return;
		mymenuChangeValue = &menuChangeValue;
		mymenuChangeValue10 = &menuChangeValue10;
		mymenuChangeYesNo = menuChangeYesNo;
		mymenuChangeProtect = menuChangeProtect;
		mysettings = &arenas[arena].settings;
		break;
		
	case VOTE:
		arena = ent->client->resp.context;
		if (arena < 1 || arena > num_arenas)
			return;
		mymenuChangeValue = mymenuChangeValue10 = mymenuChangeYesNo = mymenuChangeProtect = NULL;
		mysettings = &arenas[arena].voteinfo.propsettings;
		cursettings = &arenas[arena].settings;
		menul=CreateQMenu(ent, "Proposed Changes");
		AddMenuItem(menul, "Arena:                 ",NULL,arena,NULL);
		if (arenas[arena].pickup != 1) {
			if(mysettings->needed_players_team != cursettings->needed_players_team)
				AddMenuItem(menul, "Players per team:      ",NULL, mysettings->needed_players_team, NULL);
		}
		if(mysettings->health != cursettings->health)
			AddMenuItem(menul, "Initial Health:        ",NULL, mysettings->health, mymenuChangeValue10);
		if(mysettings->armor != cursettings->armor)
			AddMenuItem(menul, "Initial Armor:         ",NULL, mysettings->armor, mymenuChangeValue10);
		if (arenas[arena].pickup != 1)
		{
			if(mysettings->minping != cursettings->minping)
				AddMenuItem(menul, "Minimum Ping:          ",NULL, mysettings->minping, mymenuChangeValue10);
			if(mysettings->maxping != cursettings->maxping)
				AddMenuItem(menul, "Maximum Ping:          ",NULL, mysettings->maxping, mymenuChangeValue10);
		}
		if(mysettings->rounds != cursettings->rounds)
			AddMenuItem(menul, "Rounds:                ",NULL, mysettings->rounds, mymenuChangeValue);
		if((mysettings->weapons & weapon_vals[0]) != (cursettings->weapons & weapon_vals[0]))
			AddMenuItem(menul, "Allow Shotgun:         ",(mysettings->weapons & weapon_vals[0]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if((mysettings->weapons & weapon_vals[1]) != (cursettings->weapons & weapon_vals[1]))
			AddMenuItem(menul, "Allow Super Shotgun:   ",(mysettings->weapons & weapon_vals[1]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if((mysettings->weapons & weapon_vals[2]) != (cursettings->weapons & weapon_vals[2]))
			AddMenuItem(menul, "Allow Machine gun:     ",(mysettings->weapons & weapon_vals[2]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if((mysettings->weapons & weapon_vals[3]) != (cursettings->weapons & weapon_vals[3]))
			AddMenuItem(menul, "Allow Chain gun:       ",(mysettings->weapons & weapon_vals[3]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if((mysettings->weapons & weapon_vals[4]) != (cursettings->weapons & weapon_vals[4]))
			AddMenuItem(menul, "Allow Grenade Launcher:",(mysettings->weapons & weapon_vals[4]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if((mysettings->weapons & weapon_vals[5]) != (cursettings->weapons & weapon_vals[5]))
			AddMenuItem(menul, "Allow Rocket Launcher: ",(mysettings->weapons & weapon_vals[5]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if((mysettings->weapons & weapon_vals[6]) != (cursettings->weapons & weapon_vals[6]))
			AddMenuItem(menul, "Allow Hyperblaster:    ",(mysettings->weapons & weapon_vals[6]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if((mysettings->weapons & weapon_vals[7]) != (cursettings->weapons & weapon_vals[7]))
			AddMenuItem(menul, "Allow Railgun:         ",(mysettings->weapons & weapon_vals[7]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if((mysettings->weapons & weapon_vals[8]) != (cursettings->weapons & weapon_vals[8]))
			AddMenuItem(menul, "Allow BFG10K:          ",(mysettings->weapons & weapon_vals[8]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mysettings->healthprotect != cursettings->healthprotect)
			AddMenuItem(menul, "Health: ",StringForProtect(mysettings->healthprotect), -1, mymenuChangeProtect);
		if(mysettings->armorprotect != cursettings->armorprotect)
			AddMenuItem(menul, "Armor:  ",StringForProtect(mysettings->armorprotect), -1, mymenuChangeProtect);
		if(mysettings->fallingdamage != cursettings->fallingdamage)
			AddMenuItem(menul, "Falling Damage:        ",(mysettings->fallingdamage) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		AddMenuItem(menul, "",NULL,-1,NULL);
		break;
	}
	if(mode != VOTE)
	{
		menul=CreateQMenu(ent, "Arena Admin Menu");
		AddMenuItem(menul, "Arena:                 ",NULL,arena,NULL);
		if (arenas[arena].pickup != 1) {
			if(mode == FORCE || mysettings->allow_voting_playersperteam )
				AddMenuItem(menul, "Players per team:      ",NULL, mysettings->needed_players_team, mymenuChangeValue);
		}
		if(mode == FORCE || mysettings->allow_voting_health )
			AddMenuItem(menul, "Initial Health:        ",NULL, mysettings->health, mymenuChangeValue10);
		if(mode == FORCE || mysettings->allow_voting_armor )
			AddMenuItem(menul, "Initial Armor:         ",NULL, mysettings->armor, mymenuChangeValue10);
		if (arenas[arena].pickup != 1)
		{
			if(mode == FORCE || mysettings->allow_voting_minping )
				AddMenuItem(menul, "Minimum Ping:          ",NULL, mysettings->minping, mymenuChangeValue10);
			if(mode == FORCE || mysettings->allow_voting_maxping )
				AddMenuItem(menul, "Maximum Ping:          ",NULL, mysettings->maxping, mymenuChangeValue10);
		}
		if(mode == FORCE || mysettings->allow_voting_rounds )
			AddMenuItem(menul, "Rounds:                ",NULL, mysettings->rounds, mymenuChangeValue); //have to make NO at least 3 chars so that when you strcpy YES into it it fits
		if(mode == FORCE || mysettings->allow_voting_shotgun )
			AddMenuItem(menul, "Allow Shotgun:         ",(mysettings->weapons & weapon_vals[0]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_supershotgun )
			AddMenuItem(menul, "Allow Super Shotgun:   ",(mysettings->weapons & weapon_vals[1]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_machinegun )
			AddMenuItem(menul, "Allow Machine gun:     ",(mysettings->weapons & weapon_vals[2]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_chaingun )
			AddMenuItem(menul, "Allow Chain gun:       ",(mysettings->weapons & weapon_vals[3]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_grenadelauncher )
			AddMenuItem(menul, "Allow Grenade Launcher:",(mysettings->weapons & weapon_vals[4]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_rocketlauncher )
			AddMenuItem(menul, "Allow Rocket Launcher: ",(mysettings->weapons & weapon_vals[5]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_hyperblaster )
			AddMenuItem(menul, "Allow Hyperblaster:    ",(mysettings->weapons & weapon_vals[6]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_railgun )
			AddMenuItem(menul, "Allow Railgun:         ",(mysettings->weapons & weapon_vals[7]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_bfg )
			AddMenuItem(menul, "Allow BFG10K:          ",(mysettings->weapons & weapon_vals[8]) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE || mysettings->allow_voting_healthprotect )
			AddMenuItem(menul, "Health: ",StringForProtect(mysettings->healthprotect), -1, mymenuChangeProtect);
		if(mode == FORCE || mysettings->allow_voting_armorprotect )
			AddMenuItem(menul, "Armor:  ",StringForProtect(mysettings->armorprotect), -1, mymenuChangeProtect);
		if(mode == FORCE || mysettings->allow_voting_fallingdamage )
			AddMenuItem(menul, "Falling Damage:        ",(mysettings->fallingdamage) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		if(mode == FORCE)
			AddMenuItem(menul, "Lock Arena:            ",(mysettings->lock_arena) ? "YES" : "NO ", -1, mymenuChangeYesNo);
		AddMenuItem(menul, "",NULL,-1,NULL);
	}
	switch(mode) 
	{
	case FORCE:
		AddMenuItem(menul, "Apply",NULL,-1,&menuApplyArenaAdmin);
	case PROPOSE:
		AddMenuItem(menul, "Propose",NULL,-1,&menuApplyArenaAdmin);
		break;
	case VOTE:
		AddMenuItem(menul, "Vote ","Yes",-1,&menuVote);
		AddMenuItem(menul, "Vote ","No",-1,&menuVote);
		
		break;
	}
	AddMenuItem(menul, "Cancel", NULL, -1, &menuCancel);
	FinishMenu(ent,menul,true);
}

/*******
menuMotdContinue - clear motd and go on
********/
int menuMotdContinue(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	
	ent->client->pers.showmotd = false;
	menuRefreshTeamList(ent,NULL, NULL, 0);
	return 0;
}

/*******
motd_menu - displays motd if there is one
********/
extern arena_link_t motd;
void motd_menu(edict_t *ent)
{
	arena_link_t *menul, *motdl;
	
	if (!motd.next)
	{
		menuMotdContinue(ent, NULL, NULL, 0);
		return;
    }
	menul=CreateQMenu(ent, "Message of the Day");
	
	//AddMenuItem(menul, "", NULL, -1, NULL);
	AddMenuItem(menul, "---------Continue----------", NULL, -1, &menuMotdContinue);
	
	motdl = &motd;
	while (motdl->next)
	{
		motdl = motdl->next;
		AddMenuItem(menul, motdl->it,NULL, -1, NULL);
	}
	FinishMenu(ent,menul,true);
	
	
}

int menuNo(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	return 0;
}

int menuTeamConfirm(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	AddtoArena(ent, ((menu_item_t *)selected->it)->itemvalue, true,false);
	return 1;
}

/***********
show_teamconfirm_menu - shows a yes no menu
************/
void show_teamconfirm_menu(edict_t *ent, int arena)
{
	arena_link_t *menul;
	
	menul=CreateQMenu(ent, "Confirmation");
	AddMenuItem(menul,"You have too few players",NULL, -1, NULL);
	AddMenuItem(menul,"Do you wish to continue?",NULL, -1, NULL);
	AddMenuItem(menul,"",NULL, -1, NULL);
	AddMenuItem(menul,"Yes, continue to arena ",NULL, arena, &menuTeamConfirm);
	AddMenuItem(menul,"No, choose another", NULL, -1, &menuNo);
	FinishMenu(ent, menul,true);
}

/*************
menu_centerprint

  pops up a menu with a message
*************/
void menu_centerprint(edict_t *ent, char *msg)
{
	char buf[2048];
	arena_link_t *menul;
	char x;
	char *c = msg;
	char *b = buf;
	char *a = NULL;
	char *d = buf;
	int i=0;
	
	if (!ent->client->showmenu) //no menu up
	{
		gi.centerprintf(ent, msg);
		return;
		
	}
	// pop off current menu if it's centerprint
	if(menul = ((arena_link_t *) ent->client->curmenulink)) {
		if(!strcmp( ((arena_link_t *)menul->it)->it, "Message")) {
			//			gi.bprintf(PRINT_HIGH, "!!\n");
			ent->client->menuselectframe = 0;
			UseMenu(ent, 1);
		}
	}
	
	menul=CreateQMenu(ent, "Message");
	AddMenuItem(menul, "---------Continue----------", NULL, -1, &menuNo);
	
	while(x=*c++) {
		*b++ = x;
		i++;
		if(x == ' ' || x == '\n') {
			a = b-1;
			*a = ' ';
		}
		if(i >= 27) {
			if(a)
				*a = '\0';
			else
				*b = '\0';
			AddMenuItem(menul, d, NULL, -1, NULL);
			i -= strlen(d);
			if(a)
				d = a+1;
			else
				d = b;
		}
	}
	*b='\0';
	AddMenuItem(menul, d, NULL, -1, NULL);
	
	//	AddMenuItem(menul, msg, NULL, -1, NULL);
	FinishMenu(ent, menul,true);
}
