! $XFree86: xc/programs/xdm/config/xdm-conf.cpp,v 1.1.1.2.4.2 1999/10/12 18:33:29 hohndel Exp $
!
! $XConsortium: xdm-conf.cpp /main/3 1996/01/15 15:17:26 gildea $
DisplayManager.errorLogFile:	/var/log/xdm-errors
DisplayManager.pidFile:		/var/run/xdm-pid
DisplayManager.keyFile:		XDMDIR/xdm-keys
DisplayManager.servers:		XDMDIR/Xservers
DisplayManager.accessFile:	XDMDIR/Xaccess
! All displays should use authorization, but we cannot be sure
! X terminals will be configured that way, so by default
! use authorization only for local displays :0, :1, etc.
DisplayManager._0.authorize:	true
DisplayManager._1.authorize:	true
! The following three resources set up display :0 as the console.
! DisplayManager.*.setup:	XDMDIR/Xsetup
DisplayManager._0.setup:	XDMDIR/Xsetup
DisplayManager._0.startup:	XDMDIR/Startup_0
DisplayManager._0.reset:	XDMDIR/TakeConsole
!
DisplayManager*resources:	XDMDIR/Xresources
DisplayManager*session:		XDMDIR/Xsession
DisplayManager*authComplain:	false
#if HAS_DES_AUTH == YES
DisplayManager._0.authName:     MIT-MAGIC-COOKIE-1
#endif
#ifdef XPM
! this is a new line Caolan, 9312811@ul.ie
DisplayManager*loginmoveInterval:	10
#endif /* XPM */
! SECURITY: do not listen for XDMCP or Chooser requests
! Comment out this line if you want to manage X terminals with xdm
DisplayManager.requestPort:	0
