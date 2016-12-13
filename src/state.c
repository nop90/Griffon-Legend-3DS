/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include "SDL_3ds.h"

#include "griffon.h"
#include "state.h"

char player_sav[256] = "/3ds/GriffonLegend/data/player%i.sav";

// externs (to be removed later)
extern PLAYERTYPE playera[4];
extern int asecstart[4];

int state_load(int slotnum)
{
	FILE *fp;
	char line[256];

	sprintf(line, player_sav, slotnum);

	fp = fopen(line, "rb");
	if(fp) {
		fread( line, sizeof(line), 1, fp );
		fread( &player, sizeof(PLAYERTYPE), 1, fp );
		fread( &secstart, sizeof(int), 1, fp );
		fread( &curmap, sizeof(int), 1, fp );
		fread( objmapf, sizeof(objmapf), 1, fp );
		fread( roomlocks, sizeof(roomlocks), 1, fp );

		fclose(fp);
		player.pause = SDL_GetTicks();
		return 1; // success
	}

	fclose(fp);
	return 0; // fail
}

/* fill PLAYERTYPE playera; */
int state_load_player(int slotnum)
{
	FILE *fp;
	char line[256];

	sprintf(line, player_sav, slotnum);

//	playera[slotnum].level = 0;

	fp = fopen(line, "rb");
	if(fp) {
		fread( line, sizeof(line), 1, fp );
		fread( &playera[slotnum], sizeof(PLAYERTYPE), 1, fp );
		fread( &asecstart[slotnum], sizeof(int), 1, fp );

		fclose(fp);
		return 1; // success
	}

	fclose(fp);

	return 0; // fail
}

int state_save(int slotnum)
{
	FILE *fp;
	char line[256];
	int time = secstart + secsingame;

	sprintf(line, player_sav, slotnum);

	fp = fopen(line, "wb+");
	if(fp) {

		fwrite( line, sizeof(line), 1, fp );
		fwrite( &player, sizeof(PLAYERTYPE), 1, fp );
		fwrite( &time, sizeof(int), 1, fp );
		fwrite( &curmap, sizeof(int), 1, fp );
		fwrite( objmapf, sizeof(objmapf), 1, fp );
		fwrite( roomlocks, sizeof(roomlocks), 1, fp );

		fclose(fp);
		return 1; // success
	}

	return 0; // fail
}
