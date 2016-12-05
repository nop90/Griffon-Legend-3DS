
#ifndef _CLIPBG_H_
#define _CLIPBG_H_

#include "3ds.h"

void setClipRect(u32 *clip, u32 x, u32 y, u32 w, u32 h, u32 col);

void setClipVal(u32 *clip, u32 x, u32 y, u32 col);

u32 getClipVal(u32 *clip, u32 x, u32 y);

u32* initClip(void);

void freeClip(u32 *clip);

u32 calcClipVal(u8 a, u8 b, u8 c);

void clipCopy(u32 *clip1,u32 *clip2);

void clipLine(u32 *clip, u32 x1, u32 y1, u32 x2, u32 y2, u32 col);

#endif

