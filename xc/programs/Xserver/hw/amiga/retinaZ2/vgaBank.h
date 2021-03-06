/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 *
 * $Header: /tmp/OpenBSD-X11-repo/xc/programs/Xserver/hw/amiga/retinaZ2/vgaBank.h,v 1.1 1998/04/16 20:34:39 niklas Exp $
 */

#ifndef VGA_BANK_H
#define VGA_BANK_H

#include "retina.h"

#define BANK_FLAG(a) \
  vgaWriteFlag = (((unsigned long)a >= VGABASE && \
		   (unsigned long)a < (VGABASE + vgaSegmentSize)) ? TRUE : FALSE); \
  vgaReadFlag = FALSE;

#define BANK_FLAG_BOTH(a,b) \
  vgaReadFlag = (((unsigned long)a >= VGABASE && \
		   (unsigned long)a < (VGABASE + vgaSegmentSize)) ? TRUE : FALSE); \
  vgaWriteFlag  = (((unsigned long)b >= VGABASE && \
		   (unsigned long)b < (VGABASE + vgaSegmentSize)) ? TRUE : FALSE);

#define SETR(x)  { if(vgaReadFlag) x = vgaSetRead(x); }
#define SETW(x)  { if(vgaWriteFlag) x = vgaSetWrite(x); }
#define SETRW(x) { if(vgaWriteFlag) x = vgaSetReadWrite(x); }
#define CHECKRO(x) { if(vgaReadFlag && ((void *)x >= vgaReadTop)) \
			 x = vgaReadNext(x); }
#define CHECKRU(x) { if(vgaReadFlag && ((void *)x < vgaReadBottom)) \
			 x = vgaReadPrev(x); }
#define CHECKWO(x) { if(vgaWriteFlag && ((void *)x >= vgaWriteTop)) \
			 x = vgaWriteNext(x); }
#define CHECKWU(x) { if(vgaWriteFlag && ((void *)x < vgaWriteBottom)) \
			 x = vgaWritePrev(x); }
#define CHECKRWO(x) { if(vgaWriteFlag && ((void *)x >= vgaWriteTop)) \
			  x = vgaReadWriteNext(x); }
#define CHECKRWU(x) { if(vgaWriteFlag && ((void *)x < vgaWriteBottom)) \
			  x = vgaReadWritePrev(x); }

#define NEXTR(x) { x = vgaReadNext(x);}
#define NEXTW(x) { x = vgaWriteNext(x); }
#define PREVR(x) { x = vgaReadPrev(x); }
#define PREVW(x) { x = vgaWritePrev(x); }

#define SAVE_BANK()     { if (vgaWriteFlag) vgaSaveBank(); }
#define RESTORE_BANK()  { if (vgaWriteFlag) vgaRestoreBank(); }

#define PUSHR()         { if(vgaWriteFlag) vgaPushRead(); }
#define POPR()          { if(vgaWriteFlag) vgaPopRead(); }

#define TESTRWO(x)	(vgaWriteFlag && ((void *) x >= vgaWriteTop))

#endif /* VGA_BANK_H */
