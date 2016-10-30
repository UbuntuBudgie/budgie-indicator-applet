budgie-indicator-applet
-----------------------

AppIndicator applet for budgie-desktop.

Note: It is intended that the applet replaces the system-tray - not to run both side-by-side

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

 - [ ] Applet background needs to use panel colour 
 - [ ] Code cleanup - copyright statements, unused code, change boilerplate budgie-applet
 - [ ] Correct debian/copyright
 - [ ] check all the build dependencies are actually needed

Enhancements
-----

 - [ ] For the applet settings add capability to change indicator order
 - [ ] Appindicator spacing is too wide when not using built-in-theme
 - [ ] Applet background needs to respect raven stylise regions option
