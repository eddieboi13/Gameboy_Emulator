#include <stdio.h>
#include <stdbool.h>
typedef struct{
	bool start, select, b, a, down, up, left, right;
}JoypadState;
void update_joyp(SDL_Event *event, bool is_down);
