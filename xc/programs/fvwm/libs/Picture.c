/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/


/****************************************************************************
 *
 * Routines to handle initialization, loading, and removing of xpm's or mono-
 * icon images.
 *
 ****************************************************************************/

#include "../configure.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/keysym.h>
#include <sys/types.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef XPM
#include <X11/xpm.h>
#endif

#include "fvwmlib.h"


static Picture *PictureList=NULL;
static Colormap PictureCMap;


void InitPictureCMap(Display *dpy,Window Root)
{
  XWindowAttributes root_attr;
  XGetWindowAttributes(dpy,Root,&root_attr);
  PictureCMap=root_attr.colormap;
}


Picture *LoadPicture(Display *dpy,Window Root,char *path)
{
  int i,l;
  Picture *p;
#ifdef XPM
  XpmAttributes xpm_attributes;
#endif

  p=(Picture*)safemalloc(sizeof(Picture));
  p->count=1;
  p->name=path;
  p->next=NULL;

#ifdef XPM
  /* Try to load it as an X Pixmap first */
  xpm_attributes.colormap=PictureCMap;
  xpm_attributes.closeness=40000; /* Allow for "similar" colors */
  xpm_attributes.valuemask=
    XpmSize | XpmReturnPixels | XpmColormap | XpmCloseness;
  
  if(XpmReadFileToPixmap(dpy,Root,path,&p->picture,&p->mask,&xpm_attributes)
     == XpmSuccess) 
    { 
      p->width = xpm_attributes.width;
      p->height = xpm_attributes.height;
      p->depth = DefaultDepthOfScreen(DefaultScreenOfDisplay(dpy));
      return p;
    }
#endif

  /* If no XPM support, or XPM loading failed, try bitmap */
  if(XReadBitmapFile(dpy,Root,path,&p->width,&p->height,&p->picture,&l,&l)
     == BitmapSuccess)
    {
      p->depth = 0;
      p->mask = None;
      return p;
    }

  free(path);
  free(p);
  return NULL;
}


Picture *GetPicture(Display *dpy,Window Root,char *IconPath,char *PixmapPath,
		    char *name)
{
  char *path;
  if(!(path=findIconFile(name,PixmapPath,R_OK)))
    if(!(path=findIconFile(name,IconPath,R_OK)))
      return NULL;
  return LoadPicture(dpy,Root,path);
}


Picture *CachePicture(Display *dpy,Window Root,char *IconPath,char *PixmapPath,
		    char *name)
{
  char *path;
  Picture *p=PictureList;
  int i,l;

  /* First find the full pathname */
  if(!(path=findIconFile(name,PixmapPath,R_OK)))
    if(!(path=findIconFile(name,IconPath,R_OK)))
      return NULL;

  l=strlen(path);

  /* See if the picture is already cached */
  while(p)
    {
      i=l; /* Check for matching name; backwards compare will probably find
	      differences fastest, but is a little 'unclean' (Doesn't check
	      length of pl->name, compares beyond end, should do no harm... */

      while(i>=0 && path[i]==p->name[i])
	i--;
      if(i<0) /* We have found a picture with the wanted name */
	{
	  p->count++; /* Put another weight on the picture */
	  return p;
	}
      p=p->next;
    }

  /* Not previously cached, have to load it ourself. Put it first in list */
  p=LoadPicture(dpy,Root,path);
  if(p)
    {
      p->next=PictureList;
      PictureList=p;
    }
  return p;
}


void DestroyPicture(Display *dpy,Picture *p)
{
  Picture *q=PictureList;

  if (!p) /* bag out if NULL */
    return;
  if(--(p->count)>0) /* Remove a weight, still too heavy? */
    return;

  /* Let it fly */
  if(p->name!=NULL)
    free(p->name);
  if(p->picture!=None)
    XFreePixmap(dpy,p->picture);
  if(p->mask!=None)
    XFreePixmap(dpy,p->mask);

  /* Link it out of the list (it might not be there) */
  if(p==q) /* in head? simple */
    PictureList=p->next;
  else
    {
      while(q && q->next!=p) /* fast forward until end or found */
	q=q->next;
      if(q) /* not end? means we found it in there, possibly at end */
	q->next=p->next; /* link around it */
    }
  free(p);
}

/****************************************************************************
 *
 * Find the specified icon file somewhere along the given path.
 *
 * There is a possible race condition here:  We check the file and later
 * do something with it.  By then, the file might not be accessible.
 * Oh well.
 *
 ****************************************************************************/
char *findIconFile(char *icon, char *pathlist, int type)
{
  char *path;
  char *dir_end;
  int l1,l2;

  if(icon != NULL)
    l1 = strlen(icon);
  else 
    l1 = 0;

  if(pathlist != NULL)
    l2 = strlen(pathlist);
  else
    l2 = 0;

  path = safemalloc(l1 + l2 + 10);
  *path = '\0';
  if (*icon == '/') 
    {
      /* No search if icon begins with a slash */
      strcpy(path, icon);
      return path;
    }
     
  if ((pathlist == NULL) || (*pathlist == '\0')) 
    {
      /* No search if pathlist is empty */
      strcpy(path, icon);
      return path;
    }
 
  /* Search each element of the pathlist for the icon file */
  while ((pathlist)&&(*pathlist))
    { 
      dir_end = strchr(pathlist, ':');
      if (dir_end != NULL)
	{
	  strncpy(path, pathlist, dir_end - pathlist);
	  path[dir_end - pathlist] = 0;
	}
      else 
	strcpy(path, pathlist);

      strcat(path, "/");
      strcat(path, icon);
      if (access(path, type) == 0) 
	return path;
      strcat(path, ".gz");
      if (access(path, type) == 0) 
	return path;

      /* Point to next element of the path */
      if(dir_end == NULL)
	pathlist = NULL;
      else
	pathlist = dir_end + 1;
    }
  /* Hmm, couldn't find the file.  Return NULL */
  free(path);
  return NULL;
}
 

