/* © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.*/

#include "../../include/mid.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>

_Bool mute;

enum { NCHAN = 64 };

bool sndinit(void)
{
	if(mute)
		return true;

	if (SDL_WasInit(0) == 0) {
		if (SDL_Init(SDL_INIT_AUDIO) < 0)
			return false;
	} else {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
			return false;
	}

	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
		return false;
	Mix_AllocateChannels(NCHAN);

	sndvol(SndVolDefault);

	return true;
}

void sndfree(void)
{
	if(mute)
		return;

	Mix_CloseAudio();
}

int sndvol(int v)
{
	if(mute)
		return 0;

	Mix_VolumeMusic(v);
	return Mix_Volume(-1, v);
}

_Bool sndread(char *fname){
	FILE *f = fopen(fname, "r");
	if(!f)
		return 1;

	int v = 0;
	if (fscanf(f, "%d", &v) != 1)
		return false;
	if(ferror(f))
		return fclose(f);

	sndvol(v);
	return 0;
}

_Bool sndwrite(char *fname){
	FILE *f = fopen(fname, "w");
	if(!f)
		return 1;

	fprintf(f, "%d\n", sndvol(-1));
	return fclose(f);
}

struct Music {
	Mix_Music *m;
};

Music *musicnew(const char *path)
{
	Music *m = xalloc(1, sizeof(*m));

	if(mute)
		return m;

	m->m = Mix_LoadMUS(path);
	if (!m->m) {
		xfree(m);
		return NULL;
	}

	return m;
}

void musicfree(Music *m)
{
	if(!mute)
		Mix_FreeMusic(m->m);
	xfree(m);
}

void musicstart(Music *m, int fadein)
{
	if(mute)
		return;

	Mix_FadeInMusic(m->m, 1, fadein);
}

void musicstop(int fadeout)
{
	if(mute)
		return;

	Mix_FadeOutMusic(fadeout);
}

void musicpause(void)
{
	if(mute)
		return;

	Mix_PauseMusic();
}

void musicresume(void)
{
	if(mute)
		return;

	Mix_ResumeMusic();
}

struct Sfx {
	Mix_Chunk *c;
};

Sfx *sfxnew(const char *path)
{
	Sfx *s = xalloc(1, sizeof(*s));

	if(mute)
		return s;

	s->c = Mix_LoadWAV(path);
	if (!s->c) {
		xfree(s);
		return NULL;
	}

	return s;
}

void sfxfree(Sfx *s)
{
	if(!mute)
		Mix_FreeChunk(s->c);
	xfree(s);
}

void sfxplay(Sfx *s)
{
	if(mute)
		return;

	if (Mix_Playing(-1) < NCHAN)
		Mix_PlayChannel(-1, s->c, 0);
}
