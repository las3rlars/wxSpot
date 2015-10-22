# wxSpot
wxSpot - a native ui frontend for Spotify

![alt tag](http://vps188527.ovh.net/~viktor/wxSpot/wxSpot%20screenshot%202.png)

## Running

Unzip the zip file, double click wxSpot.exe and you will be prompted for your credentials.
You will also be prompted to enter a spotify cache directory. This is where the client will store cache
of the music you are playing. If you are using a SSD + Mechanical harddrive - this allows you to put
the cache on your mechanical disk.

If your keyboard supports media keys - play/pause, next, prev should now work globaly

### Visualizations

wxSpot has projectM visualizations baked in. So you can enjoy Milkdrop just like in the good old winamp days

<kbd>alt</kbd> + <kbd>enter</kbd> to go fullscreen
<kbd>space</kbd> for next track


### FAQ

* Q: Why is there no volume control? A: PortAudio does not support native volume control at this moment.
I don't want to reduce the volume of the samples as that might hurt quality



## Compiling

I think compiling has become quite hard with the projectM requirement. If you want help getting this to compile
contact me.

AppKey.h is not included - it only contains the app key from spotify. I'm not really sure if it's ok to put
it in public or not


The content of AppKey.h is like:

        #pragma once
        
        #include <cstdint>
        
        const uint8_t g_appkey[] = {
        	0x01, 0x29, 0x6F, 0xD0, 0x20, 0x0A, 0x84, 0x9E, 0x99, 0x0A, 0x2A, 0x56, 0xFD, 0x85, 0x9E, 0x3A,
        	0x38, 0x62, 0xF4, 0xF6, ......
	
You need to set the library and include paths to libspotify, wxwidgets, projectM and libspotify
Check their documentation of how to compile.
