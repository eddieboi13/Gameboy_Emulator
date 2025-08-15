#include <stdio.h>
#include <SDL2/SDL.h>
#include "joypad.h"
#include "flags.h"
JoypadState joypad = {true, true, true, true, true, true, true, true};
void update_joyp(SDL_Event *event, bool is_down){
		switch (event->key.keysym.scancode){
			case SDL_SCANCODE_W:     joypad.up     = !is_down; break;
        	case SDL_SCANCODE_S:     joypad.down   = !is_down; break;
        	case SDL_SCANCODE_A:     joypad.left   = !is_down; break;
        	case SDL_SCANCODE_D:     joypad.right  = !is_down; break;
        	case SDL_SCANCODE_K:     joypad.a      = !is_down; break;
        	case SDL_SCANCODE_L:     joypad.b      = !is_down; break;
			case SDL_SCANCODE_SPACE: joypad.select = !is_down; break;
        	case SDL_SCANCODE_RETURN:joypad.start  = !is_down; break;
        	default: break;
		}
}


