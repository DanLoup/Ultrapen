#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <SDL2/SDL.h>
#include "Gui.h"
#include "MainWindow.h"
SDL_Renderer * rd;
int mouse_x,mouse_y,mouse_bt,mouse_dbl,mouse_edup,mouse_eddn;
int olmouse_bt;
int keyctrl=0,keyshift=0,enterpressed=0,keydir=0;
int keys[256];
int keycombo[256];
int spmask[]={SDLK_UP,SDLK_RIGHT,SDLK_DOWN,SDLK_LEFT,SDLK_DELETE,SDLK_INSERT,SDLK_BACKSPACE,SDLK_ESCAPE,SDLK_LSHIFT,SDLK_RSHIFT,SDLK_LCTRL,SDLK_RCTRL,SDLK_LALT,SDLK_RALT};
int moudbct=0;
bool running=true;
void setWindowsIcon(SDL_Window *sdlWindow);
void RunAudio();

int main( int argc, char * argv[] ){
	SDL_Init(SDL_INIT_VIDEO || SDL_INIT_AUDIO);
	SDL_Window * win = SDL_CreateWindow("Ultrapen Editor", 100, 50, 1024, 768, SDL_WINDOW_SHOWN);
	SDL_Surface * sr=SDL_LoadBMP("upen.bmp");
	SDL_Surface * sur=SDL_ConvertSurfaceFormat(sr,SDL_PIXELFORMAT_RGBA8888,0);
	SDL_LockSurface(sur);
	unsigned char * pax=(unsigned char*)sur->pixels;
	for (int a=0;a<(64*64*4);a+=4){
		if (pax[a+1]<1 && pax[a+2]>254){pax[a]=0;}
	}
	SDL_UnlockSurface(sur);
	SDL_SetWindowIcon(win,sur);
	rd = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Event eev;
	SDL_Texture * font=LoadFont("sfont.bmp");
	fnt =font;
	bootaudio();
	InitalizeMainWindow();

	while (running){
		mouse_dbl=0;
		olmouse_bt=mouse_bt;
		while (SDL_PollEvent(&eev)) {
			if (eev.type == SDL_QUIT) { running = false; }
			if (eev.type == SDL_MOUSEMOTION) {
				mouse_x=eev.motion.x;mouse_y=eev.motion.y;
			}
            if (eev.type == SDL_MOUSEBUTTONDOWN) {
                if (eev.button.button == 1){mouse_bt |= 1;}
                if (eev.button.button == 2) { mouse_bt |= 2; }
                if (eev.button.button == 3) { mouse_bt |= 4; }
				if (moudbct>0){mouse_dbl=1;moudbct=0;}
            }
            if (eev.type == SDL_MOUSEBUTTONUP) {
                if (eev.button.button == 1) { mouse_bt &= 6; }
                if (eev.button.button == 2) { mouse_bt &= 5; }
                if (eev.button.button == 3) { mouse_bt &= 3; }
				moudbct=15;
				mouse_dbl=0;
            }
            if (eev.type == SDL_KEYDOWN){
				int kba=eev.key.keysym.sym;

				if (kba==SDLK_LSHIFT){keyshift=1;}
				if (kba==SDLK_LCTRL){keyctrl=1;}
				if (kba==SDLK_RETURN){enterpressed=1;}
				for (int a=0;a<14;a++){if (spmask[a]==kba){kba=a+128;}}
				keys[kba & 255]=1;keycombo[kba & 255]=15;
				RunKeyboard();
            }
            if (eev.type == SDL_KEYUP){
				int kba=eev.key.keysym.sym;

				if (kba==SDLK_LSHIFT){keyshift=0;}
				if (kba==SDLK_LCTRL){keyctrl=0;}
				if (kba==SDLK_RETURN){enterpressed=0;}
				for (int a=0;a<14;a++){if (spmask[a]==kba){kba=a+128;}}
				keys[kba & 255]=0;keycombo[kba & 255]=0;
				
				RunKeyboard();

            }
		}
		mouse_edup=0;mouse_eddn=0;
		if (mouse_bt==1 && olmouse_bt == 0){mouse_eddn = 1;}
		if (mouse_bt==0 && olmouse_bt == 1){mouse_edup = 1;}
		
		if (mouse_bt==1){
			int breako=0;
		}
		if (moudbct>0){moudbct--;}
		SDL_Rect src,dst;
		SDL_SetRenderDrawColor(rd,192,192,192,255);
		SDL_RenderClear(rd);
		RunWindows();
		RunKeyboard();
		SDL_RenderPresent(rd);
		RunAudio();
		
	}
	SDL_DestroyRenderer(rd);
	SDL_DestroyWindow(win);
    SDL_CloseAudio();
	SDL_Quit();

	return 0;
}

