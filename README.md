# wxSpot
wxSpot - a native ui frontend for Spotify

AppKey.h is not included - it only contains the app key from spotify. I'm not really sure if it's ok to put
it in public or not


The content of AppKey.h is like:

        #pragma once
        
        #include <cstdint>
        
        const uint8_t g_appkey[] = {
        	0x01, 0x29, 0x6F, 0xD0, 0x20, 0x0A, 0x84, 0x9E, 0x99, 0x0A, 0x2A, 0x56, 0xFD, 0x85, 0x9E, 0x3A,
        	0x38, 0x62, 0xF4, 0xF6, ......
	
You need to set the library and include paths to libspotify, wxwidgets and libspotify
Check their documentation of how to compile.
