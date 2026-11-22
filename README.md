xlibbar, a simple xlib top bar for NetBSD  

A lightweight, minimalist top status bar built specifically for NetBSD, 
designed to run with minimal dependencies and ultra-low resource usage.

Features

Left Side:
   Simple digital clock.
	
Right Side (ASCII/Keyboard character indicators):
   Network Status: Displays -C- when connected and -X- when disconnected.
   Audio Level: Displays <) accompanied by characters that scale 
   dynamically according to the current volume level.

Dependencies
   Xlib
   aiomixer
   netstat

Installation
To build and install xlibbar, simply run:
   make
   make install

Autostart (XDM)
After installing, add the binary to your X session startup file 
so it launches automatically 

when logging in:
Open /etc/X11/xdm/Xsession with your preferred text editor (as root).
Add the following line before "fi"):
   /usr/bin/xlibbar &

Support & Donations
If you find this project useful and would like to support the developer,contributions are greatly appreciated:
   PayPal: anludesousa@gmail.com
   PIX (Brasil only): chavepixanlu@gmail.com

This software was developed by Anlu Rodrigues de Sousa (she/her), 
a proud Transfem!
