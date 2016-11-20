budgie-indicator-applet
-----------------------

AppIndicator applet for budgie-desktop.

*Help required to resolve the TODO and ENHANCEMENTS lists*


To compile:

    sudo apt install git libtool dpkg-dev, intltool, libtool, libgtk-3-dev, libido3-0.1-dev, libindicator3-dev, libpeas-dev, budgie-core-dev
    
    
    git clone https://github.com/budgie-remix/budgie-indicator-applet
    cd budgie-indicator-applet
    ./autogen.sh --prefix=/usr
    make
    sudo make install

To run: install the recommended packages

    sudo apt install gir1.2-appindicator3-0.1
    
    budgie-panel --replace &
    
    Use Raven to add the applet to the panel.
    

TODO
-----

 - [x] Applet background needs to use panel colour for inbuilt-theme 
 - [x] Code cleanup - copyright statements, unused code, change boilerplate budgie-applet
 - [x] Correct debian/copyright
 - [x] check all the build dependencies are actually needed

Enhancements
-----

 - [ ] For the applet settings add capability to change indicator order
 - [x] Appindicator spacing is too wide when not using built-in-theme
 - [x] Applet background needs to respect raven stylise regions option
 - [x] Applet background needs to use panel colour for user-defined theme
 - [ ] When used with the system-tray applet hide the network applet icon rather than permanently hiding
 - [ ] Change from using GtkMenu and GtkMenuItem to GtkButtonBox/GtkButton and therefore allow GtkPopover when button click
 
 *Tips for Development*
 
 Use the following to run GTK Inspector - use to investigate CSS and other properties
 
     GTK_DEBUG=interactive budgie-panel --replace
     
 Use the following to print out g_debug messages i.e. use "zzz" in the g_debug to show in the grep filter
 
     G_MESSAGES_DEBUG=all budgie-panel --replace | grep "zzz"
