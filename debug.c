/*
static int d_dummy_team = 1;
static edict_t *d_cur_dummy = NULL;
static gclient_t d_dummy_clients[16];

void d_make_dummy(edict_t *e)
{
        int i=0;
        gclient_t *client;
        edict_t *it = G_Spawn();

        while(i<16) {
                if(!d_dummy_clients[i].ar) {
                        client = &d_dummy_clients[i];
                        break;
                }
                i++;
        }

        VectorCopy(e->s.origin, it->s.origin);
        it->groundentity = NULL;
        it->takedamage = DAMAGE_NO;
        it->movetype = MOVETYPE_NONE;
        it->classname = "player";
        it->mass = 200;
        it->solid = SOLID_NOT;
//        it->clipmask = MASK_PLAYERSOLID;
        it->model = "models/items/armor/body/tris.md2";
//        it->model = "players/male/tris.md2";
        it->client = client;
        strcpy(it->client->pers.netname, va("Dummy%d", i));
//        it->s.skinnum = 1;

        gi.setmodel(it, it->model);
        gi.linkentity(it);

        init_player(it);
        it->client->ar->next3 = (void *)d_cur_dummy;
        d_cur_dummy = it;
        add_msg(va("Made dummy #%d\n", i));
//        gi.bprintf(PRINT_MEDIUM, "Made dummy #%d\n", i);
        it->health = i;
}

void d_cycle_dummy_team(void)
{
        if(++d_dummy_team == 5) {
                d_dummy_team = 1;
        }
        add_msg(va("dummy team is %d\n", d_dummy_team));
//        gi.bprintf(PRINT_MEDIUM, "dummy team is %d\n");
}

void d_ready_dummy(void)
{
        if(!d_cur_dummy) return;
        chose_team(d_cur_dummy, d_dummy_team);
}

void d_cycle_dummy(void)
{
        qboolean got = false;
        int i = 0;
        edict_t *e;

        e = d_cur_dummy;
        do {
                e = G_Find(e, FOFS(classname), "player");
                if(!e) {
                        i++;
                } else {
                        if(strncmp(e->client->pers.netname, "Dummy", 5)) continue;
                        d_cur_dummy = e;
                        add_msg(va("Current dummy is %s\n", e->client->pers.netname));
//                        gi.bprintf(PRINT_MEDIUM, "Current dummy is %s\n", e->client->pers.netname);
                        break;
                }
        } while(i < 2);
}

void d_skin_dummy(edict_t *e)
{
        add_msg(va("Skin #%d\n", e->s.skinnum));
}

*/

