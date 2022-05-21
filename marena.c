#include "g_local.h"

extern float	PlayersRangeFromSpot (edict_t *spot);


edict_t *SelectFarthestArenaSpawnPoint (char *classn, int arena)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), classn)) != NULL)
	{
		//gi.bprintf (PRINT_HIGH,"arena %d spot %d\n", arena, spot->arena);
		if (spot->arena != arena && arenas[arena].idarena==false) continue;
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
	while ((spot = G_Find (spot, FOFS(classname), classn)) != NULL && (spot->arena != arena))
	if (spot->arena == arena)
		return spot;
	else
		return NULL;
}
