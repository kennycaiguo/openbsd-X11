/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: plytxt16.m,v 1.10 94/04/17 21:12:05 rws Exp $
 */
>>TITLE PolyText16 XPROTO
>>SET startup protostartup
>>SET cleanup protocleanup
>>SET tpstartup tpfontstartup
>>SET tpcleanup tpfontcleanup
>>EXTERN
/* Touch test for PolyText16 request */

/****************************************************************************
 * Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                          *
 *                                                                          *
 *                         All Rights Reserved                              *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation for any purpose and without fee is hereby granted,         *
 * provided that the above copyright notice appears in all copies and that  *
 * both that copyright notice and this permission notice appear in          *
 * supporting documentation, and that the name of Sequent not be used       *
 * in advertising or publicity pertaining to distribution or use of the     *
 * software without specific, written prior permission.                     *
 *                                                                          *
 * SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS; IN NO EVENT SHALL *
 * SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 * SOFTWARE.                                                                *
 ****************************************************************************/

#include "Xstlib.h"


#define CLIENT 0
static TestType test_type = SETUP;
Window win;
xPolyText16Req *req;
xCreateGCReq *gc_req;
xOpenFontReq *font_req;

static
void
tester()
{
	Set_Init_Timer();

	Create_Client(CLIENT);

	win = Create_Default_Window(CLIENT);

	/* open the xtfont2 font to get its resource id for GContext */

	font_req = (xOpenFontReq *) Make_Req(CLIENT, X_OpenFont);
	font_req = (xOpenFontReq *) Clear_Counted_Value (font_req);
	font_req = (xOpenFontReq *) Add_Counted_Value (font_req, 'x');
	font_req = (xOpenFontReq *) Add_Counted_Value (font_req, 't');
	font_req = (xOpenFontReq *) Add_Counted_Value (font_req, 'f');
	font_req = (xOpenFontReq *) Add_Counted_Value (font_req, 'o');
	font_req = (xOpenFontReq *) Add_Counted_Value (font_req, 'n');
	font_req = (xOpenFontReq *) Add_Counted_Value (font_req, 't');
	font_req = (xOpenFontReq *) Add_Counted_Value (font_req, '2');
	Send_Req(CLIENT, (xReq *) font_req);
	Log_Trace("client %d sent default OpenFont request\n", CLIENT);
	(void) Expect_Nothing(CLIENT);

	/* create a default GContext with xtfont2 as the font */

	gc_req = (xCreateGCReq *) Make_Req (CLIENT, X_CreateGC);
	gc_req = (xCreateGCReq *) Add_Masked_Value (gc_req, GCFont, font_req->fid);
	Send_Req (CLIENT, (xReq *) gc_req);
	Log_Trace ("client %d sent CreateGC request\n", CLIENT);
	Expect_Nothing (CLIENT);
	Set_Default_GContext (CLIENT, gc_req->gc);
	Free_Req (gc_req);
	Free_Req (font_req);

	/* map window, send PolyText16 request, and check results visually */

	Map_Window(CLIENT, win);

	Set_Test_Type(CLIENT, test_type);
	req = (xPolyText16Req *) Make_Req(CLIENT, X_PolyText16);
	Send_Req(CLIENT, (xReq *) req);
	Set_Test_Type(CLIENT, GOOD);
	switch(test_type) {
	case GOOD:
		Log_Trace("client %d sent default PolyText16 request\n", CLIENT);
		(void) Expect_Nothing(CLIENT);
		Visual_Check();
		break;
	case BAD_LENGTH:
		Log_Trace("client %d sent PolyText16 request with bad length (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	case TOO_LONG:
	case JUST_TOO_LONG:
		Log_Trace("client %d sent overlong PolyText16 request (%d)\n", CLIENT, req->length);
		(void) Expect_BadLength(CLIENT);
		(void) Expect_Nothing(CLIENT);
		break;
	default:
		Log_Err("INTERNAL ERROR: test_type %d not one of GOOD(%d), BAD_LENGTH(%d), TOO_LONG(%d) or JUST_TOO_LONG(%d)\n",
			test_type, GOOD, BAD_LENGTH, TOO_LONG, JUST_TOO_LONG);
		Abort();
		/*NOTREACHED*/
		break;
	}
	Free_Req(req);
	Exit_OK();
}
>>ASSERTION Good A
When a client sends a valid xname protocol request to the X server,
then the X server does not send back an error, event or reply to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server does not send back an error, event or reply.
Open a connection to the X server using reversed byte sex.
Send a valid xname protocol request to the X server.
Verify that the X server does not send back an error, event or reply.
>>CODE

	test_type = GOOD;

	/* Call a library function to exercise the test code */
	testfunc(tester);

>>ASSERTION Bad A
When a client sends an invalid xname protocol request to the X server,
in which the length field of the request is not the minimum length required to 
contain the request,
then the X server sends back a BadLength error to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one less than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.

Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the minimum length required to contain the request.
Verify that the X server sends back a BadLength error.
>>CODE

	test_type = BAD_LENGTH; /* < minimum */

	/* Call a library function to exercise the test code */
	testfunc(tester);

	test_type = JUST_TOO_LONG; /* > minimum */

	/* Call a library function to exercise the test code */
	testfunc(tester);

>>ASSERTION Bad B 1
When a client sends an invalid xname protocol request to the X server,
in which the length field of the request exceeds the maximum length accepted
by the X server,
then the X server sends back a BadLength error to the client.
>>STRATEGY
Call library function testfunc() to do the following:
Open a connection to the X server using native byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the maximum length accepted by the server.
Verify that the X server sends back a BadLength error.
Open a connection to the X server using reversed byte sex.
Send an invalid xname protocol request to the X server with length 
  one greater than the maximum length accepted by the server.
Verify that the X server sends back a BadLength error.
>>CODE

	test_type = TOO_LONG;

	/* Call a library function to exercise the test code */
	testfunc(tester);
