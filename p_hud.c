#include "g_local.h"



/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	//arena
	clear_menus(ent);

	if (deathmatch->value || coop->value)
// arena
		ent->client->showscores = 2;	// 2 is the server wide scoreboard
//		ent->client->showscores = true;	
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	//arena
	n=0;
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		n++;
		MoveClientToIntermission (client);
		gi.dprintf("%s\n",client->client->pers.netname);
	}
	//arena
	if (!n) 
		level.exitintermission = 1; //go straight to next level
	else
		gi.dprintf("%d Clients on level change\n",n);
}

void Serverwide_ScoreboardMessage(edict_t *ent)
{
char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
	
	char p_name[100];
	char tempentry[1024];	// sort the clients by score

	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	string[0] = 0;
	stringlength = strlen(string);

	
	// make a header for the data 
	Com_sprintf(entry, sizeof(entry), 
		"xv 0 yv 32 string2 \"Frags Ping   Name        Team       A\" "
		"xv 0 yv 40 string2 \"›››››››››››››››››››››››››››››››››››››\" "); 
	j = strlen(entry); 
	if (stringlength + j < 1024) 
	{ 
		strcpy (string + stringlength, entry); 
		stringlength += j; 
	} 
	
	if (total > 23) 
		total = 23; 
	
	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];
		
		x = 32; 
		y = 48 + 8 * i; 
		
		if (cl->resp.team > -1)
		{
			strncpy(p_name,((arena_team_t *)teams[cl->resp.team].it)->teamname,100);
		} else
			sprintf(p_name,"None");
		p_name[99]=0;
		Com_sprintf(tempentry,sizeof(tempentry),"%3i %4i %12.12s %12.12s %1i", cl->resp.score, cl->ping,cl->pers.netname, p_name,cl->resp.context);
		if (cl_ent == ent)
			HiPrint(tempentry);	
		Com_sprintf(entry, sizeof(entry), 
			"xv 8 yv %i string2 \"%s\"", y,tempentry); 
		
		j = strlen(entry); 
		if (stringlength + j > 1024) 
			break; 
		strcpy (string + stringlength, entry); 
		stringlength += j; 
		
		
	}	
	gi.WriteByte (svc_layout);
	gi.WriteString (string);

}

void Arena_ScoreboardMessage(edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAXTEAMS];
	int		sortedscores[MAXTEAMS];
	int		score, total;
//	int		picnum;
//	int		x,y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	int sorted2[MAX_CLIENTS];
	int sortedscores2[MAX_CLIENTS];
	int pings[MAXTEAMS];
	int total2;
	char p_name[1024];
	int context, avgping, nplayers, linenum, l;
	arena_link_t *cur;
	arena_team_t *curteam;
	extern arena_link_t *teams;

	context = ent->client->resp.context;
	
	// pickup the teams ...
	total = 0;
	for(i=0; i<MAXTEAMS; i++) {
		if(!teams[i].it) continue;
		if(((arena_team_t *)teams[i].it)->context != context || ((arena_team_t *)teams[i].it)->observeonly) continue;
		// accumulate players scores and ping times
		cur=&teams[i];
		score = 0, nplayers = 0, avgping = 0;
		while(cur->next) {
			cur = cur->next;
			cl_ent = cur->it;
			cl = cl_ent->client;
			score += cl->resp.score;
			avgping += cl->ping;
			nplayers++;
		}
		if(!nplayers) continue;	// no players
		avgping = (int)(avgping / nplayers);
		for(j=0; j < total; j++) {
			if(score > sortedscores[j]) break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
			pings[k] = pings[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		pings[j] = avgping;
		total++;
	}

	// now teams are sorted
	// build the scoreboard
	string[0] = 0;
	stringlength = strlen(string);

	Com_sprintf(entry, sizeof(entry),
		"xv 0 yv 40 string2 \"Teams\" xv 160 string2 \"Players\" ");
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	linenum = 1;
	total = (total > 20) ? 20 : total;
	for(l=0; l<total; l++) {
		curteam = (arena_team_t *)teams[sorted[l]].it;
		Com_sprintf(p_name, sizeof(p_name),
			"%-2d %-3d %.11s",
			sortedscores[l], pings[l], curteam->teamname);
		if(curteam->in_arena) {	// put them in white text, if in arena
			HiPrint(p_name);
		}
		Com_sprintf(entry, sizeof(entry),
			"xv 0 yv %d string2 \"%s\" ",
			40 + linenum * 8, p_name);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;

		// now do the players...
		total2 = 0;
		cur = (arena_link_t *)curteam->ar.it;
		while(cur->next) {
			cur = cur->next;

			cl_ent = cur->it;
			score = cl_ent->client->resp.score;
			for (j=0 ; j<total2 ; j++)
			{
				if (score > sortedscores2[j])
					break;
			}
			for (k=total2 ; k>j ; k--)
			{
				sorted2[k] = sorted2[k-1];
				sortedscores2[k] = sortedscores2[k-1];
			}
			sorted2[j] = cl_ent - g_edicts - 1;
			sortedscores2[j] = score;
			total2++;
		}
		// display them
		total2 = (total2 > 20) ? 20 : total2;
		for(i=0; i<total2; i++) {
			cl_ent = g_edicts + 1 + sorted2[i];
			cl = &game.clients[sorted2[i]];
			Com_sprintf(p_name, sizeof(p_name),
				"%-2d %-3d %.11s",
				cl->resp.score, cl->ping, cl->pers.netname);
			if(cl_ent->takedamage != DAMAGE_NO) {
				HiPrint(p_name);
			}
			Com_sprintf(entry, sizeof(entry),
				"xv 160 yv %d string2 \"%s\" ",
				40 + linenum * 8, p_name);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;

			linenum++;
		}
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void Pickup_ScoreboardMessage(edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
//	int		picnum;
//	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	int sorted2[MAX_CLIENTS];
	int sortedscores2[MAX_CLIENTS];
	int bluescore = 0, redscore = 0;
	int total2;
	char p_name[1024];

	extern arena_link_t *teams;

	total = 0;
	for(i=0; i<game.maxclients; i++) {
		cl_ent = g_edicts + 1 + i;
		if(!cl_ent->inuse ||
			cl_ent->client->resp.context != ent->client->resp.context)
				continue;
		if(((arena_team_t *)teams[cl_ent->client->resp.team].it)->side != 0)	// not on red team
				continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		redscore = ((arena_team_t *)teams[cl_ent->client->resp.team].it)->wins;
		total++;
	}
	total2 = 0;
	for(i=0; i<game.maxclients; i++) {
		cl_ent = g_edicts + 1 + i;
		if(!cl_ent->inuse ||
			cl_ent->client->resp.context != ent->client->resp.context)
				continue;
		if(((arena_team_t *)teams[cl_ent->client->resp.team].it)->side != 1)	// not on blue team
				continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total2 ; j++)
		{
			if (score > sortedscores2[j])
				break;
		}
		for (k=total2 ; k>j ; k--)
		{
			sorted2[k] = sorted2[k-1];
			sortedscores2[k] = sortedscores2[k-1];
		}
		sorted2[j] = i;
		sortedscores2[j] = score;
		bluescore = ((arena_team_t *)teams[cl_ent->client->resp.team].it)->wins;
		total2++;
	}

	string[0] = 0;
	stringlength = strlen(string);

	// put the two teams at the top
	if(redscore < 0) redscore = 0;
	if(bluescore < 0) bluescore = 0;
	Com_sprintf(entry, sizeof(entry),
		"xv 0 yv 40 string2 \"Team Red  : %d\" xv 160 yv 40 string2 \"Team Blue : %d\" ",
		redscore, bluescore);
	j = strlen(entry);
//		if (stringlength + j > 1024)
//			break;
	strcpy (string + stringlength, entry);
	stringlength += j;

	total = (total > 20) ? 20 : total; //cap at 20
	total2 = (total2 > 20) ? 20 : total2; //cap at 20

	// add players to the list
	for(i=0; i<total || i<total2; i++) {
		if(i < total) {	// show person from red
			cl_ent = g_edicts + 1 + sorted[i];
			cl = &game.clients[sorted[i]];
			strcpy(p_name, cl->pers.netname);
			if(cl_ent->takedamage == DAMAGE_NO) {
				LoPrint(p_name);
			} else {
				HiPrint(p_name);
			}
				Com_sprintf(entry, sizeof(entry),
				"xv 0 yv %d string2 \"%2d %3d %.12s\" ",
					i * 8 + 48, cl->resp.score, cl->ping, p_name);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}
		if(i < total2) {	// show person from blue
			cl_ent = g_edicts + 1 + sorted2[i];
			cl = &game.clients[sorted2[i]];
			strcpy(p_name, cl->pers.netname);
		if(cl_ent->takedamage == DAMAGE_NO) {
				LoPrint(p_name);
			} else {
				HiPrint(p_name);
			}
			Com_sprintf(entry, sizeof(entry),
				"xv 160 yv %d string2 \"%2d %3d %.12s\" ",
				i * 8 + 48, cl->resp.score, cl->ping, p_name);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
/*
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
*/
	if(!ent->client->resp.context && ent->client->showscores == 1)
		ent->client->showscores = 2;

	if(ent->client->showscores == 2) {
		Serverwide_ScoreboardMessage(ent);
	} else {
		if(arenas[ent->client->resp.context].pickup) {
			// do the pickup game scoreboard
			Pickup_ScoreboardMessage(ent);
		} else {
			// do the new scoreboard
			Arena_ScoreboardMessage(ent);
		}
	}}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	//arena
	DeathmatchScoreboardMessage (ent, ent->enemy);
/*	if (level.framenum + 1 == ent->client->menuframe)
		return; //don't want to send on the same frame as a menu
	ent->client->menuframe = level.framenum+1;
	//arena -- dont send reliable if on a menu frame
*/
	if (ent->client->showscores == 2)
		gi.unicast (ent, true); //always send main scoreboard reliable
	else
		gi.unicast (ent, false);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (!deathmatch->value && !coop->value)
		return;

// arena
	if(ent->client->showscores == 2) {
		ent->client->showscores = 0;
	} else {
		if(ent->client->resp.context == 0)
			ent->client->showscores = 2;	// if in lobby, only gets server wide scoreboard
		else
			ent->client->showscores++;
	}
/*
	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}
	ent->client->showscores = true;
*/
	DeathmatchScoreboard (ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
// arena
	ent->client->showscores = 0;
//	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;
// arena
	char	s[256];
	int i,j,k;

	//
	// health
	//
// arena
//	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
// set health icon to skin icon
	sprintf (s, "%s_i", Info_ValueForKey(ent->client->pers.userinfo, "skin"));
	j = gi.imageindex(s);
	k = level.pic_health;
	for(i=0; i<NUMSKINS; i++) {
		if(teamskins_precachem[i] == j || teamskins_precachef[i]==j ||
			teamskins_precachecw[i] == j || teamskins_precachecb[i] == j) {
			k = j;
			break;
		}
	}

	ent->client->ps.stats[STAT_HEALTH_ICON] = k;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
	
	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ArmorIndex (ent);
	if (power_armor_type && (!index || (level.framenum & 8) ) )
	{	// flash between power armor and other armor icon
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	//
	// pickup message
	//
	
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;
	// arena
	if (ent->client->resp.context == 0)
	{
		ent->client->ps.stats[STAT_ARENA_COUNT] = 0;
		ent->client->ps.stats[STAT_ARENA_TEXT] = 0;
		ent->client->ps.stats[STAT_ARENA_PICKUP] = 0;
	} else if(arenas[ent->client->resp.context].pickup) {
		if(arenas[ent->client->resp.context].game_state == GAME_ON || arenas[ent->client->resp.context].game_state == COMP_OFF || arenas[ent->client->resp.context].game_state == COMP_END) {
			ent->client->ps.stats[STAT_ARENA_RED_LEFT] = count_players_queue( (arena_link_t *)((arenas[ent->client->resp.context].red_team)->ar.it) );
			ent->client->ps.stats[STAT_ARENA_BLUE_LEFT] = count_players_queue( (arena_link_t *)((arenas[ent->client->resp.context].blue_team)->ar.it) );
		} else {
			ent->client->ps.stats[STAT_ARENA_RED_LEFT] = count_queue( (arena_link_t *)((arenas[ent->client->resp.context].red_team)->ar.it) );
			ent->client->ps.stats[STAT_ARENA_BLUE_LEFT] = count_queue( (arena_link_t *)((arenas[ent->client->resp.context].blue_team)->ar.it) );
		}
		ent->client->ps.stats[STAT_ARENA_RED_LEFT_STR] = CS_ARENASTRINGS+2;
		ent->client->ps.stats[STAT_ARENA_BLUE_LEFT_STR] = CS_ARENASTRINGS+3;
		ent->client->ps.stats[STAT_ARENA_PICKUP] = 1;
	} else {
		ent->client->ps.stats[STAT_ARENA_PICKUP] = 0;
	}
	CTFSetIDView(ent);
}

