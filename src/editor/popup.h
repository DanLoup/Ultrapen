#ifndef POPUP_H_INCLUDED
#define POPUP_H_INCLUDED
#include "Gui.h"
#include "data.h"
//#include "dirent.h"
#include <sys/stat.h>

struct ffilter{
	std::string label;
	std::vector <std::string> extens;
	bool all;
	bool foldermode;
};

struct dirfile { char fname[512]; int type; };

extern Window *palwin,*filewin;

void InitPaletteWindow();
void RunPaletteWindow();
void InitFileWindow();
void RunFileWindow();
void BootFileMenu();
void InitNewFileMenu();
void RunNewFileMenu();
void RunBuildMenu();
void InitBuildMenu();

void LoadBMP();
void SaveBMP();
bool LoadDir(char* dirin, bool fmode);

extern int popmode;
extern bool gotfile;
extern char outfile[1024];
extern char curdir[1024];
extern char outfilemini[1024];
extern char filter[1024];
extern char filtername[1024];
extern int newfitp;
extern bool newfi;
extern ffilter filt[32];
extern int filtct;
extern bool foldermode;
extern std::vector <dirfile> df;

#endif // POPUP_H_INCLUDED
