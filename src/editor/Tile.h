#ifndef TILE_H_INCLUDED
#define TILE_H_INCLUDED
#include "Gui.h"

class bgtile{
public:
	char tmap[64];
	char plines[8][2];
	bgtile();
	void GetBits(char * data);
	void GetPals(char * data);
	void SetBits(char * data);
	void SetPals(char * data);
	void BlitTile(char *fb,int fbwi,ivec2 pos);
};
class sptile{
public:
	char tmap[256];
	char plines[16][3];
	char limix[16];
	sptile();
	void GetBMP(char * data);
	void GetBits(char * data);
	void GetPals(char * data);
	void SetBits(char * data);
	void SetPals(char * data);
	void BlitTile(char *fb,int fbwi,ivec2 pos,int mc);
	void Refreshlimix();
};
struct palette{color col[16];};
extern int selL,selR;


struct fb{
	SDL_Renderer * ren;
	SDL_Texture * bf;
	ivec2 size=ivec2(0,0);
	std::vector <unsigned char> buff;
	void draw(ivec2 pos,float scale);
	void drawclipped(ivec2 pos,float scale, ivec2 cut);
	void Init(SDL_Renderer * iren,ivec2 isize);
	void update();
	void clear(color col);
};

struct tile{
	char pix[256];ivec2 tilesiz=ivec2(8,8);int tiletype=0;
	void draw(fb * framebuffer,ivec2 pos);
	void drawlc(char * pic,int wi, ivec2 pos);
	void fill(fb * framebuffer,ivec2 pos,color cl);
};


struct bgtileunit{
	char pix[64];
	void draw(char *fb,int fbwi,ivec2 pos);
	void fill(char *fb,int fbwi,ivec2 pos,color cl);
};
struct bgindexunit{int box[4];};

struct objtileunit{
	char pix[256];
	void draw(char *fb,int fbwi,ivec2 pos);
	void fill(char *fb,int fbwi,ivec2 pos,color cl);
};

struct corridorunit{
	char cbls[64];

};



#endif // TILE_H_INCLUDED


