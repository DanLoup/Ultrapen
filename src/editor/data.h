#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED
#include "Tile.h"


#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include<iostream>

struct linkf{
	int dir,sscreen,dscreen,dcourse;
};
struct object{
	std::string name; //As it's one per file, it's not even worth trying to index it by number
	int type,subtype,x,y,p1,p2,p3,p4;
	int file, fpos;
	int intern = 0;
};
struct Coursehd{
	int numscreens;
	int exitside;
	unsigned char * course;
	int linkct;
	linkf lks[16];
	int objct;
	object obj[100];
};

struct UObject{
	char name[16];
	int type,subtype,siz,multic,framect;
	sptile sptiles[64];
	int frames[64][4];
};

struct FileType{
    std::string name;
    std::string fpath;
	bool changed;
	bool named;
	enum ftt{FT_LEVEL,FT_OBJECTSET,FT_TILESET,FT_MUSICSET,FT_INSTRUMENTSET,FT_PROGRAMSET};
	ftt typ;
	virtual int Encode(unsigned char * data);
	virtual void Decode(unsigned char * data);
	virtual void Boot();
	virtual void Run(int status);
};

struct DH{
	int pos=0;
	unsigned char * data;
	void wshort(unsigned short val){data[pos]=val & 255;data[pos+1]=(val/256) & 255;pos+=2;}
	unsigned short rshort(){pos+=2;return data[pos-2]+(data[pos-1]*256);}
	void wbyte(unsigned char val){data[pos++]=val;}
	unsigned char rbyte(){return data[pos++];}
};

struct BGtileF : FileType{
	tile tiles[512];
	bgindexunit groups[512];
	bgindexunit clusters[512];
	int bltype[512]; //Tied to groups
	int tict,grct,clct,breath=0;
	int Encode(unsigned char * data);
	void Decode(unsigned char * data);
	BGtileF(){for (int a = 0;a < 512;a++){tiles[a].tilesiz = ivec2(8,8);}}
	void Boot();
	void Run(int status);
	void HandlePaint();
	void HandleTiles();
	void HandleBlocks();
	void HandleClusters();
	void RepaintTextures();
	Window * tiwin;
	Control * penmode[4];

};

struct objst{
	int type,subtype,siz,framect,multic,tilct;
	tile objtls[512];
	int frames[64][4];
	char name[64];
	objst(){for (int a = 0;a < 512;a++){objtls[a].tilesiz=ivec2(16,16);objtls[a].tiletype=1;}}
};

struct OBtileF : FileType{
	objst objs[512];
	int objct;
	int Encode(unsigned char * data);
	void Decode(unsigned char * data);
	OBtileF() {
		objct = 0;
		for (int a = 0; a < 511; a++) {
			objs[a].framect = 0; objs[a].multic = 0; objs[a].tilct = 0; objs[a].type = 0; objs[a].subtype = 0; objs[a].siz = 0; 
			memset(objs[a].frames, 0, 256); sprintf(objs[a].name, "Obj%i", a);
		}
	}
	void Boot();
	void Run(int status);

	Control *lsobj,*fwobj,*obtype,* namebox,*obsize,*multicolor,*framesc,*tilesc,*objmenu,*numfra,*penmodo[5],*delobj;
	Window * obwin;
	SDL_Texture * obuffers[3];
	char tltex[262144],astex[262144],fratex[262144];
	fb ofbs[3];
	void HandleObjDraw();
	void HandleFrames();
	void RefreshBuffs();
	void LoadObjControls();
	void SaveObjControls();
	bool boot = true;
};

struct corridor{
	std::vector<int> blocks;
	std::vector<int> blfiles;
	std::vector<object> objects;
	std::vector<int> objf;
	linkf lks[16];
	int exitside;
	int linkct;
};
struct fileindex{
	std::string flname;
	int id;
};
extern char stpic[10000];

struct prgtpl{
	char pix[32000];
	int wid,hei;
	int trueid;
	std::string name;
	prgtpl(){memcpy(pix,stpic,16*16*4); wid = 16;hei = 16;}
	void blit(char * buff,int bwid,ivec2 pos){
		for (int y = 0; y < hei; y++){
			for (int x = 0; x < wid; x++){
				int addr = (x+pos.x + ((y +pos.y) *bwid)) * 4;
				buff[addr + 0] = pix[((x + (y * wid)) * 4) + 0];buff[addr + 1] = pix[((x + (y * wid)) * 4) + 1];
				buff[addr + 2] = pix[((x + (y * wid)) * 4) + 2];buff[addr + 3] = pix[((x + (y * wid)) * 4) + 3];
			}
		}
	}
};


struct ProgramFile : FileType {
	std::vector <std::string> prg;
	std::string nameclip;
	int Encode(unsigned char* data);
	void Decode(unsigned char* data);
	void Boot();
	void Run(int status);
	Window * aswin;

};

struct note{
	int pitch,volume,instrument,size;
	int cmd;
};

struct song{
	std::vector<note> notes;
	int bpm;
};
struct MusicSet : FileType{
	std::vector <song> songs;
	int Encode(unsigned char* data);
	void Decode(unsigned char* data);
	void Boot();
	void Run(int status);
	Window * aswin;
	int ms;
};

struct instrument{
	std::string name;
	int chip; //0 PSG, 1 SCC, 2 OPLL
	int instype; //0 square, 2 noise, 3 bass hack
	int adsr_sus; //Attack, decay, sustain, release (sustain is a volume)
	int adsr_siz[3];
	std::vector <int> arpt;
	int effect,effpar[3]; //I decided to use fixed programmed effects instead
	int fl_repeat; //Means it will keep looping the adsr
	int data[32]; //Used by SCC and possibly OPLL 
};
struct InstrumentSet : FileType{
	std::vector <instrument> insts;
	int Encode(unsigned char* data);
	void Decode(unsigned char* data);
	void Boot();
	void Run(int status);
	void Update();
	Window * aswin;
	int curins=0;
	int ms=-1;
};

struct mapel{
	std::string name;
	int itile,etile;
	int igro,egro;
	int iclu,eclu;
};
struct o_macro{
	std::string name;
	int id;
	o_macro(std::string namei,int idi){
		name=namei;
		for (int a = 0 ;a < namei.size();a++){
			int nu = namei[a];if (nu > 96 && nu < 123){nu = nu & 223;}name[a]=nu;
		}
		id=idi;
	}
};

struct LVFile : FileType {
	corridor cds[32];
	std::vector <FileType*> files;
	std::vector <fileindex> fitable;
	std::vector<int> fmapt;
	std::vector<int> fmapf;
	std::vector<prgtpl> programs;
	std::vector <o_macro> macros;
	std::vector<mapel> blomap;
	std::vector<tile> tiles;
	std::vector<bgindexunit> groups;
	std::vector<bgindexunit> clusters;
	std::vector<int> bltypes; //group types
	std::vector<objst> objgfx; //Those are not directly used by the level, so stack in files too
	std::vector<ProgramFile*> prf;
	bool outdated;
	int Encode(unsigned char* data);
	void Decode(unsigned char* data);
	void Boot();
	void Run(int status);
	void remap(std::vector<mapel> newblo);
	void FillIcon(prgtpl * pr);
	void ReloadFiles();
	Window * cowin;
	Window * subblo,*subobj,*subfile;
	Control * coboxes[4],* lscour,*fwcour,*courlab,* glabels[8],* gtexbox[4];
};

extern char flexmenu[100];
extern int flexmenuout;
extern bgtile bgtiles[256];
extern int blmaps[256][4],clmaps[256][4];
extern Coursehd Courses[32];
extern palette pals[16];
extern int blpar[256];
extern UObject uobs[128];

extern std::vector <FileType*> fils;
extern int curfile;


void DecodeLevel(unsigned char * data,int old);
int Encodelevel(unsigned char * data,int old);
bool stcmp(char * st1,char * st2,int siz);

extern int bmfilemode,bmwi,bmhe,bmbytes;
extern unsigned char bitmem[4000000];
extern bool bmdone;

bool LoadFile(char* file, char* filemini,bool nolevel);
bool LoadFile(const char* file,const char* filemini,bool nolevel);

extern FileType* fileoutld;
extern OBtileF * basegfx;

void ImportOldDataFile();
std::string ucaser(std::string in);

struct funccal{
	std::string name,desc,uname;
	funccal(std::string nam,std::string des){
		name=nam;desc=des; uname=ucaser(nam);
	}
};
extern std::vector<funccal> fcalls;
void fllldescs();



#endif

