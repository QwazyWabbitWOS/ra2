//include <stdio.h>
//include <sys/stat.h>
#include "g_local.h"

int broken = 0;

/*static*/ arena_obj_t arenas[MAXARENAS];
/*static */int num_arenas;// = 4;//1;      // ** needs to be loaded from map or config info
arena_link_t *teams;            // buffer of teams to [max_teams]
arena_link_t motd;
qboolean idmap;
cvar_t *admincode;

char *teamskins[]={"r2red","r2blue","r2dgre","r2oran","r2yell","r2aqua","r2lgre"};
char *vwepmodels[]={"male","female","cyborg","crakhor"};
int teamskins_precachem[NUMSKINS], teamskins_precachef[NUMSKINS], teamskins_precachecw[NUMSKINS], teamskins_precachecb[NUMSKINS];
/* todo:

-??* Fix spawning in same spot -- has to do with telefrag_time and protection
-??* Fix allow voting
-* Fix health/armor damage (documentation)
* Fix "getting stuck"
* Tell bot person they are a bot
-* falling damage voting
  
	
10/10/98:
-* Added "logname" and gamedir logging in gslog.c, default is stdlog.log in arena dir
-* Pick random team for even/odd spots
-* Lock Arenas menu option/config file option
* If command stats with / don't world print it
-* Moved countdown to off center
-* Countdown doesn't make menu disappear
-* checks at end of 15 sec to make sure both teams have players
  
5/18/98:
-* Spawn one team at even numbered spawnpoints, other team at odd for pickup
-* Add grappling hook and grappling hook server var
-* Add health and armor protect to voting menu
-* Only show changed items in voting menu
-* Allow config file to specify things that cant be voted on
-* Allow votetries to be specified in the config file
-* Hand grenade held too long keeps you ticking?
-* Fix telefrag protection on lifts
-* Observers telefrag players bug
* Bug with dying after match on tiegames?
-* Crackhor and cyborg skins
-* Player remaining count on status bars in pickup

* when one teammate leaves a map the observer menus on the others dont get cleared
* mem leak cause menus are cleared when player dies?
* fix "players" command???
  * get rid of needed_teams from arena object structure
* Level voting
* Betting
* Better support for 1v1 play (more info, less menus)
* Server linking
* Tournament Play:
* Allow map makers to define default weapons for their levels
* when walking onto a teleporter it creates a team for normal arena, or puts you on a pickup team for pickup arena
* When timelimit is hit, freeze arenas so no new matches start 
* grapple?
* Non-fixed teams.  When match starts, for each new team in arena
* On the arena selection menu, it should list basic arena information
* When voting on changes, there should be some indication exactly wha


--* id maps w/o config file default to pickup
--* Fixed ID line-of-sight bug
--* fixed no lastframe bug.. caused by gib when holding hand grenade down, set granade_time to 0
--* teammates can now telefrag each other
--* GLOB problem mostly taken care of.. may be some left, but not as big.
--* took change observer mode off menu
--* moved the setting of context to move_to_Arena, so observer menu isnt sent multiple times
--* rewrote tracknext to go any direction
--* added ID on statusbar for observers
--* single buffer for menus, keeps from getting overruns
--* move multi_arena_think ahead of clientendserverframe
--* 0.5 sec delay between menuselects, keeps people from double clicking
--* vote timer doesn't actually start until match is over, so everyone can vote
--* fixed stufftext play cmd if sound is off
--* default talk mode if nothing type is say_world
--* 10 sec countdown if vote in progress
--* rewrote menu code to allocate/deallocate mem correctly
--* added voting so und
--* fix say_team
--* fatality sound, animality on kill self?
--* check public 1 before setting netlog
--* cant move view in railgun22 arena 2? (removed svf_teleport from respawn)
--* fixedtime cheat?
--* level change from admin menu crashes? ?? 
--* Better support for id maps (less extra menus)
--* Chat only in your arena
--* Team scoring
--* observers use teleporters
--* new scoreboard
--* Default skins and support for custom skins, lock skins for each team
--  set one team to male, other to female
--  set colors starting from 0 on one team, max on the other
--* default ammo amounts for arena
--* unload config file when done with it
--* time between matches
--* add all settings to arena config menu
--* move arena_init to end of SpawnEntities
--* reload default settings when all teams leave arena
--* check pings in check teams
--* leave team (without leaving arena)
---* make invnext and invprev skip over blank menu items
---* move playersperteam cvar to maploop.c
--* max teams per arena
--* round counter
--* use teleporters
--* Trackcam
 --*Free flying (like observer mode on id maps now)
 --*Trackcam (whatever you think is best)
 --*In eyes view (if possible, try a simple method first)
---* Player status for people playing
---* Admin on the arena level
--* MAKE Option for not hurting teammates
---* MOTD
---* Logfile support
---* idmap in per arena basis
---* get showrank to work on the statusbar
****/

#ifdef MEMDEBUG
#include "malloc.h"
#include <crtdbg.h>

void crashme()
{
	strcpy((char *)0x0,"this string will crash the program I hope, so I can debug it");
}

void checkheap( void )
{   

   if (!_CrtCheckMemory( ))
		crashme();
/*	if ( _heapchk() != _HEAPOK )
		crashme();*/
}


void *myTagMalloc(int size, int tag)
{
	void *newRec;

	checkheap();
if ((newRec = _malloc_dbg(size, _NORMAL_BLOCK, 
        __FILE__, __LINE__)) == NULL)
		crashme();
	return newRec;
}
void myTagFree(void *p)
{
	checkheap();
	_free_dbg(p,_NORMAL_BLOCK);	
}

#endif

/*******
add_to_queue
*******/
void add_to_queue(arena_link_t *t, arena_link_t *que)
{
	while(que->next)
		que=que->next;
	que->next = t;
	t->prev = que;
	t->next = NULL;
}

/*******
remove_from_queue

if NULL is given as first parameter, top list item is popped off

item that is removed is returned, or NULL if not found
*******/
arena_link_t *remove_from_queue(arena_link_t *t, arena_link_t *que)
{
	arena_link_t *got = NULL;

	if(!t) 
		if (que)
			t = que->next;
	if(!t) return(NULL);
	if (t->prev)
		t->prev->next = t->next;
	if(t->next)
		t->next->prev = t->prev;
t->next=t->prev=NULL;
	return(t);
}

/*******
add_to_front_queue
*******/
void add_to_front_queue(arena_link_t *t, arena_link_t *que)
{
	remove_from_queue(t, NULL);
	t->prev = que;
	t->next = que->next;
	if (que->next)
		que->next->prev = t;
	que->next = t;
}

/*******
count_queue
*******/
int count_queue(arena_link_t *que)
{
	int i = 0;

	while(que->next) {
		que=que->next;
		i++;
	}

	return(i);
}

/*******
count_players_queue
*******/
int count_players_queue(arena_link_t *que)
{
	int i = 0;

	while(que->next) {
		que=que->next;
		if(((edict_t *)que->it)->client->resp.flag == ARENA_PLAYER) {
			i++;
		}
	}

	return(i);
}

/*******
set_damage

sets players damage flag
*********/
void set_damage(int context, damage_t dtype)
{
	arena_link_t *t, *p;

	t = &arenas[context].players;

	while (t->next) //cycle teams
	{
		t=t->next;
		p=t->it;
		while (p->next) //cycle players
		{
			p=p->next;
			if (((edict_t *)p->it)->client->resp.flag != ARENA_OBSERVER) //dont set damage for teammates that are still observers (i.e. joined after count started)
				((edict_t *)p->it)->takedamage = dtype;
		}
	}

}

extern gitem_armor_t bodyarmor_info;
extern int weapon_vals[];

/*******
give_ammo

** ammo should be moved to the arena object struct at some point so that
   ammo can be set per arena
*******/
void give_ammo(edict_t *e)
{
	arena_obj_t *a = &arenas[e->client->resp.context];
//	edict_t *e;
	qboolean	needswitch;
	gitem_t *w[9];
	gitem_t *it, *rl;
	int i;
			//	  0  2  3  4  5   6    9   8   7
	int weapon_vals_x[] = { 256, 1, 2, 4, 8, 16, 128, 64, 32 };

// give health
	if(a->settings.health)
		e->health = a->settings.health;
	else
		e->health = 100;

// give weapons
	rl = NULL;
	memset(w, 0, sizeof(w));
	w[0] = FindItemByClassname("weapon_bfg");
	w[1] = FindItemByClassname("weapon_shotgun");
	w[2] = FindItemByClassname("weapon_supershotgun");
	w[3] = FindItemByClassname("weapon_machinegun");
	w[4] = FindItemByClassname("weapon_chaingun");
	w[5] = FindItemByClassname("weapon_grenadelauncher");
	w[6] = FindItemByClassname("weapon_railgun");
	w[7] = FindItemByClassname("weapon_hyperblaster");
	w[8] = FindItemByClassname("weapon_rocketlauncher");
	needswitch = false;
	for(i=8; i>=0; i--) {
		if(a->settings.weapons & weapon_vals_x[i]) {
			if(!rl) {
				rl = w[i];
			}
			if(!e->client->pers.inventory[ITEM_INDEX(rl)] || needswitch) {
				e->client->newweapon = rl;
				e->client->pers.selected_item = e->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(rl);
				needswitch = false;
			}
			e->client->pers.inventory[ITEM_INDEX(w[i])] = 1;
		} else {
			if(e->client->pers.weapon == w[i]) {
				needswitch = true;
			}
			e->client->pers.inventory[ITEM_INDEX(w[i])] = 0;
		}
	}
	if(needswitch) {
		rl = FindItemByClassname("weapon_blaster");
		e->client->newweapon = rl;
		e->client->pers.selected_item = e->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(rl);
	}		
// give ammo
	if(it = FindItemByClassname("ammo_shells")) e->client->pers.inventory[ITEM_INDEX(it)] = a->settings.shells;	//shells	100
	if(it = FindItemByClassname("ammo_bullets")) e->client->pers.inventory[ITEM_INDEX(it)] = a->settings.bullets;	//bullets       200
	if(it = FindItemByClassname("ammo_slugs")) e->client->pers.inventory[ITEM_INDEX(it)] = a->settings.slugs;	//slugs	 50
	if(it = FindItemByClassname("ammo_grenades")) e->client->pers.inventory[ITEM_INDEX(it)] = a->settings.grenades;	//grenades      50
	if(it = FindItemByClassname("ammo_rockets")) e->client->pers.inventory[ITEM_INDEX(it)] = a->settings.rockets;	//rockets       50
	if(it = FindItemByClassname("ammo_cells")) e->client->pers.inventory[ITEM_INDEX(it)] = a->settings.cells;	//cells	 200
// give body armor
	if(it = FindItemByClassname("item_armor_body")) {
//		if(a->settings.armor) {
			e->client->pers.inventory[ITEM_INDEX(it)] = a->settings.armor;
//		} else {
//			e->client->pers.inventory[ITEM_INDEX(it)] = bodyarmor_info.max_count;
//		}
	}
	if(allow_grapple)
		if(it = FindItem("Grapple")) e->client->pers.inventory[ITEM_INDEX(it)] = 1;
}


/*******
add_to_team
*******/
arena_team_t *add_to_team(edict_t *e, char *teamname)
{
	int i;
	
	arena_team_t *ateam;



	for(i=0; i<MAXTEAMS; i++) {
		if(teams[i].it) {
			ateam=teams[i].it;
			if( !strcmp(ateam->teamname, teamname)) {

				if (ateam->context)
				{
					if ( count_queue(&teams[i]) == arenas[ateam->context].settings.needed_players_team)
						return NULL; //don't join if already in an arena, and they are full
					if (arenas[ateam->context].settings.lock_arena) 
						return NULL; //arena is locked
				}

				add_to_queue(&e->client->resp.ar, &teams[i]);
				e->client->resp.team = i;
				if (ateam->skin != -1)
					setteamskin(e,e->client->pers.userinfo,ateam->skin);

			gi.bprintf(PRINT_MEDIUM, "%s has been added to team %d (%s)\n", e->client->pers.netname, i, teamname);
				return ateam;
			}
		}
	}

// make the team
	for(i=0; i<MAXTEAMS; ) {
		if(!teams[i].it)
			break;
		i++;
	}
	/*ateam = malloc(sizeof(arena_team_t));*/
	ateam = gi.TagMalloc(sizeof(arena_team_t), TAG_LEVEL);
	if (!ateam)
	{
		gi.error("Ateam malloc failed!\n");
		return NULL;
	}

	ateam->teamname = teamname;
	ateam->teamnum = i;
	ateam->context = 0; //always create teams in the lobby
	ateam->wins = -1;
	ateam->total_wins = 0;
	ateam->skin = -1;
	ateam->ar.it = &teams[i]; //point back  to myself
	teams[i].it = ateam;
	if (e) //adding an entity
	{
		add_to_queue(&ateam->ar, &arenas[0].unassigned); //add the team to the unassigned in lobby
		ateam->pickup = 0;
		ateam->side = -1;
		add_to_queue(&e->client->resp.ar, &teams[i]);
		e->client->resp.team = i;
		gi.bprintf(PRINT_MEDIUM, "%s has created team number %d (%s)\n", e->client->pers.netname, i, teamname);
	} else //must be a pickup team
	{
		ateam->pickup = 1;
	}
	return ateam;
}

/*******
remove_from_team
*******/
void remove_from_team(edict_t *e) //should we clear the team?
{
	arena_link_t *t;
	char *teamname;

	if(e->client->resp.team < 0) return;
	t = &e->client->resp.ar;

	teamname = ((arena_team_t *)teams[e->client->resp.team].it)->teamname;

	gi.bprintf(PRINT_MEDIUM, "%s has been removed from team %d (%s)\n", e->client->pers.netname,e->client->resp.team, teamname);

	remove_from_queue(t, NULL);
	check_teams(e->client->resp.context);
	e->client->resp.team = -1;
}


extern float	PlayersRangeFromSpot (edict_t *spot);

edict_t *SelectRandomArenaSpawnPoint (char *classn, int arena, int evenodd)
{
	edict_t	*spot;
	int		count = 0;
	int		selection;

	spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), classn)) != NULL)
	{
		if (spot->arena != arena && idmap==false) continue;
		count++;
	}

	if (!count)
		return NULL;

	selection = rand() % count;

	if(evenodd) {
		selection = selection & -2;
		if(evenodd == 1) {
			selection++;
			if(selection >= count)
				selection = 1;
		}
	}

//	gi.dprintf("%d spots, %d selected (%d evenodd)\n",count,selection,evenodd);

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname),classn);
		if (spot->arena != arena && idmap==false) 
			selection++;
	} while(selection--);

	return spot;
}

/*******
SelectFarthestArenaSpawnPoint - does what it says
******/
edict_t *SelectFarthestArenaSpawnPoint (char *classn, int arena)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;
	int		count = 0;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 50;
	while ((spot = G_Find (spot, FOFS(classname), classn)) != NULL)
	{
		//gi.bprintf (PRINT_HIGH,"arena %d spot %d\n", arena, spot->arena);
		if (spot->arena != arena && idmap==false) continue;
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	return SelectRandomArenaSpawnPoint (classn, arena, 0);
}

void track_SetStats(edict_t *ent)
{
	edict_t *targ = ent->client->resp.track_targ;
	int f = ent->client->resp.score;	// remember our frags
	
	memcpy(ent->client->ps.stats,targ->client->ps.stats, sizeof(targ->client->ps.stats[0])*MAX_STATS);
	ent->client->ps.stats[STAT_FRAGS] = f;
	if (ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	else
			ent->client->ps.stats[STAT_LAYOUTS] &= ~1;
	CTFSetIDView(ent);
}

void eyecam_think(edict_t *ent, usercmd_t *ucmd)
{
	qboolean blocked = false;
	int i;
	vec3_t look, delta;
	vec3_t odd = { 0, 0, 0 };
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	edict_t *targ = ent->client->resp.track_targ;

	if(!targ || targ->client->resp.flag != ARENA_PLAYER) {
		track_next(ent);
		return;
	}

	gi.unlinkentity(ent);
		VectorCopy(targ->s.origin, ent->s.origin);
	AngleVectors(targ->client->v_angle, look, NULL, NULL);
	VectorScale(look, 20, delta);
	VectorAdd(ent->s.origin, delta, ent->s.origin); //move the view out in front
		ent->s.origin[2] += 22;	// add the view height 
		VectorCopy(odd, ent->velocity);
	// set our angles
	VectorCopy(targ->client->v_angle, ent->s.angles);

	VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
	VectorCopy(targ->client->v_angle, ent->client->v_angle);
	for(i=0; i<3; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->s.angles[i] - ent->client->resp.cmd_angles[i]);

	gi.linkentity(ent);
	//set stats
	track_SetStats(ent);
}

void track_think(edict_t *ent, usercmd_t *ucmd)
{
	qboolean blocked = false;
	int i;
	vec3_t look, delta;
	vec3_t odd = { 0, 0, 0 };
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	trace_t trace;
	edict_t *targ = ent->client->resp.track_targ;

	if(!targ || targ->client->resp.flag != ARENA_PLAYER) {
		track_next(ent);
		return;
	}

	VectorCopy(ent->client->ps.viewangles, look);
	AngleVectors(look, look, NULL, NULL);
	VectorScale(look, 150, delta);
	VectorSubtract(targ->s.origin, delta, delta);
	// check to make sure we dont hit a wall
	trace = gi.trace(targ->s.origin, odd, odd, delta, targ, MASK_SOLID);
	if(trace.fraction < 1.0) {
		VectorScale(look, -130 * trace.fraction, delta);
		VectorAdd(targ->s.origin, delta, delta);
	}
	// if we're blocked, just set their origin, otherwise, set their velocity
	trace = gi.trace(ent->s.origin, mins, maxs, ent->s.origin, ent, MASK_PLAYERSOLID);
	if(trace.contents & (CONTENTS_SOLID | CONTENTS_WINDOW)) blocked = true;
	if(!blocked)
		trace = gi.trace(ent->s.origin, mins, maxs, delta, ent, MASK_SOLID);
	if(trace.fraction < 1.0 || blocked) {
		gi.unlinkentity(ent);
		VectorCopy(delta, ent->s.origin);
		gi.linkentity(ent);
		VectorCopy(odd, ent->velocity);
	} else {
		VectorSubtract(delta, ent->s.origin, delta);	// find the distance
		for(i=0; i<3; i++) {
			ent->velocity[i] = delta[i] * 10;	// get there in 1/10th the time
		}
	}
	//set stats
	track_SetStats(ent);
}

void track_change(edict_t *ent, int value)
{
	qboolean fail = false;
	edict_t *targ;
	edict_t *pos;
	int i;

	targ = ent->client->resp.track_targ;
	if (!targ)
	{
		targ = g_edicts + 1;
		fail = true;
	} else
		if(targ->client->resp.flag != ARENA_PLAYER ||
            targ->client->resp.context != ent->client->resp.context)
			fail = true;

	i = targ - g_edicts;
	do {
		i+=value;
		if (i > maxclients->value)
			i = 1;
		if (i < 1)
			i = maxclients->value;

		pos = g_edicts + i;
		if (!pos->inuse)
			continue;
		if(pos->client->resp.flag != ARENA_PLAYER ||
            pos->client->resp.context != ent->client->resp.context) continue;
		if (pos->solid != SOLID_NOT)
			break;

	} while (pos != targ);
	if (pos != targ || !fail)
	{
		ent->client->resp.track_targ = pos;
		gi.cprintf(ent, PRINT_HIGH, "Tracking %s\n", pos->client->pers.netname);
	}
/*if(!targ) 
	{
		fail = true;
		targ = g_edicts;
	}
	
	while(targ = G_Find(targ, FOFS(classname), "player")) {
		if(targ->client->resp.flag != ARENA_PLAYER ||
                   targ->client->resp.context != ent->client->resp.context) continue;

		ent->client->resp.track_targ = targ;
		gi.cprintf(ent, PRINT_HIGH, "Tracking %s\n", targ->client->pers.netname);
		return;
	}
	for (i=targ - g_edicts   ; i<maxclients->value ; i++)
	{
		pos = g_edicts + 1 + i;
		if(pos->client->resp.flag != ARENA_PLAYER ||
                   pos->client->resp.context != ent->client->resp.context) continue;

		ent->client->resp.track_targ = pos;
		gi.cprintf(ent, PRINT_HIGH, "Tracking %s\n", pos->client->pers.netname);
		return;
	}
	if(!(fail)) {	//try again (wrap)
		ent->client->resp.track_targ = NULL;
		track_next(ent);
	}*/ else {
		ent->client->resp.omode = ent->client->resp.old_omode;
		move_to_arena(ent,ent->client->resp.context, 1);
		gi.cprintf(ent, PRINT_HIGH, "No one to track\n");
	}
}
void track_next(edict_t *ent)
{
	track_change(ent,1);
}

void track_prev(edict_t *ent)
{
	track_change(ent,-1);
}
void SetObserverMode(edict_t *ent)
{
		int i;

		switch(ent->client->resp.omode) 
		{
			case NORMAL:
				ent->movetype = MOVETYPE_WALK;
				ent->solid = SOLID_BBOX;
				ent->clipmask = MASK_PLAYERSOLID;
				ent->svflags&=~SVF_NOCLIENT;						
				ent->client->resp.track_targ = NULL;
				ent->s.modelindex = 255;
				//ent->s.modelindex2 = 255;
				ShowGun(ent);
				//stuffcmd(ent, "cl_predict 1\n");
				ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			break;
			case FREEFLYING :
				ent->movetype = MOVETYPE_NOCLIP;
				ent->solid = SOLID_NOT;
				ent->clipmask = 0;
				ent->svflags|=SVF_NOCLIENT;				
				ent->client->resp.track_targ = NULL;
				//stuffcmd(ent, "cl_predict 1\n");
				//gi.dprintf("PMF: %d\n", (int) ent->client->ps.pmove.pm_flags);
				ent->client->ps.pmove.pm_time = 0;
				ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				ent->client->ps.pmove.pm_flags &= ~PMF_TIME_TELEPORT;
				//ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			break ;
		    case TRACKCAM :
				ent->movetype = MOVETYPE_NOCLIP;
				ent->solid = SOLID_NOT;
				ent->clipmask = 0;
				ent->svflags|=SVF_NOCLIENT;
				ent->s.modelindex = 0;
				ent->s.modelindex2 = 0;
				//stuffcmd(ent, "cl_predict 0\n");
				ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
				
				// set angles to 0, 0, 0
				for(i=0; i<3; i++) {
					ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(0 - ent->client->resp.cmd_angles[i]);
					ent->s.angles[i] = 0;
				}
				VectorCopy (ent->s.angles, ent->client->ps.viewangles);
				VectorCopy (ent->s.angles, ent->client->v_angle);
				if(! ent->client->resp.track_targ ||  ent->client->resp.track_targ->client->resp.flag != ARENA_PLAYER)
					track_next(ent);
			break;
		    case EYECAM :
				ent->movetype = MOVETYPE_NOCLIP;
				ent->solid = SOLID_NOT;
				ent->clipmask = 0;
				ent->svflags|=SVF_NOCLIENT;
				//stuffcmd(ent, "cl_predict 0\n");
				ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
				// set angles to 0, 0, 0
				for(i=0; i<3; i++) {
					ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(0 - ent->client->resp.cmd_angles[i]);
					ent->s.angles[i] = 0;
				}
				VectorCopy (ent->s.angles, ent->client->ps.viewangles);
				VectorCopy (ent->s.angles, ent->client->v_angle);
				if(! ent->client->resp.track_targ ||  ent->client->resp.track_targ->client->resp.flag != ARENA_PLAYER)
					track_next(ent);
			break;
		}			
}
/*******
move_to_arena - move a single entity to an arena
******/
extern void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void move_to_arena (edict_t *ent, int arena, qboolean observer)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};

	edict_t	*dest;
	int i;

//arena zbot
	if (ent->client->resp.isbot)
	{
		gi.dprintf("\n%s IS A ZBOT %d\n",ent->client->pers.netname, ent->client->resp.isbot);
	//tell them
		gi.centerprintf(ent, "The server seems to think you\nare a bot. If you aren't,\n you may wish to reconnect");
	}
	

	if (observer)
	{
		if (!arenas[arena].idarena)
			dest=SelectFarthestArenaSpawnPoint ("misc_teleporter_dest", arena);		
		else
			dest=SelectFarthestArenaSpawnPoint ("info_player_deathmatch", arena);

		if (arena != 0)
		{
				if (ent->client->resp.context == 0)
				{
					ent->client->resp.context=arena;
					show_observer_menu(ent); //else, they already have the observer menu
				}
		}
		else
		{
			ent->client->resp.track_targ = NULL;
			if (ent->client->resp.team != -1)
			{			
				show_arena_menu(ent); //else it is the first time
			}
		}
		ent->client->resp.context=arena;

	}
	else
	{
		//keep the menu up
/*		ent->client->showmenu = 0;
		DisplayMenu(ent);*/
		ent->client->resp.context=arena;
		ClientUserinfoChanged (ent, ent->client->pers.userinfo); //to set skins
		if (arenas[arena].pickup)
			dest=SelectRandomArenaSpawnPoint ("info_player_deathmatch", arena,
												( ((arena_team_t *)teams[ent->client->resp.team].it)->side == arenas[arena].eventeam) ? 1 : 2);
		else
			dest=SelectFarthestArenaSpawnPoint ("info_player_deathmatch", arena);
	}

	if (!dest)
	{
		gi.bprintf (PRINT_HIGH,"no dest found\n");
		return;
	}
	

gi.unlinkentity (ent);

	VectorCopy (dest->s.origin, ent->s.origin);
	VectorCopy (dest->s.origin, ent->s.old_origin);
	ent->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (ent->velocity);
	ent->client->ps.pmove.pm_time = 160>>3;		// hold time
	ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	if (!observer)
		ent->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - ent->client->resp.cmd_angles[i]);

	VectorClear (ent->s.angles);
	VectorClear (ent->client->ps.viewangles);
	VectorClear (ent->client->v_angle);
	// telefrag avoidance at destination
	if (!KillBox (ent))
	{
	}
	if (observer)
	{
		if (arenas[arena].idarena && ent->client->resp.omode==NORMAL) //so the menu item shows up right
			ent->client->resp.omode=FREEFLYING;

		SetObserverMode(ent);
	} else {
		ent->client->resp.omode = NORMAL;
		SetObserverMode(ent);
	}

	gi.linkentity (ent);//gilink

	if (arenas[arena].voteinfo.endtime > level.time && !ent->client->resp.voted)//inform them of vote in progress
	{
		menu_centerprint(ent, "Settings changes have been proposed!\nGoto the observer menu (TAB) to vote");
//		gi.centerprintf(ent, "Settings changes have been proposed!\nGoto the observer menu (TAB) to vote");
		stuffcmd(ent,"play misc/pc_up.wav\n");
	}
}

char *omode_descriptions[]={"Normal","Free Flying","Trackcam","In Eyes"};

void ChangeOMode(edict_t *ent)
{
	if (ent->client->resp.flag != ARENA_OBSERVER)
		return;
	if(ent->client->resp.omode != TRACKCAM &&
		ent->client->resp.omode != EYECAM) {
		ent->client->resp.old_omode = ent->client->resp.omode;
	}
	ent->client->resp.omode = (ent->client->resp.omode + 1) % NUM_OMODES;
	gi.cprintf(ent, PRINT_HIGH, "Switched Observer Mode to: %s\n", omode_descriptions[ent->client->resp.omode]);
	move_to_arena (ent,ent->client->resp.context, 1);

}


int getfreeskin(int arena)
{
	int usedskins[NUMSKINS];
	int i;

	memset(usedskins,0,sizeof(int)*NUMSKINS);
	for (i=0; i<MAXTEAMS; i++)
		if (teams[i].it && ((arena_team_t *)teams[i].it)->context == arena)
		{
			if (((arena_team_t *)teams[i].it)->skin != -1)
				usedskins[((arena_team_t *)teams[i].it)->skin]=1;
		}
		
	for (i=0; i<NUMSKINS; i++)
		if (!usedskins[i])
			return i;
		//all in use.. pick a random one
	return (rand() % NUMSKINS);

}

char *mylcase(char *s)
{
	char *p;
	int i;
	p=s;
	while (*p)
	{
		i = (int)*p;
		if ((i >= (int)'A' && i<=(int)'Z'))
			*p=(byte)*p+(int)'a'-(int)'A';
		p++;
	}
	return s;

}

int checkvwepmodel(char *s)
{
	int i;

	for (i=0; i < VWEPMODELCT; i++)
		//if (strstr(mylcase(s),vwepmodels[i]))
		if (strstr(s,vwepmodels[i]))
			return 1;
	return 0;

}


void setteamskin(edict_t *ent, char *userinfo, int skinnum)
{
	int playernum;
	char *s;
	

	playernum = ent-g_edicts-1;
	s = Info_ValueForKey (userinfo, "skin");
	if (s[0]=='f') 
	{
		if (strcmp(s,va("female/%s",teamskins[skinnum])))
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\female/%s", ent->client->pers.netname, teamskins[skinnum]) );
	Info_RemoveKey(userinfo,"skin");
	strcat(userinfo, va("\\skin\\female/%s",teamskins[skinnum]));
	stuffcmd(ent,"skin female/nullxxx\n"); //hack so that they can change skins latter
	}
	else if (s[0]=='c' && s[1] == 'r')
	{
		if (strcmp(s,va("crakhor/%s",teamskins[skinnum])))
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\crakhor/%s", ent->client->pers.netname, teamskins[skinnum]) );
	Info_RemoveKey(userinfo,"skin");
	strcat(userinfo, va("\\skin\\crakhor/%s",teamskins[skinnum]));
	stuffcmd(ent,"skin crakhor/nullxxx\n"); //hack so that they can change skins latter
	}
	else if (s[0]=='c' && s[1] == 'y')
	{
		if (strcmp(s,va("cyborg/%s",teamskins[skinnum])))
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\cyborg/%s", ent->client->pers.netname, teamskins[skinnum]) );
	Info_RemoveKey(userinfo,"skin");
	strcat(userinfo, va("\\skin\\cyborg/%s",teamskins[skinnum]));
	stuffcmd(ent,"skin cyborg/nullxxx\n"); //hack so that they can change skins latter
	}
	else
	{
	if (strcmp(s,va("male/%s",teamskins[skinnum]))) //if it changed
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\male/%s", ent->client->pers.netname, teamskins[skinnum]) );
	Info_RemoveKey(userinfo,"skin");
	strcat(userinfo, va("\\skin\\male/%s",teamskins[skinnum]));
	stuffcmd(ent,"skin male/nullxxx\n"); //hack so that they can change skins latter
	}
	
		

}

/*******
SendTeamToArena - send an entire team to an arena, as observers or players
*******/
void SendTeamToArena(arena_link_t *team, int arena, qboolean observe, qboolean addtoline)
{
	arena_link_t	*cur;
	edict_t	*ent;

	cur=team;
	if (!((arena_team_t *)team->it)->observeonly)
	{
		if (arena != 0 && ((arena_team_t *)team->it)->skin == -1 && (arenas[arena].settings.needed_players_team > 1 || arenas[arena].pickup))
			((arena_team_t *)team->it)->skin = getfreeskin(arena);
		else
			if (arena==0 || (arenas[arena].settings.needed_players_team == 1 && !arenas[arena].pickup))
				((arena_team_t *)team->it)->skin = -1;
	}
	while (cur->next)
	{
		cur=cur->next;
	//ent->client->resp.context=arena; move to move_to_arena
		ent=cur->it;
		if (((arena_team_t *)team->it)->skin != -1)
			setteamskin(ent,ent->client->pers.userinfo,((arena_team_t *)team->it)->skin);

		if (observe)
		{
			ent->client->resp.flag = ARENA_OBSERVER;
			ent->takedamage = DAMAGE_NO;
			move_to_arena(ent, arena, true);

		}
		else
		{
			ent->client->resp.flag = ARENA_PLAYER;
			ent->takedamage = DAMAGE_NO; //set damage_aim at start
			move_to_arena(ent, arena, false);
			give_ammo(ent);
		}
	}
	if (addtoline)
	{
		if (observe)
		{
			add_to_queue(&(((arena_team_t *)(team->it))->ar), &arenas[arena].unassigned);
		}
		else
		{
			add_to_queue(&(((arena_team_t *)(team->it))->ar), &arenas[arena].players);
		}
	}
	((arena_team_t *)(team->it))->context = arena;
// clear their total score when they move back to the lobby
	((arena_team_t *)(team->it))->total_wins = 0;
	gi.dprintf("%d: %d %s entered\n",arena, ((arena_team_t *)(team->it))->teamnum, ((arena_team_t *)(team->it))->teamname);			


}

/*******
AddtoArena - called by team leader when he selects an arena, returns 0 if sucessful
*******/
int AddtoArena(edict_t *ent, int arena, qboolean force, qboolean observeonly)
{
	int i=0;
	int tcount;
	
	
	if (!observeonly)
	{
		// check ping
		if(arenas[arena].settings.minping) {
			if(ent->client->ping < arenas[arena].settings.minping) {
				menu_centerprint(ent, va("Your ping is too low\nMinimum ping for this arena: %d", arenas[arena].settings.minping));
				return(1);
			}
		}
		if(arenas[arena].settings.maxping) {
			if(ent->client->ping > arenas[arena].settings.maxping) {
				menu_centerprint(ent, va("Your ping is too high\nMaximum ping for this arena: %d", arenas[arena].settings.maxping));
				return(1);
			}
		}
		//check lock
		if (arenas[arena].settings.lock_arena)
		{
				menu_centerprint(ent, "Sorry, that Arena is locked by an admin\n");
				return(1);
		}
		if (arenas[arena].pickup)
		{
			menu_centerprint(ent, "You must join a pickup team to\n enter that arena");
			return(1);
		}
		
		if (count_queue(&arenas[arena].unassigned) + count_queue(&arenas[arena].players) >= arenas[arena].max_teams) 
		{
			menu_centerprint(ent, "Sorry, that arena is full");
			return(1);
		}
	}
	tcount = count_queue(&teams[ent->client->resp.team]);
	if ((tcount == arenas[arena].settings.needed_players_team) || (tcount <=arenas[arena].settings.needed_players_team && force) )
	{
		((arena_team_t *)(teams[ent->client->resp.team].it))->observeonly = observeonly;
		if (!observeonly)
		{
			remove_from_queue(&(((arena_team_t *)(teams[ent->client->resp.team].it))->ar), NULL); //take them out of arenas[0].unassigned line
			SendTeamToArena(&teams[ent->client->resp.team], arena, true, true); //observe, end of line
		} else
			SendTeamToArena(&teams[ent->client->resp.team], arena, true, false); //observe, dont add to line
		return 0;
	} else if (count_queue(&teams[ent->client->resp.team]) < arenas[arena].settings.needed_players_team)
	{
		show_teamconfirm_menu(ent, arena);
		return 1;
	} else {
		menu_centerprint(ent, va("You have the incorrect number\nof team members, you need %d to play \nin that arena", arenas[arena].settings.needed_players_team));
//		gi.centerprintf(ent, "You have the incorrect number\nof team members, you need %d to play \nin that arena", arenas[arena].settings.needed_players_team);
		return 1;
	}

}

/*******
check_teams
*******/
void check_teams(int context)
{
	int i=0, aping;
	arena_link_t *t, *p, *hold;
	qboolean badping;


//first, check for empty teams
	for(i=0; i<MAXTEAMS; i++) 
	{
		if(teams[i].it) 
		{
			if(!(count_queue(&teams[i])) && !(((arena_team_t *)(teams[i].it))->pickup)) 
			{
				remove_from_queue(&(((arena_team_t *)(teams[i].it))->ar), NULL); //take them out of whatever line their in
				gi.dprintf("Clearing team %d (%s)\n",  ((arena_team_t *)(teams[i].it))->teamnum, ((arena_team_t *)(teams[i].it))->teamname);
				gi.TagFree(teams[i].it);
				teams[i].it=NULL;
			}
		}
	}

	if (context == 0)
		return;
//now, see if all the teams in the arena have the correct # of players
	t = &arenas[context].unassigned;

	while (t->next) //cycle teams
	{
		t=t->next;
		p=t->it;
		badping = false;
		if (!arenas[context].pickup) //dont check pings on pickup
		while (p->next) //cycle players, check pings
		{
			p=p->next;
			aping = ((edict_t *)p->it)->client->ping;
			if ( (aping > arenas[context].settings.maxping && aping < 1000) || //dont kick out because of bad lag
				aping < arenas[context].settings.minping)
			{
				badping = true;
				gi.cprintf(((edict_t *)p->it), PRINT_HIGH, "Sorry, your ping of %d does not work in this arena\n",aping);
			}
		}

		if (/*count_queue(t->it) == 0 || */(count_queue(t->it) > arenas[context].settings.needed_players_team) || badping) //t->it is the actual team
		{
			gi.bprintf(PRINT_MEDIUM, "Removing team %d (%s)\n", ((arena_team_t *)((arena_link_t *) t->it)->it)->teamnum, ((arena_team_t *)((arena_link_t *) t->it)->it)->teamname);
			hold = t->prev;
			remove_from_queue(t, NULL); //take them out of line
			SendTeamToArena(t->it, 0, true, true);//send them back to waiting area
			t = hold; //so that we stay in the correct position
		}
	}
	if (arenas[context].settings.changed)
		if (count_queue(&arenas[context].unassigned)+count_queue(&arenas[context].players)==0) //arena is empty
		{
			set_config(context,context); //reset to defaults for that arena
			gi.dprintf("%d: Reseting to default config\n",context);
		}


}
/*******
init_player
*******/
void init_player(edict_t *e)
{
	

	e->client->resp.ar.it = (void *)e;
	  //don't put player in any queue till they join a team  // ** should add to queue until arena is chosen
	e->client->resp.flag = ARENA_OBSERVER;
	e->client->resp.context = 0;    // the meeting area is 0
	e->client->resp.team = -1;
	if (e->client->pers.showmotd)
		motd_menu(e);
	else
		menuRefreshTeamList(e,NULL, NULL, 0);
	e->client->resp.track_targ = NULL;
	e->client->resp.old_omode = FREEFLYING;
	e->takedamage = DAMAGE_NO;

	send_configstring(e, CS_ARENASTRINGS+2, " Red");
	send_configstring(e, CS_ARENASTRINGS+3, "Blue");
}

/*******
reinit_player on respawn
*******/
void reinit_player(edict_t *e) 
{
	e->client->resp.flag = ARENA_OBSERVER;
	e->client->resp.track_targ = NULL;
	e->client->resp.old_omode = FREEFLYING;
}
/*******
show_stringc
*******/
void show_stringc(char *s, int context)
{
	int i;
	edict_t *e;

	for (i=0 ; i<maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if(e->inuse && e->client && e->client->resp.context == context) {
			gi.centerprintf(e, s);
		}
	}
}
/*******
show_string
*******/
void show_string(int priority, char *s, int context)
{
	int i;
	edict_t *e;

	for (i=0 ; i<maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if(e->inuse && e->client && e->client->resp.context == context) {
			gi.cprintf(e,priority, s);
		}
	}
}
/*******
stuffcmd
*******/
void stuffcmd(edict_t *e, char *s) {
        gi.WriteByte (11);
        gi.WriteString (s);
        gi.unicast (e, true);
}


/*******
send_sound_to_arena
*******/
void send_sound_to_arena(char *soundname, int context)
{
	int i;
	edict_t *e;

	for (i=0 ; i<maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if(e->inuse && e->client && e->client->resp.context == context) {
			stuffcmd(e, va("play %s\n",soundname));

		}
	}
}

void send_configstring(edict_t *ent, int index, char *str)
{
	gi.WriteByte (0x0D); //configstring
	gi.WriteShort(index); //status bar index
	gi.WriteString (str);
	gi.unicast (ent, true);
}

extern char *dm_statusbar;
/*******
show_countdown
*******/
void show_countdown(int count, int context)
{
	int i;
	edict_t *e;

	for (i=0 ; i<maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if(e->inuse && e->client && e->client->resp.context == context) 
		{
			if (arenas[context].game_state == GAME_PRECOUNT)
				send_configstring(e, CS_ARENASTRINGS, "Waiting for match to start");
			else
				send_configstring(e, CS_ARENASTRINGS, arenas[context].teamstr);
			if (arenas[context].settings.rounds > 1)
				send_configstring(e, CS_ARENASTRINGS+1, va("Round %d of %d",arenas[context].round, arenas[context].settings.rounds));
			else
				send_configstring(e, CS_ARENASTRINGS+1, "");
			e->client->ps.stats[STAT_ARENA_TEXT] = CS_ARENASTRINGS;
			e->client->ps.stats[STAT_ARENA_ROUNDT] = CS_ARENASTRINGS + 1;
			e->client->ps.stats[STAT_ARENA_COUNT] = count;
			if(count == 15 || count==10 || count == 5)  
			{
				if (!e->client->showmenu) //if menu isn't up
					SendStatusBar(e, dm_statusbar,true); //since they may have observer one on
			} 

			if (count > 0 && count < 4 && arenas[context].game_state != GAME_PRECOUNT)
			{
				stuffcmd(e, va("play ra/%d.wav\n",count));
			}
			else if (count == 0) 
			{
				if (arenas[context].game_state != GAME_PRECOUNT)
				{
					stuffcmd(e, "play ra/fight.wav\n");
					gi.centerprintf(e, "FIGHT!");
				}
			}
		}
	}
}

/*******
show_rank
*******/
int show_rank(arena_link_t *que)
{
	int i = 0;

	while((que = que->prev) != NULL) 
		i++;
	return i;
}

/*******
check_for_teams - checks to see if we have enough teams to begin
*******/
qboolean check_for_teams(int context)
{
	int i;
	arena_link_t *pos;

	if(count_queue(&arenas[context].unassigned) >= arenas[context].needed_teams)
	{
		pos = &arenas[context].unassigned;
		i=0;
		while (pos->next && i < arenas[context].needed_teams)
		{
			pos=pos->next;
			i++;
			if (!count_queue(pos->it)) //if there is an empty pickup team, dont start
				return false;
		}
		return (true);
	}
	else
		return (false);


}

/*******
fill_arena - sends teams from the waiting line to the arena
*******/
qboolean fill_arena(int context)
{
	arena_link_t *t;
	int i;
	char longstr[256];
	int lastskin=-1;

	longstr[0]='\0';
	arenas[context].eventeam = rand() % 2; //0 or 1 !! do this before the loop!!
	for (i=0; i<arenas[context].needed_teams; i++)
	{
		t = remove_from_queue(NULL, &arenas[context].unassigned); //t is team[i].it->ar, ar->it is team
		if (t == NULL)
		{//team left during a multi round match
			gi.dprintf("Team left during multi-round match\n");
			return true;
		}
		if (lastskin==-1)
			lastskin = ((arena_team_t *)(((arena_link_t *)(t->it))->it))->skin;
		else
			if (lastskin == ((arena_team_t *)(((arena_link_t *)(t->it))->it))->skin)
			{
				gi.dprintf("Skin conflict in arena %d\n",context);
				((arena_team_t *)(((arena_link_t *)(t->it))->it))->skin= (lastskin + 1) % NUMSKINS;
			}
		SendTeamToArena(t->it, context, false, true);//dont observe
		if (i!=0)
			strcat(longstr," vs ");
		strcat(longstr, ((arena_team_t *)(((arena_link_t *)(t->it))->it))->teamname);
		if (arenas[context].round == 1)
			((arena_team_t *)(((arena_link_t *)(t->it))->it))->wins = 0;
		((arena_team_t *)(((arena_link_t *)(t->it))->it))->in_arena = true;

	}
	strncpy(arenas[context].teamstr, longstr, MAX_QPATH - 1); //copy max of 63 chars
	arenas[context].teamstr[MAX_QPATH - 1] = '\0'; //make sure it ends in null
gi.dprintf("%d: %s\n",context,arenas[context].teamstr);
	return true;
	
}

/*******
fight_done - determines when the match is over by check if more than 1 team is left,
           returns -2 if still going, -1 if everyone is dead
*******/
int fight_done(int context)
{
	arena_link_t *t, *p;
	edict_t *ent;
	int aliveteam=-1;

	t = &arenas[context].players;

	while (t->next) //cycle teams
	{
		t=t->next;
		p=t->it;
		while (p->next) //cycle players
		{
			p=p->next;
			ent=((edict_t *)p->it);
			if (ent->takedamage == DAMAGE_AIM && ent->deadflag==DEAD_NO)
			{
				if (aliveteam==-1)
					aliveteam=ent->client->resp.team;
				else
					if (aliveteam != ent->client->resp.team)
						return -2; //more than 1 team left
			}
		}
	}

	return aliveteam; //-1 if everyone is dead, otherwise winning team number


}
/*******
CTFSetIDView
*******/
static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}
static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}

void CTFSetIDView(edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
/*	edict_t	*who, *best;
	float	bd = 0, d;
	int i;*/

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;

	if (ent->client->resp.flag != ARENA_OBSERVER)
		return;
	if (ent->client->resp.track_targ)
	{
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_PLAYERSKINS + (ent->client->resp.track_targ - g_edicts - 1);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, CONTENTS_MONSTER|CONTENTS_SOLID); //<-- people can't see through walls (including observer walls)
	if (tr.fraction < 1 && tr.ent && tr.ent->client && tr.ent->solid != SOLID_NOT) {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_PLAYERSKINS + (tr.ent - g_edicts - 1);
		return;
	}
/* -- too laggy.. only direct viewing
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	if (bd > 0.90)
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_PLAYERSKINS + (best - g_edicts - 1);
			--*/
}

/*******
UpdateStatusBars
*******/
void UpdateStatusBars(int context)
{
	arena_link_t *t, *p;
	edict_t *e;
	char *playernames[2][4], *teamnames[2];
	int playerhealth[2][4];
	int tcount,pcount[2],tc,pc,i,xp,yp;
	char my_statusbar[1400], *pos;

//get the info
	t = &arenas[context].players;
	tcount = -1;
	while (t->next && tcount <2) //cycle teams
	{
		tcount++;
		t=t->next;
		p=t->it;
		teamnames[tcount]=((arena_team_t *)(p->it))->teamname;
		pcount[tcount] = -1;
		while (p->next && pcount[tcount]< MAX_PLAYERS_TODISP - 1) //cycle players (max is 3.. since we go from 0..3)
		{
			p=p->next;
			e=((edict_t *)p->it);
			if (e->takedamage == DAMAGE_AIM && e->deadflag==DEAD_NO)
			{
				pcount[tcount]++;
				playernames[tcount][pcount[tcount]]=e->client->pers.netname;
				playerhealth[tcount][pcount[tcount]]=e->health;

			}
		}
	}
//build the statusbar
	xp = 8;
	yp = 40;
//  frags
	strcpy(my_statusbar, "xl 8 yb -10 string2 \"Line Position:\" xl 100 yb -24 num 2 19 ");

	pos = my_statusbar+strlen(my_statusbar);
	for (tc=0; tc <= tcount; tc++)
	{
		sprintf(pos,"xl %d yt %d string2 \"%s\" ",		
			xp, yp, teamnames[tc]);
		pos = my_statusbar+strlen(my_statusbar);
		yp+=8;

		for (pc=0; pc <= pcount[tc]; pc++)
		{
			sprintf(pos,"xl %d yt %d string2 \"%s: %d\" ",	
				xp, yp, playernames[tc][pc], playerhealth[tc][pc]);
			pos = my_statusbar+strlen(my_statusbar);
			yp+=8;
		}
		yp+=8;
	}
	strcpy(pos,"if 20 " "xv 0 "  "yb -58 " "stat_string 20 " "endif ");


//send to clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if(e->inuse && e->client && e->client->resp.context == context && e->client->resp.flag == ARENA_OBSERVER && e->client->showmenu==0)
		{
			if (e->client->resp.track_targ || e->client->showscores) //if tracking someone, or scoreboard or menu is up
				SendStatusBar(e, dm_statusbar,true); //don't display stats if the scoreboard or menu is up
			else
			{
				e->client->ps.stats[STAT_ARENA_POS] = show_rank(&((arena_team_t *)(teams[e->client->resp.team].it))->ar);
				SendStatusBar(e, my_statusbar,true);
			}
		}
	}

}

/*******
check_telefrag
*******/
void check_telefrag(int context)
{
	int i;
	edict_t *e;
	trace_t		tr;
	vec3_t	tv1, tv2;

	for (i=0 ; i<maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if(e->inuse && e->client && e->client->resp.context == context && e->client->resp.flag != ARENA_OBSERVER)
		{
			if(e->client->resp.telefrag_time &&
				e->client->resp.telefrag_time <= level.framenum) 
			{
				tr = gi.trace (e->s.origin, e->mins, e->maxs, e->s.origin, NULL, MASK_PLAYERSOLID);
				if(tr.contents == CONTENTS_SOLID) 
				{	// rematerialized in a platform?
					e->solid = SOLID_NOT;
					tv1[YAW] = rand() % 360;
					tv1[PITCH] = 0;
					tv1[ROLL] = 0;
					AngleVectors(tv1, tv2, NULL, NULL);
					VectorScale(tv2, 600, tv2);
					VectorAdd(e->velocity, tv2, e->velocity);
					e->client->resp.telefrag_time = level.framenum + 0.5 / FRAMETIME;
				} else 
				{
					e->solid = SOLID_BBOX;
					gi.unlinkentity(e);
					KillBox(e);
					gi.linkentity(e);
				}
			}
		}
	}
}


/*******
start_voting
*******/
void start_voting(edict_t *e, int arena)
{
	int i;
	edict_t *ent;

	if (arenas[arena].game_state == GAME_ON || arenas[arena].game_state == GAME_COUNT)
		arenas[arena].voteinfo.endtime = level.time + 30000; //will be set to 30 after the match is done
	else
		arenas[arena].voteinfo.endtime = level.time + 30;
	arenas[arena].voteinfo.numvoters = arenas[arena].voteinfo.ncount = arenas[arena].voteinfo.ycount = 0;

	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if(ent->inuse && ent->client && ent->client->resp.context == arena)
		{
			ent->client->resp.voted = false;
			arenas[arena].voteinfo.numvoters++;
			if (ent->client->resp.flag == ARENA_OBSERVER)	//only send message to observers
			{
				if(ent != e) {
					menu_centerprint(ent, "Settings changes have been proposed!\nGoto the observer menu (TAB) to vote");
//					gi.centerprintf(ent, "Settings changes have been proposed!\nGoto the observer menu (TAB) to vote");
					stuffcmd(ent,"play misc/pc_up.wav\n");
				} else {
//					menu_centerprint(ent, "Settings changes have been proposed!\n");
//					gi.centerprintf(ent, "Settings changes have been proposed!\n");
					stuffcmd(ent,"play misc/pc_up.wav\n");
				}
			}
		}
	}
	gi.dprintf("Starting Voting in Arena %d with %d voters\n",arena, arenas[arena].voteinfo.numvoters);

}


/*******
check_voting
*******/
void check_voting(int arena)
{
	char votestr[80];
	int i;
	edict_t *ent;
	if (arenas[arena].voteinfo.endtime == 0)
		return; //no voting
	if (arenas[arena].voteinfo.endtime > level.time)
		return; //time left
	
	arenas[arena].voteinfo.endtime = 0;
	if ( (arenas[arena].voteinfo.ycount - arenas[arena].voteinfo.ncount) >= (float)arenas[arena].voteinfo.numvoters / 3.0)
	{ //yes votes - no votes > 1/3 total
		memcpy(&arenas[arena].settings, &arenas[arena].voteinfo.propsettings, sizeof(arena_settings_t));
		arenas[arena].settings.changed = 1;
		sprintf(votestr,"Changes Passed! Yes votes: %d No votes: %d\n",arenas[arena].voteinfo.ycount, arenas[arena].voteinfo.ncount);
	}
	else { //failed
		sprintf(votestr,"Changes Failed! Yes votes: %d No votes: %d\n",arenas[arena].voteinfo.ycount, arenas[arena].voteinfo.ncount);
	}
	
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if(ent->inuse && ent->client && ent->client->resp.context == arena)
		{
			gi.cprintf(ent, PRINT_CHAT, votestr);
			//reset their votetries now that a vote passed
				ent->client->resp.votetries = votetries_setting;
		}
	}
gi.dprintf(votestr);
	check_teams(arena);
}


/*******
arena_think
*******/
void arena_think(int context)
{
	
	arena_obj_t *a = &arenas[context];
	arena_link_t *t,*pos;
	int winner;
	
	check_teams(context);	//catch teams where players disconnected during match
	check_voting(context);
	check_telefrag(context);
	
	if (a->game_state == GAME_COUNT || a->game_state == GAME_PRECOUNT) 
	{  // print countdown to everyone
		if(!a->game_count_frame) 
		{ // need to assign the countdown
			a->game_count_frame = level.framenum + 1 / FRAMETIME;    //count in 1 sec
			if (a->game_state == GAME_PRECOUNT)
				a->game_count = 15;
			else
				if (a->voteinfo.endtime > level.time)
					a->game_count = 10; //countdown for 10 sec if vote in progress
				else
					a->game_count = 5;    //countdown ends in 5 seconds
			show_countdown(a->game_count, context);
		} else if (a->game_count_frame < level.framenum)
		{
			show_countdown(--a->game_count, context);
			if(!a->game_count) 
			{
				a->game_count_frame = 0;
				if (a->game_state == GAME_PRECOUNT)
				{
					if(!check_for_teams(context)) //ppl left!
					{
						a->game_state = COMP_WAIT;
						show_stringc("Not enough teams to start", context);
					}
					else
					{
						a->game_state = GAME_COUNT;
						fill_arena(context);
					}

				}
				else					
				{
					a->game_state = GAME_ON;
					set_damage(context, DAMAGE_AIM);   // set everyone's takedamage flag
				}
            } else
				a->game_count_frame = level.framenum + 1 / FRAMETIME;
		}
	} else if (a->game_state == GAME_ON && !broken) 
	{
		UpdateStatusBars(context);
		if(fight_done(context) > -2 ) 
		{
			a->game_state = COMP_OFF;
		}
	} else if (a->game_state == COMP_WAIT) 
	{
		if(check_for_teams(context)) { //people are ready
			a->round = 1;
			if (arenas[context].pickup)
				a->game_state = GAME_PRECOUNT;
			else
			{
				a->game_state = GAME_COUNT;     // ** should go to comp_count
				fill_arena(context);

			}
		}
	} else if (a->game_state == COMP_OFF) 
	{
		UpdateStatusBars(context);
		if(!a->game_count_frame) 
		{ //wait 3 sec for stuff to blow up
			a->game_count_frame = level.framenum + 3 / FRAMETIME; //3 second wait
			
		} else if (a->game_count_frame <  level.framenum) 
		{
			//stuff is done exploding
			a->game_state = COMP_END;
			a->game_count_frame = 0;
			if (a->voteinfo.endtime - level.time > 30) //vote was started during the match, ends 30 sec from now
				a->voteinfo.endtime = level.time + 30;
			
		}
	} else if (a->game_state == COMP_END) { 
		winner=fight_done(context);
		if (winner==-1)
		{
			sprintf(a->winstring, "It was a tie!");
		}
		else
		{
			if (++((arena_team_t *)(teams[winner].it))->wins > arenas[context].settings.rounds / 2)
			{
				Com_sprintf(a->winstring,sizeof(a->winstring), "%s has won the match!!", ((arena_team_t *)(teams[winner].it))->teamname);
				arenas[context].round = arenas[context].settings.rounds;
			}
			else
				Com_sprintf(a->winstring,sizeof(a->winstring), "%s has won the round!", ((arena_team_t *)(teams[winner].it))->teamname);
			((arena_team_t *)teams[winner].it)->total_wins++;
		//bug.. was outside.. teams [-1] was getting set!!!!
		}
		if (winner == -1 && count_queue(&arenas[context].players)!=0 && count_queue( ((arena_link_t *)(arenas[context].players.next))->it)) //don't increment round number on tie, unless arena is empty, or first team is empty (pickup)
			a->round--;
		gi.dprintf("%d: %d %s\n",context,winner, a->winstring);			

		set_damage(context, DAMAGE_NO);
		show_stringc(a->winstring, context);
		
		pos = &arenas[context].players;
		arenas[context].eventeam = rand() % 2; //0 or 1 // do before loop!
		while (pos->next) //cycle teams..
		{
			pos=pos->next;				
			if (a->round < arenas[context].settings.rounds) //multiple rounds
			{
				Com_sprintf(a->winstring,sizeof(a->winstring), "%s has %d wins and needs %d more to take the match\n", ((arena_team_t *)((arena_link_t *) pos->it)->it)->teamname, ((arena_team_t *)((arena_link_t *) pos->it)->it)->wins, arenas[context].settings.rounds / 2 + 1 - ((arena_team_t *)((arena_link_t *) pos->it)->it)->wins);
				show_string(PRINT_HIGH,a->winstring,context);
				SendTeamToArena(pos->it, context, false, false);//dont observe
			}else
			{
				
				t=remove_from_queue(NULL, &arenas[context].players);
				// clear the in arena flag
				((arena_team_t *)(((arena_link_t *)(t->it))->it))->in_arena = false;
				pos = &arenas[context].players; //goback to start
				if ( ((arena_team_t *)((arena_link_t *) t->it)->it)->teamnum==winner || winner==-1)
				{
					add_to_front_queue(t, &arenas[context].unassigned);
				//	((arena_team_t *)(((arena_link_t *)(t->it))->it))->wins = -1;
				}
				else
				{
					add_to_queue(t, &arenas[context].unassigned);
				//	((arena_team_t *)(((arena_link_t *)(t->it))->it))->wins = -1;
				}
			}
		}
		if (a->round < arenas[context].settings.rounds)
		{
			a->round++;
			a->game_state = GAME_COUNT;     // ** should go to comp_count
		} else
			a->game_state = COMP_WAIT;
	}
}


/*******
multi_arena_think

  Iterates over each arena object and calls its think function
  Different thinks run on different frames, ie, arena0 runs frame 0
  arena1 runs frame 1, etc.
  *******/
  void multi_arena_think(void)
  {
	int i;
	  //static int flag = 0;
	  
	if (level.intermissiontime)
		return;
	  
	i = level.framenum % (num_arenas * 2); //run arenas on even frames only
	if (!(i % 2))
		arena_think(i / 2 + 1);

}

/*******
arena_init

initializes the link structures
loads map loop
*******/
void arena_init(edict_t *wsent)
{
	int i;
	char *tempname;
	arena_team_t *ateam;
	
	if(!wsent) return;

	teams = gi.TagMalloc(MAXTEAMS * sizeof(teams[0]), TAG_LEVEL);
	memset(teams, 0, MAXTEAMS * sizeof(teams[0]));
	memset(arenas, 0, MAXARENAS * sizeof(arenas[0]));

	admincode = gi.cvar ("admincode", "0",0);

	num_arenas = wsent->arena; //worldspawn arena flag is # of arenas
	if (!num_arenas)
	{
		num_arenas=1;
		idmap = true;
	}
	else idmap = false;
// config file --
	load_config(num_arenas + 1);

	set_config(1,num_arenas);

	for(i=0; i<=num_arenas; i++) 
	{
		arenas[i].game_state =COMP_WAIT;

		arenas[i].idarena = idmap; //check for idarena or not
		arenas[i].needed_teams= 2;
		arenas[i].num_teams = 0;

	// init head link
		arenas[i].unassigned.prev = NULL;
		arenas[i].players.prev = NULL;
		arenas[i].unassigned.next = NULL;
		arenas[i].players.next = NULL;
		arenas[i].game_count_frame = 0;
        arenas[i].game_count = 0;

		arenas[i].voteinfo.endtime = 0;
		arenas[i].round = 0;

		if (!SelectFarthestArenaSpawnPoint ("misc_teleporter_dest", i))
		{
			gi.dprintf("Setting arena %d to idarena mode\n",i);
			arenas[i].idarena = true;
		}

		if (i && arenas[i].pickup) //dont do to arena 0
		{
			tempname=gi.TagMalloc(MAXITEMSIZE, TAG_LEVEL);
			sprintf(tempname, "#%d Pickup Red",i);
			ateam=add_to_team(NULL, tempname);
			ateam->side = 0;	// red
			SendTeamToArena(ateam->ar.it, i, true, true); //observe, end of line
			arenas[i].red_team = ateam;

			tempname=gi.TagMalloc(MAXITEMSIZE, TAG_LEVEL);
			sprintf(tempname, "#%d Pickup Blue",i);
			ateam=add_to_team(NULL, tempname);
			ateam->side = 1;	// blue
			SendTeamToArena(ateam->ar.it, i, true, true); //observe, end of line
			arenas[i].blue_team = ateam;
			arenas[i].max_teams = 2;
			arenas[i].settings.needed_players_team = 128;
		}
	}

	load_motd();
}

/*QUAKED trigger_teleport (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED
*/
// g_misc.c
void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

void SP_trigger_teleport (edict_t *self)
{
	self->touch = teleporter_touch;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_TRIGGER;
	self->use = NULL;
	gi.setmodel (self, self->model);
	gi.linkentity (self);
}
/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
void SP_func_illusionary (edict_t *self)

{
	//self.angles = '0 0 0';
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	gi.setmodel (self, self->model);
	gi.linkentity(self);
};

/*QUAKED info_teleport_destination (1 0 0) (-16 -16 -24) (16 16 32)
*/
void SP_info_teleport_destination(edict_t *self)
{
}
/*			if(a->game_mode == GAME_NORMAL)
				a->game_state = GAME_OFF;
			else if (a->game_mode == GAME_COMP)
*/

/*------------------------------------------------------------------------*/
/* GRAPPLE																  */
/*------------------------------------------------------------------------*/

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
	if (self->owner->client->ctf_grapple) {
		float volume = 1.0;
		gclient_t *cl;

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->grapple_on = false;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		G_FreeEdict(self);
	}
}

void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float volume = 1.0;

	if (other == self->owner)
		return;

	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		CTFResetGrapple(self);
		return;
	}

	VectorCopy(vec3_origin, self->velocity);

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage) {
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_UNKNOWN);
		CTFResetGrapple(self);
		return;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;

	gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	float	distance;

	AngleVectors (self->owner->client->v_angle, f, r, NULL);
	VectorSet(offset, 16, 16, self->owner->viewheight-8);
	P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);

	VectorSubtract(start, self->owner->s.origin, offset);

	VectorSubtract (start, self->s.origin, dir);
	distance = VectorLength(dir);
	// don't draw cable if close
	if (distance < 64)
		return;

#if 0
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	trace_t	tr; //!!

	tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
	if (tr.ent != self) {
		CTFResetGrapple(self);
		return;
	}
#endif

	// adjust start for beam origin being in middle of a segment
//	VectorMA (start, 8, f, start);

	VectorCopy (self->s.origin, end);
	// adjust end z for end spot since the monster is currently dead
//	end[2] = self->absmin[2] + self->size[2] / 2;

	gi.WriteByte (svc_temp_entity);
//#if 1 //def USE_GRAPPLE_CABLE
	gi.WriteByte (TE_GRAPPLE_CABLE);
	gi.WriteShort (self->owner - g_edicts);
	gi.WritePosition (self->owner->s.origin);
	gi.WritePosition (end);
	gi.WritePosition (offset);
/*#else
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (end);
	gi.WritePosition (start);
#endif
*/	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void SV_AddGravity (edict_t *ent);

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
	vec3_t hookdir, v;
	float vlen;

/* //arena -- not needed?
	if (strcmp(self->owner->client->pers.weapon->classname, "weapon_grapple") == 0 &&
		!self->owner->client->newweapon &&
		self->owner->client->weaponstate != WEAPON_FIRING &&
		self->owner->client->weaponstate != WEAPON_ACTIVATING) {
		CTFResetGrapple(self);
		return;
	}
*/
	if (self->enemy) {
		if (self->enemy->solid == SOLID_NOT) {
			CTFResetGrapple(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX) {
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity (self);
		} else
			VectorCopy(self->enemy->velocity, self->velocity);
		if (self->enemy->takedamage &&
			!CheckTeamDamage (self->enemy, self->owner)) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_UNKNOWN);
			gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}
		if (self->enemy->deadflag) { // he died
			CTFResetGrapple(self);
			return;
		}
	}

	CTFGrappleDrawCable(self);

	if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		// pull player toward grapple
		// this causes icky stuff with prediction, we need to extend
		// the prediction layer to include two new fields in the player
		// move stuff: a point and a velocity.  The client should add
		// that velociy in the direction of the point
		vec3_t forward, up;

		AngleVectors (self->owner->client->v_angle, forward, NULL, up);
		VectorCopy(self->owner->s.origin, v);
		v[2] += self->owner->viewheight;
		VectorSubtract (self->s.origin, v, hookdir);

		vlen = VectorLength(hookdir);

		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize (hookdir);
		VectorScale(hookdir, CTF_GRAPPLE_PULL_SPEED, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}

void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize (dir);

	grapple = G_Spawn();
	VectorCopy (start, grapple->s.origin);
	VectorCopy (start, grapple->s.old_origin);
	vectoangles (dir, grapple->s.angles);
	VectorScale (dir, speed, grapple->velocity);
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
	VectorClear (grapple->mins);
	VectorClear (grapple->maxs);
	grapple->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
//	grapple->nextthink = level.time + FRAMETIME;
//	grapple->think = CTFGrappleThink;
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	gi.linkentity (grapple);

	tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch (grapple, tr.ent, NULL, NULL);
	}
}	

void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume = 1.0;

	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors (ent->client->v_angle, forward, right, NULL);
//	VectorSet(offset, 24, 16, ent->viewheight-8+2);
	VectorSet(offset, 24, 8, ent->viewheight-8+2);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->silencer_shots)
		volume = 0.2;

	gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
	CTFFireGrapple (ent, start, forward, damage, CTF_GRAPPLE_SPEED, effect);

#if 0
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void CTFWeapon_Grapple_Fire (edict_t *ent)
{
	int		damage;

	damage = 10;
	CTFGrappleFire (ent, vec3_origin, damage, 0);
	ent->client->ps.gunframe++;
}

void CTFWeapon_Grapple (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int prevstate;

	// if the the attack button is still down, stay in the firing frame
	if ((ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->weaponstate == WEAPON_FIRING &&
		ent->client->ctf_grapple)
		ent->client->ps.gunframe = 9;

	if (!(ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->ctf_grapple) {
		CTFResetGrapple(ent->client->ctf_grapple);
		if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;
	}


	if (ent->client->newweapon && 
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
		ent->client->weaponstate == WEAPON_FIRING) {
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 32;
	}

	prevstate = ent->client->weaponstate;
	Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames, 
		CTFWeapon_Grapple_Fire);

	// if we just switched back to grapple, immediately go to fire frame
	if (prevstate == WEAPON_ACTIVATING &&
		ent->client->weaponstate == WEAPON_READY &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 9;
		else
			ent->client->ps.gunframe = 5;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}
