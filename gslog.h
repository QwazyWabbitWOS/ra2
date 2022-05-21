void GSLogShutdown(void);
void GSLogStartup(void);
void GSLogNewmap(void);
void GSLogDeath(edict_t *self, edict_t *inflictor, edict_t *attacker);
void GSLogEnter(edict_t *ent);
void GSLogExit(edict_t *ent);
#ifndef LINUX
int wsock32state(int mode);
#endif