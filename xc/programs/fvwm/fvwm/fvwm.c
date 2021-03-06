
/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/

/***********************************************************************
 * fvwm - "F? Virtual Window Manager"
 ***********************************************************************/

#include "../configure.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "fvwm.h"
#include "menus.h"
#include "misc.h"
#include "screen.h"
#include "parse.h"
#include "module.h"

#include <X11/Xproto.h>
#include <X11/Xatom.h>
/* need to get prototype for XrmUniqueQuark for XUniqueContext call */
#include <X11/Xresource.h>

#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif /* SHAPE */

#ifdef I18N
#include <X11/Xlocale.h>
#endif

#if defined (sparc) && defined (SVR4)
/* Solaris has sysinfo instead of gethostname.  */
#include <sys/systeminfo.h>
#endif

#define MAXHOSTNAME 255

#include "../version.h"

#ifndef lint
static char sccsid[] = "@(#)fvwm.c " VERSION " " __DATE__ " fvwm";
#endif

int master_pid;			/* process number of 1st fvwm process */

ScreenInfo Scr;		        /* structures for the screen */
Display *dpy;			/* which display are we talking to */

Window BlackoutWin=None;        /* window to hide window captures */

#ifdef FVWMRC
char *default_config_command = "Read "FVWMRC;
#else
char *default_config_command = "Read .fvwm2rc";
#endif
#define MAX_CFG_CMDS 10
static char *config_commands[MAX_CFG_CMDS];
static int num_config_commands=0;

char *output_file = NULL;

XErrorHandler FvwmErrorHandler(Display *, XErrorEvent *);
XIOErrorHandler CatchFatal(Display *);
XErrorHandler CatchRedirectError(Display *, XErrorEvent *);
void newhandler(int sig);
void CreateCursors(void);
void ChildDied(int nonsense);
void SaveDesktopState(void);
void SetMWM_INFO(Window window);
void SetRCDefaults(void);
void StartupStuff(void);

XContext FvwmContext;		/* context for fvwm windows */
XContext MenuContext;		/* context for fvwm menus */

int JunkX = 0, JunkY = 0;
Window JunkRoot, JunkChild;		/* junk window */
unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth, JunkMask;

Boolean debugging = False,PPosOverride,Blackout = False;

char **g_argv;
int g_argc;

/* assorted gray bitmaps for decorative borders */
#define g_width 2
#define g_height 2
static char g_bits[] = {0x02, 0x01};

#define l_g_width 4
#define l_g_height 2
static char l_g_bits[] = {0x08, 0x02};

#define s_g_width 4
#define s_g_height 4
static char s_g_bits[] = {0x01, 0x02, 0x04, 0x08};

#ifdef SHAPE
int ShapeEventBase, ShapeErrorBase;
Boolean ShapesSupported=False;
#endif

long isIconicState = 0;
extern XEvent Event;
Bool Restarting = False;
int fd_width, x_fd;
char *display_name = NULL;

/***********************************************************************
 *
 *  Procedure:
 *	main - start of fvwm
 *
 ***********************************************************************
 */
int main(int argc, char **argv)
{
  unsigned long valuemask;	/* mask for create windows */
  XSetWindowAttributes attributes;	/* attributes for create windows */
  void InternUsefulAtoms (void);
  void InitVariables(void);
  int i;
  extern int x_fd;
  int len;
  char *display_string;
  char message[255];
  Bool single = False;
  Bool option_error = FALSE;
  MenuRoot *mr;

  g_argv = argv;
  g_argc = argc;

  DBUG("main","Entered, about to parse args");

#ifdef I18N
  if (setlocale(LC_CTYPE, "") == NULL)
    fvwm_msg(ERR, "main", "Can't set locale. Check your $LC_CTYPE or $LANG.\n");
#endif
  for (i = 1; i < argc; i++) 
  {
    if (mystrncasecmp(argv[i],"-debug",6)==0)
    {
      debugging = True;
    }
    else if (mystrncasecmp(argv[i],"-s",2)==0)
    {
      single = True;
    }
    else if (mystrncasecmp(argv[i],"-d",2)==0)
    {
      if (++i >= argc)
        usage();
      display_name = argv[i];
    }
    else if (mystrncasecmp(argv[i],"-f",2)==0)
    {
      if (++i >= argc)
        usage();
      if (num_config_commands < MAX_CFG_CMDS)
      {
        config_commands[num_config_commands] =
          (char *)malloc(6+strlen(argv[i]));
        strcpy(config_commands[num_config_commands],"Read ");
        strcat(config_commands[num_config_commands],argv[i]);
        num_config_commands++;
      }
      else
      {
        fvwm_msg(ERR,"main","only %d -f and -cmd parms allowed!",MAX_CFG_CMDS);
      }
    }
    else if (mystrncasecmp(argv[i],"-cmd",4)==0)
    {
      if (++i >= argc)
        usage();
      if (num_config_commands < MAX_CFG_CMDS)
      {
        config_commands[num_config_commands] = strdup(argv[i]);
        num_config_commands++;
      }
      else
      {
        fvwm_msg(ERR,"main","only %d -f and -cmd parms allowed!",MAX_CFG_CMDS);
      }
    }
    else if (mystrncasecmp(argv[i],"-outfile",8)==0)
    {
      if (++i >= argc)
        usage();
      output_file = argv[i];
    }
    else if (mystrncasecmp(argv[i],"-h",2)==0)
    {
      usage();
      return 0;
    }
    else if (mystrncasecmp(argv[i],"-blackout",9)==0)
    {
      Blackout = True;
    }
    else if (mystrncasecmp(argv[i], "-version", 8) == 0)
    {
      fvwm_msg(INFO,"main", "Fvwm Version %s compiled on %s at %s\n",
              VERSION,__DATE__,__TIME__);
    }
    else
    {
      fvwm_msg(ERR,"main","Unknown option:  `%s'\n", argv[i]);
      option_error = TRUE;
    }
  }

  DBUG("main","Done parsing args");

  if (option_error)
  {
    usage();
  }
    
  DBUG("main","Installing signal handlers");

  newhandler (SIGINT);
  newhandler (SIGHUP);
  newhandler (SIGQUIT);
  newhandler (SIGTERM);
  signal (SIGUSR1, Restart);
  signal (SIGPIPE, DeadPipe);

  ReapChildren();

  if (!(dpy = XOpenDisplay(display_name))) 
  {
    fvwm_msg(ERR,"main","can't open display %s", XDisplayName(display_name));
    return 1;
  }
  Scr.screen= DefaultScreen(dpy);
  Scr.NumberOfScreens = ScreenCount(dpy);

  master_pid = getpid();

  if(!single)
  {
    int	myscreen = 0;
    char *cp;

    strcpy(message, XDisplayString(dpy));

    for(i=0;i<Scr.NumberOfScreens;i++)
    {
      if (i != Scr.screen && fork() == 0)
      {
        myscreen = i;

        /*
         * Truncate the string 'whatever:n.n' to 'whatever:n',
         * and then append the screen number.
         */
        cp = strchr(message, ':');
        if (cp != NULL)
        {
          cp = strchr(cp, '.');
          if (cp != NULL)
            *cp = '\0';		/* truncate at display part */
        }
        sprintf(message + strlen(message), ".%d", myscreen);
        dpy = XOpenDisplay(message);
        Scr.screen = myscreen;
        Scr.NumberOfScreens = ScreenCount(dpy);

        break;
      }
    }
  }

  x_fd = XConnectionNumber(dpy);
  fd_width = GetFdWidth();
    
  if (fcntl(x_fd, F_SETFD, 1) == -1) 
  {
    fvwm_msg(ERR,"main","close-on-exec failed");
    return 1;
  }
	    
  /*  Add a DISPLAY entry to the environment, incase we were started
   * with fvwm -display term:0.0
   */
  len = strlen(XDisplayString(dpy));
  display_string = safemalloc(len+10);
  sprintf(display_string,"DISPLAY=%s",XDisplayString(dpy));
  putenv(display_string);
  /* Add a HOSTDISPLAY environment variable, which is the same as
   * DISPLAY, unless display = :0.0 or unix:0.0, in which case the full
   * host name will be used for ease in networking . */
  /* Note: Can't free the rdisplay_string after putenv, because it
   * becomes part of the environment! */
  if(strncmp(display_string,"DISPLAY=:",9)==0)
  {
    char client[MAXHOSTNAME], *rdisplay_string;
    mygethostname(client,MAXHOSTNAME);
    rdisplay_string = safemalloc(len+14 + strlen(client));
    sprintf(rdisplay_string,"HOSTDISPLAY=%s:%s",client,&display_string[9]);
    putenv(rdisplay_string);
  }
  else if(strncmp(display_string,"DISPLAY=unix:",13)==0)
  {
    char client[MAXHOSTNAME], *rdisplay_string;
    mygethostname(client,MAXHOSTNAME);
    rdisplay_string = safemalloc(len+14 + strlen(client));
    sprintf(rdisplay_string,"HOSTDISPLAY=%s:%s",client,
            &display_string[13]);
    putenv(rdisplay_string);
  }
  else
  {
    char *rdisplay_string;
    rdisplay_string = safemalloc(len+14);
    sprintf(rdisplay_string,"HOSTDISPLAY=%s",XDisplayString(dpy));
    putenv(rdisplay_string);
  }

  Scr.Root = RootWindow(dpy, Scr.screen);
  if(Scr.Root == None) 
  {
    fvwm_msg(ERR,"main","Screen %d is not a valid screen",(char *)Scr.screen);
    return 1;
  }


#ifdef SHAPE
  ShapesSupported=XShapeQueryExtension(dpy, &ShapeEventBase, &ShapeErrorBase);
#endif /* SHAPE */

  InternUsefulAtoms ();

  /* Make sure property priority colors is empty */
  XChangeProperty (dpy, Scr.Root, _XA_MIT_PRIORITY_COLORS,
                   XA_CARDINAL, 32, PropModeReplace, NULL, 0);


  XSetErrorHandler((XErrorHandler)CatchRedirectError);
  XSetIOErrorHandler((XIOErrorHandler)CatchFatal);
  XSelectInput(dpy, Scr.Root,
               LeaveWindowMask| EnterWindowMask | PropertyChangeMask | 
               SubstructureRedirectMask | KeyPressMask | 
               SubstructureNotifyMask|
               ButtonPressMask | ButtonReleaseMask );
  XSync(dpy, 0);

  XSetErrorHandler((XErrorHandler)FvwmErrorHandler);

  BlackoutScreen(); /* if they want to hide the capture/startup */

  CreateCursors();
  InitVariables();
  InitEventHandlerJumpTable();
  initModules();

  Scr.gray_bitmap = 
    XCreateBitmapFromData(dpy,Scr.Root,g_bits, g_width,g_height);


  DBUG("main","Setting up rc file defaults...");
  SetRCDefaults();

  DBUG("main","Running config_commands...");
  if (num_config_commands > 0)
  {
    int i;
    for(i=0;i<num_config_commands;i++)
    {
      ExecuteFunction(config_commands[i], NULL,&Event,C_ROOT,-1);
      free(config_commands[i]);
    }
  }
  else
  {
    ExecuteFunction(default_config_command, NULL,&Event,C_ROOT,-1);
  }
  DBUG("main","Done running config_commands");

  if(Scr.d_depth<2)
  {
    Scr.gray_pixmap = 
      XCreatePixmapFromBitmapData(dpy,Scr.Root,g_bits, g_width,g_height,
                                  Scr.MenuColors.fore,Scr.MenuColors.back,
                                  Scr.d_depth);	
    Scr.light_gray_pixmap = 
      XCreatePixmapFromBitmapData(dpy,Scr.Root,l_g_bits,l_g_width,l_g_height,
                                  Scr.MenuColors.fore,Scr.MenuColors.back,
                                  Scr.d_depth);
  }

  /* create a window which will accept the keyboard focus when no other 
     windows have it */
  attributes.event_mask = KeyPressMask|FocusChangeMask;
  attributes.override_redirect = True;
  Scr.NoFocusWin=XCreateWindow(dpy,Scr.Root,-10, -10, 10, 10, 0, 0,
                               InputOnly,CopyFromParent,
                               CWEventMask|CWOverrideRedirect,
                               &attributes);
  XMapWindow(dpy, Scr.NoFocusWin);

  SetMWM_INFO(Scr.NoFocusWin);

  XSetInputFocus (dpy, Scr.NoFocusWin, RevertToParent, CurrentTime);

  XSync(dpy, 0);
  if(debugging)
    XSynchronize(dpy,1);

  Scr.SizeStringWidth = XTextWidth (Scr.StdFont.font,
                                    " +8888 x +8888 ", 15);
  attributes.border_pixel = Scr.MenuColors.fore;
  attributes.background_pixel = Scr.MenuColors.back;
  attributes.bit_gravity = NorthWestGravity;
  valuemask = (CWBorderPixel | CWBackPixel | CWBitGravity);
  if(!(Scr.flags & MWMMenus))
  {
    Scr.SizeWindow = XCreateWindow (dpy, Scr.Root,
                                    0, 0, 
                                    (unsigned int)(Scr.SizeStringWidth +
                                                   SIZE_HINDENT*2),
                                    (unsigned int) (Scr.StdFont.height +
                                                    SIZE_VINDENT*2),
                                    (unsigned int) 0, 0,
                                    (unsigned int) CopyFromParent,
                                    (Visual *) CopyFromParent,
                                    valuemask, &attributes);
  }
  else
  {
    Scr.SizeWindow = XCreateWindow (dpy, Scr.Root,
                                    Scr.MyDisplayWidth/2 - 
                                    (Scr.SizeStringWidth +
                                     SIZE_HINDENT*2)/2,
                                    Scr.MyDisplayHeight/2 -
                                    (Scr.StdFont.height + 
                                     SIZE_VINDENT*2)/2, 
                                    (unsigned int)(Scr.SizeStringWidth +
                                                   SIZE_HINDENT*2),
                                    (unsigned int) (Scr.StdFont.height +
                                                    SIZE_VINDENT*2),
                                    (unsigned int) 0, 0,
                                    (unsigned int) CopyFromParent,
                                    (Visual *) CopyFromParent,
                                    valuemask, &attributes);
  }
#ifndef NON_VIRTUAL
  initPanFrames();
#endif

  MyXGrabServer(dpy);

#ifndef NON_VIRTUAL
  checkPanFrames();
#endif
  MyXUngrabServer(dpy);
  UnBlackoutScreen(); /* if we need to remove blackout window */
  DBUG("main","Entering HandleEvents loop...");
  HandleEvents();
  DBUG("main","Back from HandleEvents loop?  Exitting...");
  return;
}


/*
** StartupStuff
**
** Does initial window captures and runs init/restart function
*/
void StartupStuff(void)
{
  MenuRoot *mr;

  CaptureAllWindows();
  MakeMenus();
      
  if(Restarting)
  {
    mr = FindPopup("RestartFunction");
    if(mr != NULL)
      ExecuteFunction("Function RestartFunction",NULL,&Event,C_ROOT,-1);
  }
  else
  {
    mr = FindPopup("InitFunction");
    if(mr != NULL)
      ExecuteFunction("Function InitFunction",NULL,&Event,C_ROOT,-1);
  }
} /* StartupStuff */


/***********************************************************************
 *
 *  Procedure:
 *      CaptureAllWindows
 *
 *   Decorates all windows at start-up and during recaptures
 *
 ***********************************************************************/

void CaptureAllWindows(void)
{
  int i,j;
  unsigned int nchildren;
  Window root, parent, *children;
  FvwmWindow *tmp,*next;		/* temp fvwm window structure */
  Window w;
  unsigned long data[1];
  unsigned char *prop;
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;

  MyXGrabServer(dpy);

  if(!XQueryTree(dpy, Scr.Root, &root, &parent, &children, &nchildren))
  {
    MyXUngrabServer(dpy);
    return;
  }

  PPosOverride = True;

  if (!(Scr.flags & WindowsCaptured)) /* initial capture? */
  {
    /*
    ** weed out icon windows
    */
    for (i=0;i<nchildren;i++) 
    {
      if (children[i]) 
      {
        XWMHints *wmhintsp = XGetWMHints (dpy, children[i]);
        if (wmhintsp) 
        {
          if (wmhintsp->flags & IconWindowHint) 
          {
            for (j = 0; j < nchildren; j++) 
            {
              if (children[j] == wmhintsp->icon_window) 
              {
                children[j] = None;
                break;
              }
            }
          }
          XFree ((char *) wmhintsp);
        }
      }
    }
    /*
    ** map all of the non-override, non-icon windows
    */
    for (i = 0; i < nchildren; i++)
    {
      if (children[i] && MappedNotOverride(children[i]))
      {
        XUnmapWindow(dpy, children[i]);
        Event.xmaprequest.window = children[i];
        HandleMapRequestKeepRaised (BlackoutWin);
      }
    }
    Scr.flags |= WindowsCaptured;
  }
  else /* must be recapture */
  {
    /* reborder all windows */
    tmp = Scr.FvwmRoot.next;
    for(i=0;i<nchildren;i++)
    {
      if(XFindContext(dpy, children[i], FvwmContext, 
                      (caddr_t *)&tmp)!=XCNOENT)
      {
        isIconicState = DontCareState;
        if(XGetWindowProperty(dpy,tmp->w,_XA_WM_STATE,0L,3L,False,
                              _XA_WM_STATE,
                              &atype,&aformat,&nitems,&bytes_remain,&prop)==
           Success)
        {
          if(prop != NULL)
          {
            isIconicState = *(long *)prop;
            XFree(prop);
          }
        }
        next = tmp->next;
        data[0] = (unsigned long) tmp->Desk;
        XChangeProperty (dpy, tmp->w, _XA_WM_DESKTOP, _XA_WM_DESKTOP, 32,
                         PropModeReplace, (unsigned char *) data, 1);
        
        XSelectInput(dpy, tmp->w, 0);
        w = tmp->w;
        XUnmapWindow(dpy,tmp->frame);
        XUnmapWindow(dpy,w);
        RestoreWithdrawnLocation (tmp,True); 
        Destroy(tmp);
        Event.xmaprequest.window = w;
        HandleMapRequestKeepRaised(BlackoutWin);
        tmp = next;
      }
    }
  }

  isIconicState = DontCareState;

  if(nchildren > 0)
    XFree((char *)children);

  /* after the windows already on the screen are in place,
   * don't use PPosition */
  PPosOverride = False;
  KeepOnTop();
  MyXUngrabServer(dpy);
  XSync(dpy,0); /* should we do this on initial capture? */
}

/*
** SetRCDefaults
**
** Sets some initial style values & such
*/
void SetRCDefaults()
{
  /* set up default colors, fonts, etc */
  char *defaults[] = {
    "HilightColor black grey",
    "XORValue 0",
    "MenuStyle black grey slategrey fixed fvwm",
    "TitleStyle Centered -- Raised",
    "IconFont fixed",
    "WindowFont fixed",
    "Style \"*\" Color lightgrey/dimgrey, Title",
    "Style \"*\" RandomPlacement, SmartPlacement",
    "AddToMenu builtin_menu \"Builtin Menu\" Title",
    "+ \"Exit FVWM\" Quit",
    "Mouse 1 R N Popup builtin_menu",
    "AddToFunc WindowListFunc \"I\" WindowId $0 Iconify -1",
    "+ \"I\" WindowId $0 Focus",
    "+ \"I\" WindowId $0 Raise",
    "+ \"I\" WindowId $0 WarpToWindow 5p 5p",
    NULL
  };
  int i=0;

  while (defaults[i])
  {
    ExecuteFunction(defaults[i],NULL,&Event,C_ROOT,-1);
    i++;
  }
} /* SetRCDefaults */

/***********************************************************************
 *
 *  Procedure:
 *	MappedNotOverride - checks to see if we should really
 *		put a fvwm frame on the window
 *
 *  Returned Value:
 *	TRUE	- go ahead and frame the window
 *	FALSE	- don't frame the window
 *
 *  Inputs:
 *	w	- the window to check
 *
 ***********************************************************************/

int MappedNotOverride(Window w)
{
  XWindowAttributes wa;
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;
  unsigned char *prop;

  isIconicState = DontCareState;

  if((w==Scr.NoFocusWin)||(!XGetWindowAttributes(dpy, w, &wa)))
    return False;

  if(XGetWindowProperty(dpy,w,_XA_WM_STATE,0L,3L,False,_XA_WM_STATE,
			&atype,&aformat,&nitems,&bytes_remain,&prop)==Success)
  {
    if(prop != NULL)
    {
      isIconicState = *(long *)prop;
      XFree(prop);
    }
  }
  if(wa.override_redirect == True)
  {
    XSelectInput(dpy,w,FocusChangeMask);
  }
  return (((isIconicState == IconicState)||(wa.map_state != IsUnmapped)) &&
	  (wa.override_redirect != True));
}


/***********************************************************************
 *
 *  Procedure:
 *	InternUsefulAtoms:
 *            Dont really know what it does
 *
 ***********************************************************************
 */
Atom _XA_MIT_PRIORITY_COLORS;
Atom _XA_WM_CHANGE_STATE;
Atom _XA_WM_STATE;
Atom _XA_WM_COLORMAP_WINDOWS;
extern Atom _XA_WM_PROTOCOLS;
Atom _XA_WM_TAKE_FOCUS;
Atom _XA_WM_DELETE_WINDOW;
Atom _XA_WM_DESKTOP;
Atom _XA_MwmAtom;
Atom _XA_MOTIF_WM;
 
Atom _XA_OL_WIN_ATTR;
Atom _XA_OL_WT_BASE;
Atom _XA_OL_WT_CMD;
Atom _XA_OL_WT_HELP;
Atom _XA_OL_WT_NOTICE;
Atom _XA_OL_WT_OTHER;
Atom _XA_OL_DECOR_ADD;
Atom _XA_OL_DECOR_DEL;
Atom _XA_OL_DECOR_CLOSE;
Atom _XA_OL_DECOR_RESIZE;
Atom _XA_OL_DECOR_HEADER;
Atom _XA_OL_DECOR_ICON_NAME;

void InternUsefulAtoms (void)
{
  /* 
   * Create priority colors if necessary.
   */
  _XA_MIT_PRIORITY_COLORS = XInternAtom(dpy, "_MIT_PRIORITY_COLORS", False);   
  _XA_WM_CHANGE_STATE = XInternAtom (dpy, "WM_CHANGE_STATE", False);
  _XA_WM_STATE = XInternAtom (dpy, "WM_STATE", False);
  _XA_WM_COLORMAP_WINDOWS = XInternAtom (dpy, "WM_COLORMAP_WINDOWS", False);
  _XA_WM_PROTOCOLS = XInternAtom (dpy, "WM_PROTOCOLS", False);
  _XA_WM_TAKE_FOCUS = XInternAtom (dpy, "WM_TAKE_FOCUS", False);
  _XA_WM_DELETE_WINDOW = XInternAtom (dpy, "WM_DELETE_WINDOW", False);
  _XA_WM_DESKTOP = XInternAtom (dpy, "WM_DESKTOP", False);
  _XA_MwmAtom=XInternAtom(dpy,"_MOTIF_WM_HINTS",False);
  _XA_MOTIF_WM=XInternAtom(dpy,"_MOTIF_WM_INFO",False);

  _XA_OL_WIN_ATTR=XInternAtom(dpy,"_OL_WIN_ATTR",False);
  _XA_OL_WT_BASE=XInternAtom(dpy,"_OL_WT_BASE",False);
  _XA_OL_WT_CMD=XInternAtom(dpy,"_OL_WT_CMD",False);
  _XA_OL_WT_HELP=XInternAtom(dpy,"_OL_WT_HELP",False);
  _XA_OL_WT_NOTICE=XInternAtom(dpy,"_OL_WT_NOTICE",False);
  _XA_OL_WT_OTHER=XInternAtom(dpy,"_OL_WT_OTHER",False);
  _XA_OL_DECOR_ADD=XInternAtom(dpy,"_OL_DECOR_ADD",False);
  _XA_OL_DECOR_DEL=XInternAtom(dpy,"_OL_DECOR_DEL",False);
  _XA_OL_DECOR_CLOSE=XInternAtom(dpy,"_OL_DECOR_CLOSE",False);
  _XA_OL_DECOR_RESIZE=XInternAtom(dpy,"_OL_DECOR_RESIZE",False);
  _XA_OL_DECOR_HEADER=XInternAtom(dpy,"_OL_DECOR_HEADER",False);
  _XA_OL_DECOR_ICON_NAME=XInternAtom(dpy,"_OL_DECOR_ICON_NAME",False);
  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	newhandler: Installs new signal handler
 *
 ************************************************************************/
void newhandler(int sig)
{
  if (signal (sig, SIG_IGN) != SIG_IGN)
    signal (sig, SigDone);
}


/*************************************************************************
 * Restart on a signal
 ************************************************************************/
void Restart(int nonsense)
{
  Done(1, *g_argv);
  SIGNAL_RETURN;
}

/***********************************************************************
 *
 *  Procedure:
 *	CreateCursors - Loads fvwm cursors
 *
 ***********************************************************************
 */
void CreateCursors(void)
{
  /* define cursors */
  Scr.FvwmCursors[POSITION] = XCreateFontCursor(dpy,XC_top_left_corner);
  Scr.FvwmCursors[DEFAULT] = XCreateFontCursor(dpy, XC_top_left_arrow);
  Scr.FvwmCursors[SYS] = XCreateFontCursor(dpy, XC_hand2);
  Scr.FvwmCursors[TITLE_CURSOR] = XCreateFontCursor(dpy, XC_top_left_arrow);
  Scr.FvwmCursors[MOVE] = XCreateFontCursor(dpy, XC_fleur);
  Scr.FvwmCursors[MENU] = XCreateFontCursor(dpy, XC_sb_left_arrow);
  Scr.FvwmCursors[WAIT] = XCreateFontCursor(dpy, XC_watch);
  Scr.FvwmCursors[SELECT] = XCreateFontCursor(dpy, XC_dot);
  Scr.FvwmCursors[DESTROY] = XCreateFontCursor(dpy, XC_pirate);
  Scr.FvwmCursors[LEFT] = XCreateFontCursor(dpy, XC_left_side);
  Scr.FvwmCursors[RIGHT] = XCreateFontCursor(dpy, XC_right_side);
  Scr.FvwmCursors[TOP] = XCreateFontCursor(dpy, XC_top_side);
  Scr.FvwmCursors[BOTTOM] = XCreateFontCursor(dpy, XC_bottom_side);
  Scr.FvwmCursors[TOP_LEFT] = XCreateFontCursor(dpy,XC_top_left_corner);
  Scr.FvwmCursors[TOP_RIGHT] = XCreateFontCursor(dpy,XC_top_right_corner);
  Scr.FvwmCursors[BOTTOM_LEFT] = XCreateFontCursor(dpy,XC_bottom_left_corner);
  Scr.FvwmCursors[BOTTOM_RIGHT] =XCreateFontCursor(dpy,XC_bottom_right_corner);
}

/***********************************************************************
 *
 *  LoadDefaultLeftButton -- loads default left button # into 
 *		assumes associated button memory is already free
 * 
 ************************************************************************/
void LoadDefaultLeftButton(ButtonFace *bf, int i)
{
#ifndef VECTOR_BUTTONS
    bf->style = SimpleButton;
#else
    bf->style = VectorButton;
    switch (i % 5)
    {
    case 0:
    case 4:
	bf->vector.x[0] = 22;
	bf->vector.y[0] = 39;
	bf->vector.line_style[0] = 1;
	bf->vector.x[1] = 78;
	bf->vector.y[1] = 39;
	bf->vector.line_style[1] = 1;
	bf->vector.x[2] = 78;
	bf->vector.y[2] = 61;
	bf->vector.line_style[2] = 0;
	bf->vector.x[3] = 22;
	bf->vector.y[3] = 61;
	bf->vector.line_style[3] = 0;
	bf->vector.x[4] = 22;
	bf->vector.y[4] = 39;
	bf->vector.line_style[4] = 1;
	bf->vector.num = 5;
	break;
    case 1:
	bf->vector.x[0] = 32;
	bf->vector.y[0] = 45;
	bf->vector.line_style[0] = 0;
	bf->vector.x[1] = 68;
	bf->vector.y[1] = 45;
	bf->vector.line_style[1] = 0;
	bf->vector.x[2] = 68;
	bf->vector.y[2] = 55;
	bf->vector.line_style[2] = 1;
	bf->vector.x[3] = 32;
	bf->vector.y[3] = 55;
	bf->vector.line_style[3] = 1;
	bf->vector.x[4] = 32;
	bf->vector.y[4] = 45;
	bf->vector.line_style[4] = 0;
	bf->vector.num = 5;
	break;
    case 2:
	bf->vector.x[0] = 49;
	bf->vector.y[0] = 49;
	bf->vector.line_style[0] = 1;
	bf->vector.x[1] = 51;
	bf->vector.y[1] = 49;
	bf->vector.line_style[1] = 1;
	bf->vector.x[2] = 51;
	bf->vector.y[2] = 51;
	bf->vector.line_style[2] = 0;
	bf->vector.x[3] = 49;
	bf->vector.y[3] = 51;
	bf->vector.line_style[3] = 0;
	bf->vector.x[4] = 49;
	bf->vector.y[4] = 49;
	bf->vector.line_style[4] = 1;
	bf->vector.num = 5;
	break;
    case 3:
	bf->vector.x[0] = 32;
	bf->vector.y[0] = 45;
	bf->vector.line_style[0] = 1;
	bf->vector.x[1] = 68;
	bf->vector.y[1] = 45;
	bf->vector.line_style[1] = 1;
	bf->vector.x[2] = 68;
	bf->vector.y[2] = 55;
	bf->vector.line_style[2] = 0;
	bf->vector.x[3] = 32;
	bf->vector.y[3] = 55;
	bf->vector.line_style[3] = 0;
	bf->vector.x[4] = 32;
	bf->vector.y[4] = 45;
	bf->vector.line_style[4] = 1;
	bf->vector.num = 5;
	break;
    }
#endif /* VECTOR_BUTTONS */
}

/***********************************************************************
 *
 *  LoadDefaultRightButton -- loads default left button # into
 *		assumes associated button memory is already free
 * 
 ************************************************************************/
void LoadDefaultRightButton(ButtonFace *bf, int i)
{
#ifndef VECTOR_BUTTONS
    bf->style = SimpleButton;
#else
    bf->style = VectorButton;
    switch (i % 5)
    {
    case 0:
    case 3:
	bf->vector.x[0] = 25;
	bf->vector.y[0] = 25;
	bf->vector.line_style[0] = 1;
	bf->vector.x[1] = 75;
	bf->vector.y[1] = 25;
	bf->vector.line_style[1] = 1;
	bf->vector.x[2] = 75;
	bf->vector.y[2] = 75;
	bf->vector.line_style[2] = 0;
	bf->vector.x[3] = 25;
	bf->vector.y[3] = 75;
	bf->vector.line_style[3] = 0;
	bf->vector.x[4] = 25;
	bf->vector.y[4] = 25;
	bf->vector.line_style[4] = 1;
	bf->vector.num = 5;
	break;
    case 1:
	bf->vector.x[0] = 39;
	bf->vector.y[0] = 39;
	bf->vector.line_style[0] = 1;
	bf->vector.x[1] = 61;
	bf->vector.y[1] = 39;
	bf->vector.line_style[1] = 1;
	bf->vector.x[2] = 61;
	bf->vector.y[2] = 61;
	bf->vector.line_style[2] = 0;
	bf->vector.x[3] = 39;
	bf->vector.y[3] = 61;
	bf->vector.line_style[3] = 0;
	bf->vector.x[4] = 39;
	bf->vector.y[4] = 39;
	bf->vector.line_style[4] = 1;
	bf->vector.num = 5;
	break;
    case 2:
	bf->vector.x[0] = 49;
	bf->vector.y[0] = 49;
	bf->vector.line_style[0] = 1;
	bf->vector.x[1] = 51;
	bf->vector.y[1] = 49;
	bf->vector.line_style[1] = 1;
	bf->vector.x[2] = 51;
	bf->vector.y[2] = 51;
	bf->vector.line_style[2] = 0;
	bf->vector.x[3] = 49;
	bf->vector.y[3] = 51;
	bf->vector.line_style[3] = 0;
	bf->vector.x[4] = 49;
	bf->vector.y[4] = 49;
	bf->vector.line_style[4] = 1;
	bf->vector.num = 5;
	break;
    case 4:
	bf->vector.x[0] = 36;
	bf->vector.y[0] = 36;
	bf->vector.line_style[0] = 1;
	bf->vector.x[1] = 64;
	bf->vector.y[1] = 36;
	bf->vector.line_style[1] = 1;
	bf->vector.x[2] = 64;
	bf->vector.y[2] = 64;
	bf->vector.line_style[2] = 0;
	bf->vector.x[3] = 36;
	bf->vector.y[3] = 64;
	bf->vector.line_style[3] = 0;
	bf->vector.x[4] = 36;
	bf->vector.y[4] = 36;
	bf->vector.line_style[4] = 1;
	bf->vector.num = 5;
	break;
    }
#endif /* VECTOR_BUTTONS */
}

/***********************************************************************
 *
 *  LoadDefaultButton -- loads default button # into button structure
 *		assumes associated button memory is already free
 * 
 ************************************************************************/
void LoadDefaultButton(ButtonFace *bf, int i)
{
    int n = i / 2;
    if ((n * 2) == i) {
	if (--n < 0) n = 4;
	LoadDefaultRightButton(bf, n);
    } else
	LoadDefaultLeftButton(bf, n);
}

extern void FreeButtonFace(Display *dpy, ButtonFace *bf);

/***********************************************************************
 *
 *  ResetAllButtons -- resets all buttons to defaults
 *                 destroys existing buttons
 * 
 ************************************************************************/
void ResetAllButtons(FvwmDecor *fl)
{
    int i = 0;
    for (; i < 5; ++i) {
	int j;

	FreeButtonFace(dpy, &fl->left_buttons[i].state[0]);
	FreeButtonFace(dpy, &fl->right_buttons[i].state[0]);

	LoadDefaultLeftButton(&fl->left_buttons[i].state[0], i);
	LoadDefaultRightButton(&fl->right_buttons[i].state[0], i);

	for (j = 1; j < MaxButtonState; ++j) {
	    FreeButtonFace(dpy, &fl->left_buttons[i].state[j]);
	    FreeButtonFace(dpy, &fl->right_buttons[i].state[j]);

	    fl->left_buttons[i].state[j] = fl->left_buttons[i].state[0];
	    fl->right_buttons[i].state[j] = fl->right_buttons[i].state[0];
	}
    }
    /* standard MWM decoration hint assignments (veliaa@rpi.edu)
       [Menu]  - Title Bar - [Minimize] [Maximize] */
    fl->left_buttons[0].flags |= MWMDecorMenu;
    fl->right_buttons[1].flags |= MWMDecorMinimize;
    fl->right_buttons[0].flags |= MWMDecorMaximize;
}

/***********************************************************************
 *
 *  DestroyFvwmDecor -- frees all memory assocated with an FvwmDecor
 *	structure, but does not free the FvwmDecor itself
 * 
 ************************************************************************/
void DestroyFvwmDecor(FvwmDecor *fl)
{
  int i;
  /* reset to default button set (frees allocated mem) */
  ResetAllButtons(fl);
  for (i = 0; i < 3; ++i)
  {
      int j = 0;
      for (; j < MaxButtonState; ++j)
	  FreeButtonFace(dpy, &fl->titlebar.state[i]);
  }
#ifdef BORDERSTYLE
  FreeButtonFace(dpy, &fl->BorderStyle.active);
  FreeButtonFace(dpy, &fl->BorderStyle.inactive);
#endif
#ifdef USEDECOR
  if (fl->tag) {
      free(fl->tag);
      fl->tag = NULL;
  }
#endif
  if (fl->HiReliefGC != NULL) {
      XFreeGC(dpy, fl->HiReliefGC);
      fl->HiReliefGC = NULL;
  }
  if (fl->HiShadowGC != NULL) {
      XFreeGC(dpy, fl->HiShadowGC);
      fl->HiShadowGC = NULL;
  }
}

/***********************************************************************
 *
 *  InitFvwmDecor -- initializes an FvwmDecor structure to defaults
 * 
 ************************************************************************/
void InitFvwmDecor(FvwmDecor *fl)
{
    int i;
    ButtonFace tmpbf;
    
    fl->HiReliefGC = NULL;
    fl->HiShadowGC = NULL;

#ifdef USEDECOR
    fl->tag = NULL;
    fl->next = NULL;

    if (fl != &Scr.DefaultDecor) {
	extern void AddToDecor(FvwmDecor *, char *);
	AddToDecor(fl, "HilightColor black grey");
	AddToDecor(fl, "WindowFont fixed");
    }
#endif

    /* initialize title-bar button styles */
    tmpbf.style = SimpleButton;
#ifdef MULTISTYLE
    tmpbf.next = NULL;
#endif
    for (i = 0; i < 5; ++i) {
	int j = 0;
	for (; j < MaxButtonState; ++j) {
	    fl->left_buttons[i].state[j] =
		fl->right_buttons[i].state[j] =  tmpbf;
	}
    }
    
    /* reset to default button set */
    ResetAllButtons(fl);
    
    /* initialize title-bar styles */
    fl->titlebar.flags = 0;
    
    for (i = 0; i < MaxButtonState; ++i) {
	fl->titlebar.state[i].style = SimpleButton;
#ifdef MULTISTYLE
	fl->titlebar.state[i].next = NULL;
#endif
    }

#ifdef BORDERSTYLE
    /* initialize border texture styles */
    fl->BorderStyle.active.style = SimpleButton;
    fl->BorderStyle.inactive.style = SimpleButton;
#ifdef MULTISTYLE
    fl->BorderStyle.active.next = NULL;
    fl->BorderStyle.inactive.next = NULL;
#endif
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *	InitVariables - initialize fvwm variables
 *
 ************************************************************************/
void InitVariables(void)
{
  FvwmContext = XUniqueContext();
  MenuContext = XUniqueContext();

  /* initialize some lists */
  Scr.AllBindings = NULL;
  Scr.AllMenus = NULL;
  Scr.TheList = NULL;
  
  Scr.DefaultIcon = NULL;


  /* create graphics contexts */
  CreateGCs();

  Scr.d_depth = DefaultDepth(dpy, Scr.screen);
  Scr.FvwmRoot.w = Scr.Root;
  Scr.FvwmRoot.next = 0;
  XGetWindowAttributes(dpy,Scr.Root,&(Scr.FvwmRoot.attr));
  Scr.root_pushes = 0;
  Scr.pushed_window = &Scr.FvwmRoot;
  Scr.FvwmRoot.number_cmap_windows = 0;
  

  Scr.MyDisplayWidth = DisplayWidth(dpy, Scr.screen);
  Scr.MyDisplayHeight = DisplayHeight(dpy, Scr.screen);
    
  Scr.NoBoundaryWidth = 1;
  Scr.BoundaryWidth = BOUNDARY_WIDTH;
  Scr.CornerWidth = CORNER_WIDTH;
  Scr.Hilite = NULL;
  Scr.Focus = NULL;
  Scr.Ungrabbed = NULL;
  
  Scr.StdFont.font = NULL;

#ifndef NON_VIRTUAL  
  Scr.VxMax = 2*Scr.MyDisplayWidth;
  Scr.VyMax = 2*Scr.MyDisplayHeight;
#else
  Scr.VxMax = 0;
  Scr.VyMax = 0;
#endif
  Scr.Vx = Scr.Vy = 0;

  Scr.SizeWindow = None;

  /* Sets the current desktop number to zero */
  /* Multiple desks are available even in non-virtual
   * compilations */
  {
    Atom atype;
    int aformat;
    unsigned long nitems, bytes_remain;
    unsigned char *prop;
    
    Scr.CurrentDesk = 0;
    if ((XGetWindowProperty(dpy, Scr.Root, _XA_WM_DESKTOP, 0L, 1L, True,
			    _XA_WM_DESKTOP, &atype, &aformat, &nitems,
			    &bytes_remain, &prop))==Success)
    {
      if(prop != NULL)
      {
        Restarting = True;
        Scr.CurrentDesk = *(unsigned long *)prop;
      }
    }
  }

  Scr.EdgeScrollX = Scr.EdgeScrollY = 100;
  Scr.ScrollResistance = Scr.MoveResistance = 0;
  Scr.OpaqueSize = 5;
  Scr.ClickTime = 150;
  Scr.ColormapFocus = COLORMAP_FOLLOWS_MOUSE;

  /* set major operating modes */
  Scr.NumBoxes = 0;

  Scr.randomx = Scr.randomy = 0;
  Scr.buttons2grab = 7;

  InitFvwmDecor(&Scr.DefaultDecor);
#ifdef USEDECOR
  Scr.DefaultDecor.tag = "Default";
#endif

  Scr.SmartPlacementIsClever = False;
  Scr.ClickToFocusPassesClick = True;
  Scr.ClickToFocusRaises = True;
  Scr.MouseFocusClickRaises = False;

  /* Not the right place for this, should only be called once somewhere .. */
  InitPictureCMap(dpy,Scr.Root);

  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	Reborder - Removes fvwm border windows
 *
 ************************************************************************/
void Reborder(void)
{
  FvwmWindow *tmp;			/* temp fvwm window structure */

  /* put a border back around all windows */
  MyXGrabServer (dpy);

  InstallWindowColormaps (&Scr.FvwmRoot);	/* force reinstall */
  for (tmp = Scr.FvwmRoot.next; tmp != NULL; tmp = tmp->next)
  {
    RestoreWithdrawnLocation (tmp,True); 
    XUnmapWindow(dpy,tmp->frame);
    XDestroyWindow(dpy,tmp->frame);
  }

  MyXUngrabServer (dpy);
  XSetInputFocus (dpy, PointerRoot, RevertToPointerRoot,CurrentTime);
  XSync(dpy,0);

}

/***********************************************************************
 *
 *  Procedure:
 *	Done - cleanup and exit fvwm
 *
 ***********************************************************************
 */
void SigDone(int nonsense)
{
  Done(0, NULL);
  SIGNAL_RETURN;
}

void Done(int restart, char *command)
{
  MenuRoot *mr;

#ifndef NON_VIRTUAL
  MoveViewport(0,0,False);
#endif

  mr = FindPopup("ExitFunction");
  if(mr != NULL)
    ExecuteFunction("Function ExitFunction",NULL,&Event,C_ROOT,-1);

  /* Close all my pipes */
  ClosePipes();

  Reborder ();

  if(restart)
  {
    SaveDesktopState();		/* I wonder why ... */

    /* Really make sure that the connection is closed and cleared! */
    XSelectInput(dpy, Scr.Root, 0 );
    XSync(dpy, 0);
    XCloseDisplay(dpy);

    {
      char *my_argv[10];
      int i,done,j;

      i=0;
      j=0;
      done = 0;
      while((g_argv[j] != NULL)&&(i<8))
      {
        if(strcmp(g_argv[j],"-s")!=0)
        {
          my_argv[i] = g_argv[j];
          i++;
          j++;
        }
        else
          j++;
      }
      if(strstr(command,"fvwm")!= NULL)
        my_argv[i++] = "-s";
      while(i<10)
        my_argv[i++] = NULL;
	
      /* really need to destroy all windows, explicitly,
       * not sleep, but this is adequate for now */
      sleep(1);
      ReapChildren();
      execvp(command,my_argv);
    }
    fvwm_msg(ERR,"Done","Call of '%s' failed!!!! (restarting '%s' instead)",
             command,
             g_argv[0]);
    execvp(g_argv[0], g_argv);    /* that _should_ work */
    fvwm_msg(ERR,"Done","Call of '%s' failed!!!!", g_argv[0]); 
  }
  else
  {
    XCloseDisplay(dpy);
    exit(0);
  }
}

XErrorHandler CatchRedirectError(Display *dpy, XErrorEvent *event)
{
  fvwm_msg(ERR,"CatchRedirectError","another WM is running");
  exit(1);
}

/***********************************************************************
 *
 *  Procedure:
 *	CatchFatal - Shuts down if the server connection is lost
 *
 ************************************************************************/
XIOErrorHandler CatchFatal(Display *dpy)
{
  /* No action is taken because usually this action is caused by someone
     using "xlogout" to be able to switch between multiple window managers
     */
  ClosePipes();
  exit(1);
}

/***********************************************************************
 *
 *  Procedure:
 *	FvwmErrorHandler - displays info on internal errors
 *
 ************************************************************************/
XErrorHandler FvwmErrorHandler(Display *dpy, XErrorEvent *event)
{
  extern int last_event_type;

  /* some errors are acceptable, mostly they're caused by 
   * trying to update a lost  window */
  if((event->error_code == BadWindow)||(event->request_code == X_GetGeometry)||
     (event->error_code==BadDrawable)||(event->request_code==X_SetInputFocus)||
     (event->request_code==X_GrabButton)||
     (event->request_code==X_ChangeWindowAttributes)||
     (event->request_code == X_InstallColormap))
    return 0 ;


  fvwm_msg(ERR,"FvwmErrorHandler","*** internal error ***");
  fvwm_msg(ERR,"FvwmErrorHandler","Request %d, Error %d, EventType: %d",
           event->request_code,
           event->error_code,
           last_event_type);
  return 0;
}

void usage(void)
{
#if 0
  fvwm_msg(INFO,"usage","\nFvwm Version %s Usage:\n\n",VERSION);
  fvwm_msg(INFO,"usage","  %s [-d dpy] [-debug] [-f config_cmd] [-s] [-blackout] [-version] [-h]\n",g_argv[0]);
#else
  fprintf(stderr,"\nFvwm Version %s Usage:\n\n",VERSION);
  fprintf(stderr,"  %s [-d dpy] [-debug] [-f config_cmd] [-s] [-blackout] [-version] [-h]\n\n",g_argv[0]);
#endif
}

/****************************************************************************
 *
 * Save Desktop State
 *
 ****************************************************************************/
void SaveDesktopState()
{
  FvwmWindow *t;
  unsigned long data[1];

  for (t = Scr.FvwmRoot.next; t != NULL; t = t->next)
  {
    data[0] = (unsigned long) t->Desk;
    XChangeProperty (dpy, t->w, _XA_WM_DESKTOP, _XA_WM_DESKTOP, 32,
                     PropModeReplace, (unsigned char *) data, 1);
  }

  data[0] = (unsigned long) Scr.CurrentDesk;
  XChangeProperty (dpy, Scr.Root, _XA_WM_DESKTOP, _XA_WM_DESKTOP, 32,
		   PropModeReplace, (unsigned char *) data, 1);

  XSync(dpy, 0);
}


void SetMWM_INFO(Window window)
{
#ifdef MODALITY_IS_EVIL
  struct mwminfo
  {
    long flags;
    Window win;
  }  motif_wm_info;
  
  /* Set Motif WM_INFO atom to make motif relinquish 
   * broken handling of modal dialogs */
  motif_wm_info.flags     = 2;
  motif_wm_info.win = window;
  
  XChangeProperty(dpy,Scr.Root,_XA_MOTIF_WM,_XA_MOTIF_WM,32,
		  PropModeReplace,(char *)&motif_wm_info,2);
#endif
}

void BlackoutScreen()
{
  XSetWindowAttributes attributes;
  unsigned long valuemask;
  
  if (Blackout && (BlackoutWin == None) && !debugging)
  {
    DBUG("BlackoutScreen","Blacking out screen during init...");
    /* blackout screen */
    attributes.border_pixel = BlackPixel(dpy,Scr.screen);
    attributes.background_pixel = BlackPixel(dpy,Scr.screen);
    attributes.bit_gravity = NorthWestGravity;
    attributes.override_redirect = True; /* is override redirect needed? */
    valuemask = CWBorderPixel |
      CWBackPixel   |
      CWBitGravity  |
      CWOverrideRedirect;
    BlackoutWin = XCreateWindow(dpy,Scr.Root,0,0,
                                DisplayWidth(dpy, Scr.screen),
                                DisplayHeight(dpy, Scr.screen),0,
                                CopyFromParent,
                                CopyFromParent, CopyFromParent,
                                valuemask,&attributes);
    XMapWindow(dpy,BlackoutWin);
    XSync(dpy,0);
  }
} /* BlackoutScreen */

void UnBlackoutScreen()
{
  if (Blackout && (BlackoutWin != None) && !debugging)
  {
    DBUG("UnBlackoutScreen","UnBlacking out screen");
    XDestroyWindow(dpy,BlackoutWin); /* unblacken the screen */
    XSync(dpy,0);
    BlackoutWin = None;
  }
} /* UnBlackoutScreen */
