# $XFree86: xc/programs/Xserver/hw/xfree86/XF86Setup/texts/generic/messages.tcl,v 1.1.2.4 1998/02/26 20:11:20 hohndel Exp $
#
# messages in done.tcl :
set messages(done.1)	"\n\n\
		If you've finished configuring everything press the\n\
		Okay button to start the X server using the\
		configuration you've selected.\n\n\
		If you still wish to configure some things,\n\
		press one of the buttons at the top and then\n\
		press \"Done\" again, when you've finished."
set messages(done.2) "Okay."

# messages in srvflags :
set messages(srvflags.1) "Optional server settings\n\n\
		These should be set to reasonable values, by default,\n\
		so you probably don't need to change anything"
set messages(srvflags.2) "Allow server to be killed with\
		hotkey sequence (Ctrl-Alt-Backspace)"
set messages(srvflags.3) "Allow video mode switching"
set messages(srvflags.4) "Don't Trap Signals\
			- prevents the server from exitting cleanly"
set messages(srvflags.5) "Allow video mode changes from other hosts"
set messages(srvflags.6) "Allow changes to keyboard and mouse settings\
		from other hosts"

# messages in phase2 :
set messages(phase2.1) "Loading  -  Please wait...\n\n\n"
set messages(phase2.2) \
	    "Unable to read keyboard information from the server.\n\n\
	    This problem most often occurs when you are running when\n\
	    you are running a server which does not have the XKEYBOARD\n\
	    extension or which has it disabled.\n\n\
	    The ability of this program to configure the keyboard is\n\
	    reduced without the XKEYBOARD extension, but is still\
	    functional.\n\n\
	    Continuing..."
set messages(phase2.3) Mouse
set messages(phase2.4) Keyboard
set messages(phase2.5) Card
set messages(phase2.6) Monitor
set messages(phase2.7) Modeselection
set messages(phase2.8) Other
set messages(phase2.9) Abort
set messages(phase2.10) Done
set messages(phase2.11) Help
set messages(phase2.12) "\n\
		There are five areas of configuration that need to\
			be completed, corresponding to the buttons\n\
		along the top:\n\n\
		\tMouse\t\t- Use this to set the protocol, baud rate, etc.\
			used by your mouse\n\
		\tKeyboard\t- Set the nationality and layout of\
			your keyboard\n\
		\tCard\t\t- Used to select the chipset, RAMDAC, etc.\
			of your card\n\
		\tMonitor\t\t- Use this to enter your\
			monitor's capabilities\n\
		\tModeselction\t\t- Use this to chose the modes\
			that you want to use\n\
		\tOther\t\t- Configure some miscellaneous settings\n\n\
		You'll probably want to start with configuring your\
			mouse (you can just press \[Enter\] to do so)\n\
		and when you've finished configuring all five of these,\
			select the Done button.\n\n\
		To select any of the buttons, press the underlined\
			letter together with either Control or Alt.\n\
		You can also press ? or click on the Help button at\
			any time for additional instructions\n\n"
set messages(phase2.13) "Dismiss"

# messages in phase4 :
set messages(phase4.1) "Loading  -  Please wait..."
# phase.2-5 is generated by proc 'make_message_phase4'.
set messages(phase4.6) "Okay"
set messages(phase4.7) \
	"You can now run xvidtune to adjust your display settings,\n\
	if you want to change the size or placement of the screen image\n\n\
	If not, go ahead and exit\n\n\n\
	If you choose to save the configuration, a backup copy will be\n\
	made, if the file already exists"
set messages(phase4.8) "Save configuration to:"
set messages(phase4.9) "Run xvidtune"
set messages(phase4.10) "Save the configuration and exit"
set messages(phase4.11) "Abort - Don't save the configuration"
set messages(phase4.12) "Aborted"
set messages(phase4.13) "Congratulations, you've got a running server!\n\n"

# messages in card.tcl :
set messages(card.1) "Card selected:"
set messages(card.2) "Card selected: None"
set messages(card.3) "Read README file"
set messages(card.4) "Detailed Setup"
set messages(card.5) Server:
set messages(card.7) Chipset
set messages(card.8) RamDac
set messages(card.9) ClockChip
set messages(card.10) "RAMDAC Max Speed"
set messages(card.11) "Probed"
set messages(card.12) "Video RAM"
set messages(card.13) "Probed"
set messages(card.14) "256K"
set messages(card.15) "512K"
set messages(card.16) "1Meg"
set messages(card.17) "2Meg"
set messages(card.18) "3Meg"
set messages(card.19) "4Meg"
set messages(card.20) "6Meg"
set messages(card.21) "8Meg"
set messages(card.22) "Selected options:"
set messages(card.23) "Additional lines to\
	add to Device section of the XF86Config file:"
#set messages(card.24) "Probed: Yes"
#set messages(card.25) "Probed: No"
set messages(card.26) "Card List"
set messages(card.27) "Detailed Setup"
set messages(card.28) "Card selected: "
set messages(card.29) "Dismiss"
set messages(card.30) \
	"First make sure the correct server is selected,\
	then make whatever changes are needed\n\
	If the Chipset, RamDac, or ClockChip entries\
	are left blank, they will be probed"
set messages(card.31) \
	"Select your card from the list.\n\
	If your card is not listed,\
	click on the Detailed Setup button"
set messages(card.32) \
	"That's all there is to configuring your card\n\
	unless you would like to make changes to the\
	standard settings (by pressing Detailed Setup)"
set messages(card.33) \
	"That's probably all there is to configuring\
	your card, but you should probably check the\n\
	README to make sure. If any changes are needed,\
	press the Detailed Setup button"
set messages(card.34) \
	"You have selected a card which is not fully\
	supported by XFree86, however all of the proper\n\
	configuration options have been set such that it\
	should work in standard VGA mode"

# messages in keyboard.tcl :
set messages(keyboard.1) "Model:"
set messages(keyboard.2) "Layout (language):"
set messages(keyboard.3) "Apply"
set messages(keyboard.4) \
		"Select the appropriate model and layout"
set messages(keyboard.5) "Available options:"
set messages(keyboard.6) \
		"Variant (non U.S. Keyboards only):"
set messages(keyboard.7) "Use default setting"
set messages(keyboard.8) "Failed! Try again"
set messages(keyboard.9) "Applying..."
set messages(keyboard.10) "Please wait..."

# messages in modeselect.tcl :

set messages(modeselect.1) "Select the modes you want to use"
set messages(modeselect.2) 640x480
set messages(modeselect.3) 800x600
set messages(modeselect.4) 1024x768
set messages(modeselect.5) 1152x864
set messages(modeselect.6) 1280x1024
set messages(modeselect.7) 1600x1200
set messages(modeselect.8) 640x400
set messages(modeselect.9) 320x200
set messages(modeselect.10) 320x240
set messages(modeselect.11) 400x300
set messages(modeselect.12) 480x300
set messages(modeselect.13) 512x384
set messages(modeselect.14) "Select the default color depth you want to use"
set messages(modeselect.15) " 8bpp "
set messages(modeselect.16) " 16bpp "
set messages(modeselect.17) " 24bpp "
set messages(modeselect.18) " 32bpp "

# messages in monitor.tcl :

set messages(monitor.1) "Monitor sync rates"
set messages(monitor.2) "Monitor selected:"
set messages(monitor.3) "Horizontal"
set messages(monitor.4) "Vertical"
set messages(monitor.5) \
	"Enter the Horizontal and Vertical Sync ranges for your monitor\n\
	or if you do not have that information, choose from the list"

# messages in mouse.tcl :

set messages(mouse.1) "Lines/inch"
set messages(mouse.2) "Sample Rate"
set messages(mouse.3) "Select the mouse protocol"
set messages(mouse.4) "Applying..."
set messages(mouse.5) "Mouse Device"
set messages(mouse.6) Emulate3Buttons
set messages(mouse.7) ChordMiddle
set messages(mouse.8) "Baud Rate"
set messages(mouse.9) "Press ? or Alt-H for a list of key bindings"
set messages(mouse.10) Flags
set messages(mouse.11) ClearDTR
set messages(mouse.12) ClearRTS
set messages(mouse.13) "Sample Rate"
set messages(mouse.14) "Emulate3Timeout"
set messages(mouse.15) "Apply"
set messages(mouse.16) "Press ? or Alt-H for a list of key bindings"
set messages(mouse.17) "Exit"
set messages(mouse.18) 1
set messages(mouse.19) "Resolution"
set messages(mouse.20) "High"
set messages(mouse.21) "Medium"
set messages(mouse.22) "Low"
set messages(mouse.23) "Buttons"