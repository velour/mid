#include "mid.h"
#include <SDL/SDL.h>

int main(int argc, char *argv[]){
	Gfx *gfx = gfxinit(512, 512);
	if(!gfx)
		return 1;

	Rect rect = (Rect){ (Point){ 0, 0 }, (Point){ 10, 10 } };
	float dx = 0, dy = 0;
	Color red = { 255, 0, 0, 255 };

	Img *glenda = imgnew(gfx, "resrc/img/9logo.png");

	for(;;){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym){
				case SDLK_LEFT: dx--; break;
				case SDLK_RIGHT: dx++; break;
				case SDLK_UP: dy--; break;
				case SDLK_DOWN: dy++; break;
				default:
					SDL_Quit();
					return 0;
				}
				break;
			case SDL_KEYUP:
				switch(event.key.keysym.sym){
				case SDLK_LEFT: dx = 0; break;
				case SDLK_RIGHT: dx = 0; break;
				case SDLK_UP: dy = 0; break;
				case SDLK_DOWN: dy = 0; break;
				}
				break;
			}
		}

		rectmv(&rect, dx, dy);

		gfxclear(gfx, red);
		imgdraw(gfx, glenda, rect.a);

		gfxflip(gfx);

		SDL_Delay(50);
	}
	SDL_Quit();
	return 0;
}
