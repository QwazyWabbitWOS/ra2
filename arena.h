//define MEMDEBUG

#ifdef MEMDEBUG
	void *myTagMalloc(int size, int tag);
	void myTagFree(void *p);
#endif

#define BOT_BYPORT 1
#define BOT_BYIMP 2
#define BOT_BYANGLES 3
#define MAXTEAMS 256
#define MAXARENAS 32
#define MAX_PLAYERS_TODISP 4
#define FINALRA2
#define NUMSKINS 7
#define VWEPMODELCT 4
// config file --
#define MAX_DEF_ITEMS 256

typedef enum {
		DEF_UNDEFINED,
		DEF_VAL,
		DEF_BLOCK
} definition_type_t;

typedef struct definition_struct {
	int numkeys, numvals;
	char *keys;
	definition_type_t type;
	union {
		struct definition_struct *defs;
		char *vals;
	} val;
} definition_item_t;
// -- config file

typedef enum {
//GAME_WAIT,      //normal mode, waiting for players to enter arena
GAME_PRECOUNT, //delay for pickup arenas
GAME_COUNT,     //players in arena, countdown before fight
GAME_ON,        //players in arena, shooting
//GAME_OFF,       //grace period after fight waiting for projectiles to stop
COMP_WAIT,      //competition mode, waiting for people to choose teams
COMP_COUNT,     //show names of competing teams/players
//COMP_ON,        //same as GAME_ON
COMP_OFF,       //grace period after fight, advances round #, reforms teams, continues
COMP_END        //reset teams, go back to COMP_WAIT;
} arena_state_t;

typedef enum {
GAME_NORMAL,
GAME_COMP
} arena_mode_t;

typedef enum {
ARENA_OBSERVER,
ARENA_PLAYER,
} arena_player_t;

#define NUM_OMODES 4

typedef enum {
	NORMAL,
	FREEFLYING,
	TRACKCAM,
	EYECAM
} observer_mode_t;

typedef enum {
	FORCE,
	PROPOSE,
	VOTE,	
} adminmenu_mode_t;

typedef enum {
	CTF_GRAPPLE_STATE_FLY,
	CTF_GRAPPLE_STATE_PULL,
	CTF_GRAPPLE_STATE_HANG
} ctfgrapplestate_t;

#define CTF_GRAPPLE_SPEED					650 // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED				650	// speed player is pulled at

typedef struct arena_link_s {
        void *it;
        struct arena_link_s *next;
        struct arena_link_s *prev;
} arena_link_t;

typedef struct arena_team_s {
	char *teamname;
	int teamnum;
	int context;
	int wins;
	arena_link_t ar;
	int pickup;
	int side;		// blue or red
	int total_wins;
	int skin;
	qboolean in_arena;	//used for the scoreboard
	qboolean observeonly;
} arena_team_t;

/*
typedef struct arena_msg_s {
        char msg[80];
        int  expire_frame;
} arena_msg_t;
*/
typedef struct arena_settings_s {
    int needed_players_team;
	int rounds;
	int weapons;
	int armor;
	int health;
	int minping;
	int maxping;
	int rocket_speed;
	int shells, bullets, slugs, grenades, rockets, cells;
	int startdelay;
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
	int allow_voting_fallingdamage;
	int lock_arena;
	int changed;
} arena_settings_t;

typedef struct arena_voteinfo_s {
	float endtime;
	arena_settings_t propsettings;
	int numvoters;
	int ycount;
	int ncount;
} arena_voteinfo_t;


typedef struct arena_obj_s {
//        int needed_players;             // number of players needed for match to play

        int needed_teams;               // number of teams needed to start match

        int num_teams;
        arena_link_t unassigned;        // observers
        arena_link_t players;          // players
        arena_state_t game_state;       // what state the game in this arena is at
        arena_mode_t game_mode;         // normal or comp.
        int game_count_frame;           // used for countdown, etc.
        int game_count;                 // used for countdown, etc.
        qboolean idarena;
        char winstring[160];
		char teamstr[64]; //xyz vs. whowhat
		arena_settings_t settings;
		arena_voteinfo_t voteinfo;
		int pickup;
		int eventeam;
		int max_teams;                  // max number of teams allowed
        int round;
		arena_team_t *red_team;
		arena_team_t *blue_team;
} arena_obj_t;

//
// arena.c
//
extern arena_obj_t arenas[];
extern char *teamskins[];
extern int teamskins_precachem[];
extern int teamskins_precachef[];
extern int teamskins_precachecw[];
extern int teamskins_precachecb[];
extern arena_link_t *teams;            // buffer of teams to [max_teams]

int count_players_queue(arena_link_t *que);
void send_configstring(edict_t *ent, int index, char *str);
void CTFSetIDView(edict_t *ent);
void track_SetStats(edict_t *ent);
void eyecam_think(edict_t *ent, usercmd_t *ucmd);
void SetObserverMode(edict_t *ent);
void add_to_queue(arena_link_t *t, arena_link_t *que);
arena_link_t *remove_from_queue(arena_link_t *t, arena_link_t *que);
int count_queue(arena_link_t *que);
arena_team_t *add_to_team(edict_t *e, char *teamname);
int AddtoArena(edict_t *ent, int arena, qboolean force, qboolean observeonly);
void move_to_arena (edict_t *ent, int arena, qboolean observer);
void SendTeamToArena(arena_link_t *team, int arena, qboolean observe, qboolean addtoline);
void ChangeOMode(edict_t *ent);
void arena_init(edict_t *wsent);
void multi_arena_think(void);
void reinit_player(edict_t *e);
void init_player(edict_t *e);
void check_teams(int context);
void remove_from_team(edict_t *e);
void send_sound_to_arena(char *soundname, int context);
qboolean check_menu(edict_t *e, int index);
void track_next(edict_t *ent);
void track_prev(edict_t *ent);
void track_think(edict_t *ent, usercmd_t *ucmd);
void stuffcmd(edict_t *e, char *s);
void start_voting(edict_t *e, int arena);
void show_string(int priority, char *s, int context);
void setteamskin(edict_t *ent, char *userinfo, int skinnum);
int checkvwepmodel(char *s);
void CTFWeapon_Grapple (edict_t *ent);
void CTFPlayerResetGrapple(edict_t *ent);
void CTFGrapplePull(edict_t *self);
void CTFResetGrapple(edict_t *self);
void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect);
// ra2menus.c
char *getarenaname(int arena);
void cvar_setvalue(char *cvar, int value);
void show_observer_menu(edict_t *ent);
void show_arena_menu(edict_t *ent);
void motd_menu(edict_t *ent);
void show_teamconfirm_menu(edict_t *ent, int arena);
void Cmd_admin_f (edict_t *ent);
void Cmd_menuhelp_f(edict_t *ent);
void Cmd_arenaadmin_f (edict_t *ent, adminmenu_mode_t mode);
void menu_centerprint (edict_t *ent, char *msg);
int menuDoNothing(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuLeaveArena(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuAddtoArena(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuLeaveTeam(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuAddtoTeam(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuNewTeam(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuRefreshTeamList(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuChangeValue(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuChangeValue10(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuChangeYesNo(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuChangeMap(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuApplyAdmin(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuCancel(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuApplyArenaAdmin(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuChangeOMode(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
int menuTeamConfirm(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key);
//
// maploop.c
//
// config file --
extern definition_item_t *map_block;
extern definition_item_t **arena_blocks;
extern int votetries_setting;
extern int allow_grapple;

qboolean has_val(char *vals, char *str);
char *get_val(char *vals, int num);
void load_config(int num_arenas);
void set_config(int stnum, int endnum);
definition_item_t *find_key(char *key, definition_type_t type, definition_item_t *blocks, int num);
void list_keys(edict_t *ent);
// -- config file
char *get_next_map(char *curmap);
void print_map_loop(edict_t *e);
void load_motd(void);

//
// p_weapon.c
//
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));

//
// g_combat.c
//
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
