#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED
#include<SDL2/SDL.h>
#include<string.h>
#include<stdio.h>
#include<math.h>
//#include<unistd.h>
#include <stdlib.h>
#include <vector>
#include <string>
extern SDL_Renderer * rd;
extern SDL_Texture * fnt;
extern int mouse_x,mouse_y,mouse_bt,mouse_pulse,mouse_dbl,olmouse_bt,mouse_edup,mouse_eddn;
extern int keyctrl,keyshift,enterpressed;
extern bool kben;
extern int curpos;
extern int spmask[]; 
extern int keys[256];
extern int keycombo[256];

struct ivec2{
	int x,y;
	ivec2(){x=0;y=0;}
	ivec2(int xx,int yy){x=xx;y=yy;}
	ivec2 operator+(const ivec2& a){ivec2 vez;vez.x=x+a.x;vez.y=y+a.y;return vez;}
	ivec2 operator-(const ivec2& a){ivec2 vez;vez.x=x-a.x;vez.y=y-a.y;return vez;}
	ivec2 operator*(const float& a){ivec2 vez;vez.x=x*a;vez.y=y*a;return vez;}
	ivec2 operator/(const float& a){ivec2 vez;vez.x=x/a;vez.y=y/a;return vez;}

};
struct color{int r,g,b,a;
	color(int rr,int gg,int bb,int aa){r=rr;g=gg;b=bb;a=aa;}
	color (int rr,int gg,int bb){r=rr;g=gg;b=bb;a=255;}
	color (int gray){r=gray;g=gray;b=gray;a=255;}
	color(){r=0;g=0;b=0;a=0;}
};
void Line(ivec2 p1,ivec2 p2,color col);
void Contour(ivec2 pos,ivec2 siz,color col);
void Box(ivec2 p1,ivec2 p2,color col);
void Print(ivec2 pos, color col, const char * text,...);
bool MouseTest(ivec2 pos,ivec2 siz);
void AreaLock(ivec2 ini,ivec2 ed);
void AreaUnlock();

void ConnectKBD(std::string * pointer);
void UnconnectKB();
int tsdl(int tsdl);
void RunKeyboard();
struct sstate{
	int value[6];std::string text;
};

class Window;

class Control{
public:
	SDL_Texture * texture;
	ivec2 texdim,texzoom,pos,siz;
	char name[32];
	bool focus,textfocus;
	std::string text;
	int texsize,value[6];
	enum ctrl{BUTTON,SCROLL,TOGGLE,RADIO,MENU,LABEL,TEXT,COMBO,LIST,PICTURE,EPICTURE,TABS};
	ctrl type;
	int priority;
	Control(Window *win,ctrl type,ivec2 pos, ivec2 siz);
	Control(ctrl tp,ivec2 posi,ivec2 siz);
	~Control();
	void RunControl(Window * inwi,bool butenable);
	void setValue(int id, int value);
	int getValue(int id);
	void GetText(char * textout);
	void SetText(const char * textin);
};

/*
struct NControl{
	char name[32];
	ivec2 pos,siz;
	bool focus,textfocus;
	enum ctrl{BUTTON,SCROLL,TOGGLE,RADIO,MENU,LABEL,TEXT,COMBO,LIST,PICTURE,EPICTURE,TABS};
	ctrl type;
	int priority;
	virtual void Run(Window * inwi, bool butenable);
	virtual void setValue(int id, int value);
	virtual int getValue(int id);
	virtual void GetText(char * textout);
	virtual void SetText(const char * textin);
};
*/


class Window{
	SDL_Texture * font;
	//Control ** controls;
	std::vector<Control*> controls;
	int conct;
	color shades[4];
	int findct(char * name);
public:
	enum ctrln{BUTTON,SCROLL,TOGGLE,RADIO,MENU,LABEL,TEXT,COMBO,LIST,PICTURE,EPICTURE,TABS};
	ivec2 pos,siz;
    int radioch[32];
	void DrawBorder(ivec2 pos,ivec2 siz,int btype);
	void DrawBorderFilled(ivec2 pos,ivec2 siz,int btype,color fall);
	void DrawBorderLine(ivec2 v1,ivec2 v2,bool raised,bool thick);

	Window(SDL_Renderer * render,int maxct,ivec2 ipos,ivec2 isiz);
	void RegisterControl(Control * ct);
	void UnregisterControl(Control * ct);
	void RunWindow();
	void AddBuffer(int id,SDL_Texture * bf,ivec2 siz);
	void RunScroll(ivec2 po,ivec2 si,int * value);

	void AddControl(char * name,Control::ctrl cttype,ivec2 pos,ivec2 siz);
	void AddControl(const char * name,Control::ctrl cttype,ivec2 pos,ivec2 siz){AddControl((char*)name,cttype,pos,siz);}

	void AddControl(char * name,Control::ctrl cttype,ivec2 pos,ivec2 siz, char * initext);
	void AddControl(char * name,Control::ctrl cttype,ivec2 pos,ivec2 siz,int inival);
	int GetValue(char * name,int valid);
	void SetValue(char *name,int valid,int valnum);
	int GetValue(const char * name,int valid){return GetValue((char*)name,valid);}
	void SetValue(const char *name,int valid,int valnum){SetValue((char*)name,valid,valnum);}

	void GetText(char *name, char * text);
	void GetText(const char *name, char * text){GetText((char*)name,text);}

	void SetText(char *name, char * text);
	void SetText(const char *name, const char * text){SetText((char*)name,(char*)text);}
	void SetText(const char *name, char * text){SetText((char*)name,text);}

	int GetTextAsNumber(char *name);
    void SetNumberAsText(char *name,int number);

	int GetTextAsNumber(const char *name){return GetTextAsNumber((char*)name);}
    void SetNumberAsText(const char *name, int number){SetNumberAsText((char*)name,number);}
	void SetBitmap(char *name, char *bmp);
	ivec2 GetSize(char *name);
	std::vector <sstate> SaveState();
	void LoadState(std::vector<sstate> din);
};
extern char * kbcon;
SDL_Texture * LoadFont (const char * ftn);
/*
void strcpy_s(char *out, int nb,const char*in); //Awful kludge to make it compile
void strcat_s(char *out, int nb,const char*in);
void memcpy_s(void * dest,int maxsiz,void * src,int siz);
*/
#endif // GUI_H_INCLUDED

