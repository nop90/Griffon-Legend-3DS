
#include <string.h>
#include <stdlib.h>
#include <stdio.h> // for debug with printf
#include "3ds.h"

void setClipRect(u32 *clip, u32 x, u32 y, u32 w, u32 h, u32 col){
	int i,j;
	for (i=x; i<x+w; i++)
		for (j=y; j<y+h; j++)
			if(i*240+j < 240*320) clip[i*240+j] = col;
}

void setClipVal(u32 *clip, u32 x, u32 y, u32 col){
	if(x*240+y < 240*320) clip[x*240+y] = col;
}

u32 getClipVal(u32 *clip, u32 x, u32 y){
  if(x*240+y < 240*320) 
	return clip[x*240+y];
  else return 0;
}

u32* initClip(void){
	return (u32*) malloc(320*240*sizeof(u32));
}

void freeClip(u32 *clip){
	free(clip);
}

u32 calcClipVal(u8 a, u8 b, u8 c){
 return a | (b<<8) | (c<<16);
}

void clipCopy(u32 *clip1,u32 *clip2){
	memcpy((char*)clip2, (char*)clip1, 320* 240 * sizeof(u32));
}

void clipLine(u32 *clip, u32 x1, u32 y1, u32 x2, u32 y2, u32 col){
	int xdif = x2 - x1;
	int ydif = y2 - y1;

	if(xdif == 0) {
		for(int y = y1; y <= y2; y++) {
			if(x1*240+y < 240*320) clip[x1*240+y] = col;
		}
	}

	if(ydif == 0) {
		for(int x = x1; x <= x2; x++) {
			if(x*240+y1 < 240*320) clip[x*240+y1] = col;
		}
	}
}
