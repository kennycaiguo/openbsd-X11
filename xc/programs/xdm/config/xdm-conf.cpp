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
DisplayManager._0.startup:	XDMDIR/GiveConsole
DisplayManager._0.reset:	XDMDIR/TakeConsole
!
DisplayManager*resources:	XDMDIR/Xresources
DisplayManager*session:		XDMDIR/Xsession
DisplayManager*authComplain:	false
#ifdef XDM
! this is a new line Caolan, 9312811@ul.ie
DisplayManager*loginmoveInterval:	10
#endif /* XDM */
