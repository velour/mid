#include <SDL/SDL.h>

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

	int x = 0, y = 0;

	for(;;){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym){
				case SDLK_LEFT: x--; break;
				case SDLK_RIGHT: x++; break;
				case SDLK_UP: y--; break;
				case SDLK_DOWN: y++; break;
				default:
					SDL_Quit();
					return 0;
				}
			}
		}

	        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	
	        SDL_RenderClear(renderer);
	
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_Rect r = {x, y, 10, 10};
		SDL_RenderDrawRect(renderer, &r);
	
	        SDL_RenderPresent(renderer);

        	SDL_Delay(50);
	}
        SDL_Quit();
	return 0;
}