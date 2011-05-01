#include <SDL/SDL.h>

const int w = 512;
const int h = 512;

int main(int argc, char *argv[]){
	SDL_Window* window;
        SDL_Renderer* renderer;

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
                return 1;

        window = SDL_CreateWindow("SDL_RenderClear",
				  SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  w, h,
				  SDL_WINDOW_SHOWN);

        renderer = SDL_CreateRenderer(window, -1, 0);

	for(;;){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_KEYDOWN:
				SDL_Quit();
				return 0;
			}
		}

	        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	        SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_Rect r = {0, 0, 10, 10};
		SDL_RenderDrawRect(renderer, &r);

	        SDL_RenderPresent(renderer);

        	SDL_Delay(50);
	}
        SDL_Quit();
	return 0;
}
