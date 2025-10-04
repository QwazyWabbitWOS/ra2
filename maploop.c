
#include "g_local.h"
#include <sys/stat.h>
#include <sys/types.h>

static cvar_t *gamedir;

// config file --

// map loop
definition_item_t *map_loop;
// map block:
definition_item_t *map_block;
// per arena blocks:
definition_item_t **arena_blocks;

int votetries_setting = 3;

int num_definition_blocks = 0;
definition_item_t *definition_blocks;
static char *line;

//                    2  3  4  5   6   7   8    9    0
int weapon_vals[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256 };

#define S_GET_VAL(tval, name, defs, num)			\
	if((it = find_key(name, DEF_VAL, defs, num))) {	\
		(tval) = atoi(get_val(it->val.vals, 0));	\
	}
/*
cvar_t *weapons;
cvar_t *armor;
cvar_t *health;
cvar_t *minping;
cvar_t *maxping;
cvar_t *playersperteam;
cvar_t *rounds;
cvar_t *max_teams;
cvar_t *pickup;
cvar_t *rocket_speed;
cvar_t *shells;
cvar_t *bullets;
cvar_t *slugs;
cvar_t *grenades;
cvar_t *rockets;
cvar_t *cells;
*/
int weapons;
int armor;
int health;
int minping;
int maxping;
int playersperteam;
int rounds;
int max_teams;
int pickup;
int rocket_speed;
int shells;
int bullets;
int slugs;
int grenades;
int rockets;
int cells;
int fastswitch;
int armorprotect;
int healthprotect;
int fallingdamage;
int allow_voting_armor;
int allow_voting_health;
int allow_voting_minping;
int allow_voting_maxping;
int allow_voting_playersperteam;
int allow_voting_rounds;
int allow_voting_maxteams;
int allow_voting_armorprotect;
int allow_voting_healthprotect;
int allow_voting_shotgun;
int allow_voting_supershotgun;
int allow_voting_machinegun;
int allow_voting_chaingun;
int allow_voting_grenadelauncher;
int allow_voting_rocketlauncher;
int allow_voting_hyperblaster;
int allow_voting_railgun;
int allow_voting_bfg;
int allow_grapple;
int allow_voting_fallingdamage;
int lock_arena;

qboolean has_val(char *vals, char *str)
{
	char srch[1024];
	char *tok;

	strcpy(srch, vals);
	tok = strtok(srch, " ");
	while(tok) {
		if(!(strcmp(tok, str))) return(true);
		tok = strtok(0, " ");
	}
	return(false);
}

char *get_val(char *vals, int num)
{
	char srch[1024];
	static char fnd[1024];
	char *tok;

	strcpy(srch, vals);
	tok = strtok(srch, " ");
	while(tok && num) {
		num--;
		tok = strtok(0, " ");
	}
	if (!tok)
		fnd[0]='\0';
	else
		strcpy(fnd, tok);
	return(fnd);
}

void get_settings(definition_item_t *defs, int num_defs)
{
	definition_item_t *it;
	int v;
	int w, w2;

	if((it = find_key("weapons", DEF_VAL, defs, num_defs))) {
		v = 0;
		for(w=0; w<= 8; w++) {
			if(w == 8)
			{w2 = 0;}
			else
			{w2 = w + 2;}
			if(has_val(it->val.vals, va("%d", w2))) {
				v |= weapon_vals[w];
			}
		}
		weapons = v;
	}
	S_GET_VAL(armor, "armor", defs, num_defs)
	S_GET_VAL(health, "health", defs, num_defs)
	S_GET_VAL(minping, "minping", defs, num_defs)
	S_GET_VAL(maxping, "maxping", defs, num_defs)
	S_GET_VAL(playersperteam, "playersperteam", defs, num_defs)
	S_GET_VAL(rounds, "rounds", defs, num_defs)
	S_GET_VAL(max_teams, "maxteams", defs, num_defs)
	S_GET_VAL(pickup, "pickup", defs, num_defs)
	S_GET_VAL(rocket_speed, "rocketspeed", defs, num_defs)
	S_GET_VAL(shells, "shells", defs, num_defs)
	S_GET_VAL(bullets, "bullets", defs, num_defs)
	S_GET_VAL(slugs, "slugs", defs, num_defs)
	S_GET_VAL(grenades, "grenades", defs, num_defs)
	S_GET_VAL(rockets, "rockets", defs, num_defs)
	S_GET_VAL(cells, "cells", defs, num_defs)
	S_GET_VAL(fastswitch, "fastswitch", defs, num_defs)
	S_GET_VAL(armorprotect, "armorprotect", defs, num_defs)
	S_GET_VAL(healthprotect, "healthprotect", defs, num_defs)
	S_GET_VAL(fallingdamage, "fallingdamage", defs, num_defs)
	S_GET_VAL(allow_voting_armor, "allowvotingarmor", defs, num_defs)
	S_GET_VAL(allow_voting_health, "allowvotinghealth", defs, num_defs)
	S_GET_VAL(allow_voting_minping, "allowvotingminping", defs, num_defs)
	S_GET_VAL(allow_voting_maxping, "allowvotingmaxping", defs, num_defs)
	S_GET_VAL(allow_voting_playersperteam, "allowvotingplayersperteam", defs, num_defs)
	S_GET_VAL(allow_voting_rounds, "allowvotingrounds", defs, num_defs)
	S_GET_VAL(allow_voting_maxteams, "allowvotingmaxteams", defs, num_defs)
	S_GET_VAL(allow_voting_armorprotect, "allowvotingarmorprotect", defs, num_defs)
	S_GET_VAL(allow_voting_healthprotect, "allowvotinghealthprotect", defs, num_defs)
	S_GET_VAL(allow_voting_shotgun, "allowvotingshotgun", defs, num_defs)
	S_GET_VAL(allow_voting_supershotgun, "allowvotingsupershotgun", defs, num_defs)
	S_GET_VAL(allow_voting_machinegun, "allowvotingmachinegun", defs, num_defs)
	S_GET_VAL(allow_voting_chaingun, "allowvotingchaingun", defs, num_defs)
	S_GET_VAL(allow_voting_grenadelauncher, "allowvotinggrenadelauncher", defs, num_defs)
	S_GET_VAL(allow_voting_rocketlauncher, "allowvotingrocketlauncher", defs, num_defs)
	S_GET_VAL(allow_voting_hyperblaster, "allowvotinghyperblaster", defs, num_defs)
	S_GET_VAL(allow_voting_railgun, "allowvotingrailgun", defs, num_defs)
	S_GET_VAL(allow_voting_bfg, "allowvotingbfg", defs, num_defs)
	S_GET_VAL(allow_voting_fallingdamage, "allowvotingfallingdamage", defs, num_defs)
	S_GET_VAL(lock_arena,"lockarena", defs, num_defs);
}

extern qboolean idmap; //in arena.c

void set_config(int stnum, int endnum)
{
	int i;
	definition_item_t *it;

	for(i=stnum; i<=endnum; i++) {
		// load default values from serverinfo
		weapons = 255;//gi.cvar ("weapons", "255", CVAR_SERVERINFO);	// default no BFG
		armor = 200; //gi.cvar ("armor", "200", CVAR_SERVERINFO);
		health = 100;//gi.cvar ("health", "100", CVAR_SERVERINFO);
		minping = 0;//gi.cvar ("minping", "0", CVAR_SERVERINFO);
		maxping = 1000;//gi.cvar ("maxping", "1000", CVAR_SERVERINFO); // 1000 max to prevent temp lag/connect hack
		playersperteam = 1;//gi.cvar ("playersperteam", "1", CVAR_SERVERINFO );
		if (!idmap)
			rounds = 1;//gi.cvar("rounds","1",CVAR_SERVERINFO);
		else
			rounds = 9; //default to 9 rounds, pickup on id maps
		max_teams = 128;//gi.cvar("maxteams","128",CVAR_SERVERINFO);
		if (!idmap)
			pickup = 0;//gi.cvar("pickup","0",CVAR_SERVERINFO);
		else
			pickup = 1;//default to 9 rounds, pickup on id maps
		rocket_speed = 650;//gi.cvar("rocketspeed","650",CVAR_SERVERINFO);
		shells = 100;//gi.cvar("shells","100",CVAR_SERVERINFO);
		bullets = 200;//gi.cvar("bullets","200",CVAR_SERVERINFO);
		slugs = 50;//gi.cvar("slugs","50",CVAR_SERVERINFO);
		grenades = 50;//gi.cvar("grenades","50",CVAR_SERVERINFO);
		rockets = 50;//gi.cvar("rockets","50",CVAR_SERVERINFO);
		cells = 150;//gi.cvar("cells","200",CVAR_SERVERINFO);
		fastswitch = 1;
		armorprotect = 2;
		healthprotect = 1;
		fallingdamage = 1;
		allow_voting_armor = 1;
		allow_voting_health = 1;
		allow_voting_minping = 1;
		allow_voting_maxping = 1;
		allow_voting_playersperteam = 1;
		allow_voting_rounds = 1;
		allow_voting_maxteams = 1;
		allow_voting_armorprotect = 1;
		allow_voting_healthprotect = 1;
		allow_voting_shotgun = 1;
		allow_voting_supershotgun = 1;
		allow_voting_machinegun = 1;
		allow_voting_chaingun = 1;
		allow_voting_grenadelauncher = 1;
		allow_voting_rocketlauncher = 1;
		allow_voting_hyperblaster = 1;
		allow_voting_railgun = 1;
		allow_voting_bfg = 1;
		allow_voting_fallingdamage = 1;
		lock_arena = 0;
		
		// load global settings
		get_settings(definition_blocks, num_definition_blocks);

		// load per-map settings
		if (map_block)
		{
			get_settings(map_block->val.defs, map_block->numvals);
		}
		// load per-arena settings
		if(map_block) {
			it = arena_blocks[i];
			if(it) {
				get_settings(it->val.defs, it->numvals);
			}
		}

		arenas[i].settings.weapons = weapons;
		arenas[i].settings.armor = armor;
		arenas[i].settings.health = health;
		arenas[i].settings.minping = minping;
		arenas[i].settings.maxping = maxping;
		arenas[i].settings.needed_players_team = playersperteam;
		arenas[i].settings.rounds = rounds;
		arenas[i].max_teams = max_teams;
		arenas[i].pickup = pickup;
		arenas[i].settings.rocket_speed = rocket_speed;
		arenas[i].settings.shells = shells;
		arenas[i].settings.bullets = bullets;
		arenas[i].settings.slugs = slugs;
		arenas[i].settings.grenades = grenades;
		arenas[i].settings.rockets = rockets;
		arenas[i].settings.cells = cells;
		arenas[i].settings.fastswitch = fastswitch;
		arenas[i].settings.armorprotect = armorprotect;
		arenas[i].settings.healthprotect = healthprotect;
		arenas[i].settings.fallingdamage = fallingdamage;
		arenas[i].settings.allow_voting_armor = allow_voting_armor;
		arenas[i].settings.allow_voting_health = allow_voting_health;
		arenas[i].settings.allow_voting_minping = allow_voting_minping;
		arenas[i].settings.allow_voting_maxping = allow_voting_maxping;
		arenas[i].settings.allow_voting_playersperteam = allow_voting_playersperteam;
		arenas[i].settings.allow_voting_rounds = allow_voting_rounds;
		arenas[i].settings.allow_voting_maxteams = allow_voting_maxteams;
		arenas[i].settings.allow_voting_armorprotect = allow_voting_armorprotect;
		arenas[i].settings.allow_voting_healthprotect = allow_voting_healthprotect;
		arenas[i].settings.allow_voting_shotgun = allow_voting_shotgun;
		arenas[i].settings.allow_voting_supershotgun = allow_voting_supershotgun;
		arenas[i].settings.allow_voting_machinegun = allow_voting_machinegun;
		arenas[i].settings.allow_voting_chaingun = allow_voting_chaingun;
		arenas[i].settings.allow_voting_grenadelauncher = allow_voting_grenadelauncher;
		arenas[i].settings.allow_voting_rocketlauncher= allow_voting_rocketlauncher;
		arenas[i].settings.allow_voting_hyperblaster = allow_voting_hyperblaster;
		arenas[i].settings.allow_voting_railgun = allow_voting_railgun;
		arenas[i].settings.allow_voting_bfg = allow_voting_bfg;
		arenas[i].settings.allow_voting_fallingdamage = allow_voting_fallingdamage;
		arenas[i].settings.lock_arena = lock_arena;//we can lock arenas to start!
		arenas[i].settings.changed = 0; //defaults have been loaded
	}
}

int ra_isalnum(char token)
{
	int i;
	
	i = (int)token;
	if ((i >= (int)'0' && i <= (int)'9') || (i >= (int)'A' && i<=(int)'Z') || (i >= (int)'a' && i <= (int)'z'))
		return 1;
	else
		return 0;
	
}

char *next_token(char *arg)
{
	static char *token = NULL;
	static char foo[1024];
	char *ptr;

	if(arg) token=arg;
	else if (!arg && !token) return(NULL);
//	while(*token != '\0' && *token != '\n' && (isspace(*token))) token++;
	if(*token == '\0' || *token == '\n') {
//			gi.dprintf("%s\n", "<null>");
			return(NULL);
	}
	ptr = foo;
	if(!(ra_isalnum(*token))) {
//		gi.dprintf("notisalnum: %s, %c\n", foo, *token);
		*ptr++ = *token++;
		if(*token == '/' && ptr[-1] == '/')
			*ptr++ = *token++;
		*ptr++ = '\0';

		return(foo);
	}
	while(ra_isalnum(*token)) *ptr++ = *token++;
	*ptr++ = '\0';
//	gi.dprintf("%s\n", foo);
	return(foo);
}

definition_item_t *new_def_block(void)
{
	return( (definition_item_t *)gi.TagMalloc(sizeof(definition_item_t) * MAX_DEF_ITEMS, TAG_LEVEL) );
}

char *new_val_block (void)
{
	char *val = (char *)gi.TagMalloc(1024, TAG_LEVEL);

	sprintf(val, " ");
	return(val);
}

void add_val(char *val_str, char *val)
{
	strcat(val_str, " ");
	strcat(val_str, val);
}

definition_item_t *new_def_item(definition_item_t *items)
{
	definition_item_t *item = items;
	item->numkeys = 0;
	item->numvals = 0;
	item->keys = new_val_block();
	item->type = DEF_UNDEFINED;
	return(item);
}

typedef struct def_stack {
	int numitems;
	definition_item_t *items;
	definition_item_t *curdef;
} def_stack_t;

def_stack_t stack[32];

int read_block(FILE *cfgfile, definition_item_t *items)
{
	int lev = 0;
	definition_item_t *curdef;
	char *tok;
	enum {
		MODE_KEY,
		MODE_VAL
	} mode;
	int numitems = 0;
	int r;

	curdef = new_def_item(items++);
	mode = MODE_KEY;
	while(1) {
		r = fscanf(cfgfile, "%s", line);
		if(r < 1) {
			if(lev) {
				gi.dprintf("Error reading config file: unbalanced {}\n");
				return(0);
			}
			return(numitems);
		}
//		gi.dprintf("result: %d line: %s\n", r, line);
		tok = next_token(line);

		while(tok) {
//			gi.dprintf("%s\n", tok);
			if(tok[0] == '/' && tok[1] == '/') {	// skip rest of line
				while(1) {
					r = fgetc(cfgfile);
					if(r < 1) {
						return(numitems);
					}
					if(r == '\n') {
						break;
					}
				}
				break;
			}
			if(mode == MODE_KEY) {
				if(*tok == '{') {	//beginning of block identifier
//					curdef->keys = (char *)realloc(curdef->keys, strlen(curdef->keys) + 1);
					curdef->type = DEF_BLOCK;
					curdef->val.defs = new_def_block();

					// stack
					stack[lev].numitems = numitems;
					stack[lev].items = items;
					stack[lev].curdef = curdef;

					numitems = 0;
					items = curdef->val.defs;
					curdef = new_def_item(items++);
					lev++;
				} else
				if(*tok == ':') {	// value identifier
//					curdef->keys = (char *)realloc(curdef->keys, strlen(curdef->keys) + 1);
					curdef->type = DEF_VAL;
					curdef->val.vals = new_val_block();
					mode = MODE_VAL;
				} else
				if(*tok == '}') {	// end of block identifier
//					gi.dprintf("%d\n", level);

					if(!lev) {
						gi.dprintf("Error reading config file: unbalanced {}\n");
						return(0);
					}

					// stack
					lev--;
					curdef = stack[lev].curdef;
					curdef->numvals = numitems;
//					curdef->val.defs = (definition_item_t *)realloc(curdef->val.defs, sizeof(*curdef->val.defs) * numitems);
					items = stack[lev].items;
					numitems = stack[lev].numitems;
					curdef = new_def_item(items++);
					mode = MODE_KEY;
					numitems++;
				}
				else
				{
					// add new key
					add_val(curdef->keys, tok);
					curdef->numkeys++;
				}
			}
			else if(mode == MODE_VAL) {
				if(*tok == ';') {	// end of value
//					curdef->val.vals = (char *)realloc(curdef->val.vals, strlen(curdef->val.vals) + 1);
					curdef = new_def_item(items++);
					mode = MODE_KEY;
					numitems++;
				}
				else
				{
					// new value
					add_val(curdef->val.vals, tok);
					curdef->numvals++;
				}
			}
			tok = next_token(NULL);
		}
	}
}

void read_config(FILE *cfgfile)
{
	int r;

	definition_blocks = new_def_block();
	line = (char *)gi.TagMalloc(4096,TAG_LEVEL);
	num_definition_blocks = 0;
	r = read_block(cfgfile, definition_blocks);
	num_definition_blocks = r;
}

definition_item_t *find_key(char *key, definition_type_t type, definition_item_t *blocks, int num)
{
	char keyfind[1024];
	int i;
	char *val = NULL;

//gi.dprintf("%d %s\n",num,key); 
	for(i=0; i<num; i++) {
		if(blocks[i].type != type) continue;
		strcpy(keyfind, blocks[i].keys);
//gi.dprintf("%s\n",keyfind);
		val = strtok(keyfind, " ");
		while(val) {
//gi.dprintf("%s\n",val);
			if(!strcmp(val, key))
				return(&blocks[i]);
			val = strtok(0, " ");
		}
	}
	return(NULL);
}

void list_keys(edict_t *ent)
{
	definition_item_t *block;
	char result[1024] = { 0 };
	int i;
	int num = num_definition_blocks;
	definition_item_t *blocks = definition_blocks;
	int argc = gi.argc();

	for(i=1; i < argc; i++) {
		if(!(block = find_key(gi.argv(i), DEF_BLOCK, blocks, num))) {
			gi.cprintf(ent, PRINT_HIGH, "Block not found: %s\n", gi.argv(i));
			return;
		}
		blocks = block->val.defs;
		num = block->numvals;
	}
	for(i=0; i<num; i++) {
		strcat(result, blocks[i].keys);
		strcat(result, "  ");
		if(blocks[i].type == DEF_VAL)
			strcat(result, va("V  %s\n", blocks[i].val.vals));
		else if (blocks[i].type == DEF_BLOCK)
			strcat(result, "B\n");
		else
			strcat(result, "U\n");
	}

	gi.cprintf(ent, PRINT_HIGH, "%s", result);
}

void load_config(int num_arenas)
{
        FILE *fp;
        char mpath[80];
	definition_item_t *map, *arena, *it;
	int i;

        gamedir = gi.cvar("game", ".", CVAR_LATCH);
        strcpy(mpath, gamedir->string);

		strcat(mpath, "/arena.cfg");

        if(!(fp = fopen(mpath, "r"))) {
                gi.dprintf("Error: Couldn't read %s\n", mpath);
                return;
        }

	read_config(fp);

	fclose(fp);
	
	// set per-map and arenas blocks
	arena_blocks = (definition_item_t **) gi.TagMalloc(sizeof(definition_item_t *) * num_arenas, TAG_LEVEL);
	if((map = find_key(level.mapname, DEF_BLOCK, definition_blocks, num_definition_blocks))) {
		gi.dprintf("arena.cfg info for map found: %s\n",level.mapname);
		map_block = map;
		for(i=0; i<num_arenas; i++) {
			arena = find_key(va("%d", i), DEF_BLOCK, map->val.defs, map->numvals);
			arena_blocks[i] = arena;
		}
	} else
	{
		gi.dprintf("arena.cfg info for map not found: %s\n",level.mapname);
		map_block = NULL;
	}

	if((it = find_key("votetries", DEF_VAL, definition_blocks, num_definition_blocks))) {
		votetries_setting = atoi(get_val(it->val.vals, 0));
	}

	if((it = find_key("grapple", DEF_VAL, definition_blocks, num_definition_blocks))) {
		allow_grapple = atoi(get_val(it->val.vals, 0));
	}

// get map loop
	if((map_loop = find_key("maploop", DEF_VAL, definition_blocks, num_definition_blocks))) {
		gi.dprintf("Map loop read\n");
	}
}
// -- config file

/*******
get_next_map
*******/
char *get_next_map(char *curmap)
{
	int i;
	char *newmap;

	if(!map_loop) return(NULL);
	if(!has_val(map_loop->val.vals, curmap)) {
		return(get_val(map_loop->val.vals, 0));
	}

	i=0;
	while (i < map_loop->numvals) 
	{
		if(!strcmp(curmap, get_val(map_loop->val.vals, i))) {
			newmap = get_val(map_loop->val.vals, ++i);
			if(!strlen(newmap)) {
				return(get_val(map_loop->val.vals, 0));
			} else {
				return(newmap);
			}
		}
		i++;
	}
	return curmap;
}

/*******
print_map_loop
*******/
void print_map_loop(edict_t *ent)
{
	if(map_loop) {
		gi.cprintf(ent, PRINT_MEDIUM, "%s\n", map_loop->val.vals);
	} else {
		gi.cprintf(ent, PRINT_MEDIUM, "No map loop set\n");
	}
}

extern arena_link_t motd;

/*******
load_motd - Loads the Message of the Day from motd.txt file
*******/
void load_motd(void)
{
	FILE* fp;
	char* mappt;
	char* maps;
	char mpath[MAX_QPATH];
	arena_link_t* alink;

	motd.next = motd.prev = NULL;

	// Get game directory path
	gamedir = gi.cvar("game", ".", 0);

	// Construct path to motd.txt
	Com_sprintf(mpath, sizeof(mpath), "%s/motd.txt", gamedir->string);

	if (!(fp = fopen(mpath, "r"))) {
		gi.dprintf("Error: Couldn't read %s\n", mpath);
		return;
	}
	else
		gi.dprintf("Successfully read %s\n", mpath);

	if (!(maps = gi.TagMalloc(2048, TAG_LEVEL))) {
		gi.dprintf("Error: Couldn't malloc for MOTD\n");
		return;
	}

	mappt = maps;
	while ((mappt = fgets(mappt, 99999, fp)) > (char*)0)
	{
		if (mappt[strlen(mappt) - 1] == '\n')
			mappt[strlen(mappt) - 1] = '\0'; //get rid of newline
		alink = gi.TagMalloc(sizeof(arena_link_t), TAG_LEVEL);
		alink->it = mappt;
		add_to_queue(alink, &motd);
		mappt += strlen(mappt) + 1;
	}

	fclose(fp);
}
