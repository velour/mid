#include "mid.h"
#include <SDL/SDL.h>

Rect rect = { (Point){ 0, 0 }, (Point){ 10, 10 } };
float dx = 0, dy = 0;

int main(int argc, char *argv[]){
	SDL_Window* window;
        SDL_Renderer* renderer;

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
                return 1;

        window = SDL_CreateWindow("SDL_RenderClear",
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        512, 512,
                        SDL_WINDOW_SHOWN);

        renderer = SDL_CreateRenderer(window, -1, 0);

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

	        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	        SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_Rect r = { rect.a.x, rect.a.y, 10, 10 };
		SDL_RenderDrawRect(renderer, &r);

	        SDL_RenderPresent(renderer);

        	SDL_Delay(50);
	}
        SDL_Quit();
	return 0;
}
