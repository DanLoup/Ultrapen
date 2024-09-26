#include "MainWindow.h"
#include "SubWindows.h"
#include "data.h"
#include "tinyfiledialogs.h"
#include "popup.h"
Window * mainw;
Control * cts[20];
extern SDL_Renderer * rd;
extern bool running;
enum {W_FILEMENU,W_FILEMENU2,W_LVRADIO,W_BLRADIO,W_OBRADIO,W_MUSRADIO,W_INSRADIO,W_TABS};
void RunPalette(ivec2 pos,Window * winin);
void RunPaletteWindow();
void RunFileWindow();
void InitFileWindow();
void InitPaletteWindow();
char st_text[200];
int filemode = 0;
char fib[600];
char savfor[100],savtit[100];

void RunFiles();
void fllldescs();
std::vector <Control*> tabs;
void InitalizeMainWindow(){
	mainw=new Window(rd,50,ivec2(0,0),ivec2(1024,768));
	cts[W_FILEMENU]=new Control(mainw,Control::MENU,ivec2(0,0),ivec2(6,6));
	cts[W_FILEMENU2]=new Control(mainw,Control::MENU,ivec2(60,0),ivec2(6,6));
    cts[W_FILEMENU]->SetText("File|New|Open|Save|Save as|Import|Build|Exit");
	cts[W_TABS]=new Control(mainw,Control::TABS,ivec2(0,22),ivec2(640,27));
	cts[W_TABS]->SetText("");
	//for (int a=0;a<4;a++){cts[a+W_LVRADIO]->setValue(2,a);}mainw->radioch[0]=0;
	color defpal[]={
		color(160),color(192),color(0,153,0),color(81,187,81),color(128,43,128),color(255,81,255),color(153,43,0),color(36),
		color(187,81,43),color(0,0,255),color(255,176,0),color(255,240,230),color(0,255,255),color(192,192,192),color(128,176,0),color(255)
	};
	for (int a=0;a<16;a++){pals[0].col[a]=defpal[a];}
	strcpy(st_text,"Status bar");
	strcpy(flexmenu,"Level|Import|Export");

	//InitAssembler();
	//InitTileEditor();
	//InitObjectEditor();
	//InitCourseEditor();

	InitFileWindow();
	InitPaletteWindow();
	InitNewFileMenu();
	InitBuildMenu();
	fllldescs();
	strcpy(fib,get_current_dir_name());
	
}
int submode=0;
bool edgedel=false;
void RunWindows(){
	int curswin=mainw->radioch[0];
	mainw->DrawBorderLine(ivec2(0,20),ivec2(1024,20),false,false);
	mainw->DrawBorderLine(ivec2(0,50),ivec2(1024,50),false,false);
	mainw->DrawBorderLine(ivec2(0,740),ivec2(1024,740),false,false);
	Print(ivec2(5,745),color(0),st_text);
	bool redomenu = false;
	bool actualfile=false;
	if (cts[W_FILEMENU]->getValue(0) == 1){ popmode = 3;}


	if (cts[W_FILEMENU]->getValue(0) == 6) { popmode = 4; }
	if (cts[W_FILEMENU]->getValue(0) == 5) { 
		ImportOldDataFile();
		redomenu = true;
		//LoadObjControls();
	}


	if (cts[W_FILEMENU]->getValue(0)==2 || cts[W_FILEMENU]->getValue(0)==3 || cts[W_FILEMENU]->getValue(0)==4){
		if (cts[W_FILEMENU]->getValue(0)==2){
				filt[0].label="Upen Files";filt[0].all=false;
				filt[0].extens.clear();filt[0].extens.push_back(".utl");filt[0].extens.push_back(".uob");filt[0].extens.push_back(".ulv");
				filtct=1;filemode=1;
				actualfile=true;
		}else{
			curfile=cts[W_TABS]->getValue(1);
			int curtab=fils[curfile]->typ;
			if (fils.size()>0){
				if (cts[W_FILEMENU]->getValue(0)==3){
					if (!fils[curfile]->named){
						actualfile=true;
					}else{
						actualfile=false;
						gotfile=true;
						filemode=2;
						strcpy(outfile,fils[curfile]->fpath.c_str());
					}
				}else{
					actualfile=true;
				}
				if (actualfile){
					if (curtab==FileType::FT_TILESET){strcpy(savtit,"Upen Tilset");strcpy(savfor,"*.utl");}
					if (curtab==FileType::FT_OBJECTSET){strcpy(savtit,"Upen Object Set");strcpy(savfor,"*.uob");}
					if (curtab==FileType::FT_LEVEL){strcpy(savtit,"Upen Level");strcpy(savfor,"*.ulv");}
					if (curtab==FileType::FT_PROGRAMSET){strcpy(savtit,"Upen Program");strcpy(savfor,"*.upr");}
					if (curtab==FileType::FT_MUSICSET){strcpy(savtit,"Upen Song set");strcpy(savfor,"*.uss");}
					if (curtab==FileType::FT_INSTRUMENTSET){strcpy(savtit,"Upen Instrument set");strcpy(savfor,"*.uis");}
					filemode=2;
				}
			}
		}
		filt[filtct].label="All files";filt[filtct].all=true;filt[filtct].extens.clear();
		filtct++;
		if (actualfile){
			if (filemode==1){
				char * texto;
        		char const * filt[] = {"*.utl","*.ulv","*.uob","*.upr","*.uss","*.uis"};
				SDL_CaptureMouse(SDL_FALSE);
				texto = tinyfd_openFileDialog("Load Ultrapen file",fib,6,filt,"Ultrapen files",0);
				if (texto != NULL){
					strcpy(outfile,texto); gotfile=true;
					for (int a = strlen(outfile);a > 0;a--){
                		if (outfile[a]=='/'){strcpy(outfilemini,&outfile[a+1]);break;}
                		if (outfile[a]=='\\'){strcpy(outfilemini,&outfile[a+1]);break;}
                	}
                	strcpy(fib,texto);
                	for (int a = strlen(fib);a > 0;a--){
                    	if (fib[a]=='/'){fib[a]=0;break;}if (fib[a]=='\\'){fib[a]=0;break;}
                	}
				}
				LoadPal(outfile);
			}
		
			if (filemode==2){
				char * texto;
        		char * filt[2];
				filt[0]=new char[100];
				strcpy(filt[0],savfor);
				SDL_CaptureMouse(SDL_FALSE);
				texto = tinyfd_saveFileDialog("Save ultrapen file as",fib,1,filt,savtit);
				if (texto[strlen(texto)-4]!='.'){
					strcat(texto,&savfor[1]);
				}
				delete filt[0];
				if (texto != NULL){
					strcpy(outfile,texto); gotfile=true;
					for (int a = strlen(outfile);a > 0;a--){
                		if (outfile[a]=='/'){strcpy(outfilemini,&outfile[a+1]);break;}
                		if (outfile[a]=='\\'){strcpy(outfilemini,&outfile[a+1]);break;}
                	}
                	strcpy(fib,texto);
                	for (int a = strlen(fib);a > 0;a--){
                    	if (fib[a]=='/'){fib[a]=0;break;}if (fib[a]=='\\'){fib[a]=0;break;}
                	}
				}
				SavePal(outfile);

			}
		}

	}

	if (cts[W_FILEMENU]->getValue(0)==7){running=false;}
	cts[W_FILEMENU]->setValue(0,0);
	if (fils.size()>0){
		curfile=cts[W_TABS]->getValue(1);
		int curtab=fils[curfile]->typ;
		//if (curtab==FileType::FT_CODESET){RunAssembler();}
		fils[curfile]->Run(0);
		//if (curtab==FileType::FT_TILESET){RunTileEditor();}
		//if (curtab==FileType::FT_OBJECTSET){RunObjectEditor();}
		//if (curtab==FileType::FT_LEVEL){RunCourseEditor();}
		//if (curtab==FileType::FT_PROGRAMSET){RunAssembler();}
	}

	cts[W_FILEMENU2]->SetText(flexmenu);
	flexmenuout=cts[W_FILEMENU2]->getValue(0);
	cts[W_FILEMENU2]->setValue(0,0);
	mainw->RunWindow();
	if (popmode == 1){RunPaletteWindow();}
	if (popmode == 2){RunFileWindow();}
	if (popmode == 3){RunNewFileMenu();}
	if (popmode == 4){RunBuildMenu();}
	

	if (gotfile && filemode>0){
		
		if (filemode==1){
			int old=0,valid=0;
			gotfile=false;
			if (LoadFile(outfile, outfilemini,false)) {
				fils.push_back(fileoutld);
				curfile = fils.size() - 1;
				//if (fileoutld->typ == FileType::FT_OBJECTSET) { LoadObjControls(); }
				fils.back()->Boot(false);
			}
            filemode=0;
			redomenu = true;
			LoadPal(outfile);
		}
		if (filemode==2){
			int old=0,valid=0;
			FILE* of;

			of=fopen(outfile,"wb");
			if (of){
				unsigned char * tbuff=new unsigned char[655350];
				int siz=0;
				int curtab=fils[curfile]->typ;
				siz = fils[curfile]->Encode(tbuff);
				if (curtab!=FileType::FT_PROGRAMSET){
					fwrite("upen",4,1,of);
					fwrite(&curtab,4,1,of);
				}
				fwrite(tbuff,1,siz,of);
				fclose(of);
				fils[curfile]->name=outfilemini;
				fils[curfile]->named=true;
				fils[curfile]->fpath=outfile;
				delete [] tbuff;
			}
			gotfile=false;
            filemode=0;
			SavePal(outfile);
		}

		LoadCourseControls();
		redomenu=true;
		cts[W_TABS]->value[1]=fils.size()-1;
		//Kludgy, but guarantees that all opened level files will not have outdated files
		for (int a = 0; a < fils.size(); a++) {
			if (fils[a]->typ == FileType::FT_LEVEL) {
				LVFile* acc = (LVFile*)fils[a];
				acc->outdated = true;
			}
		}
	
	}
	if (bmfilemode>0 && gotfile){
		if (bmfilemode==1){
			SDL_Surface * surfy=SDL_LoadBMP(outfile);
			SDL_LockSurface(surfy);
			bmwi=surfy->w;
			bmhe=surfy->h;
			int sizu=surfy->pitch*surfy->h;
			bmbytes=surfy->pitch/surfy->w;
			memcpy(bitmem,surfy->pixels,sizu);
			SDL_UnlockSurface(surfy);
			SDL_FreeSurface(surfy);
			bmdone=true;
		}else{
			if (bmwi>0 && bmwi<2048 && bmhe>0 && bmhe<2048){
				SDL_Surface * surfy=SDL_CreateRGBSurfaceWithFormat(0,bmwi,bmhe,32,SDL_PIXELFORMAT_RGBA32);
				SDL_LockSurface(surfy);
				int sizu=surfy->pitch*surfy->h;
				memcpy(surfy->pixels,bitmem,sizu);
				SDL_UnlockSurface(surfy);
				SDL_SaveBMP(surfy,outfile);
				SDL_FreeSurface(surfy);
			}
		}
		bmfilemode=0;gotfile=false;
	}

	if (newfi){
		char unames[][24]={"NewLevel.ulv","NewObj.uob","NewTiles.utl","NewSongSet.uss","NewInstrSet.uis","NewProgram.upr"};
        //Level|Palette|Objects|TileMap|Corridors|Music|Programs
		if (newfitp==FileType::FT_TILESET){
			BGtileF * newfil=new BGtileF();
			newfil->name=unames[newfitp];
			newfil->typ=(FileType::ftt)newfitp;
			newfil->named=false;
			fils.push_back(newfil);
			fils.back()->Boot(true);
		}
		if (newfitp==FileType::FT_OBJECTSET){
			OBtileF * newfil=new OBtileF();
			newfil->name=unames[newfitp];
			newfil->typ=(FileType::ftt)newfitp;
			newfil->named=false;
			fils.push_back(newfil);
			fils.back()->Boot(true);
		}
		if (newfitp==FileType::FT_LEVEL){
			LVFile * newfil=new LVFile();
			newfil->name=unames[newfitp];
			newfil->typ=(FileType::ftt)newfitp;
			newfil->named=false;
			fils.push_back(newfil);
			fils.back()->Boot(true);
		}
		if (newfitp==FileType::FT_PROGRAMSET){
			ProgramFile * newfil=new ProgramFile();
			newfil->name=unames[newfitp];
			newfil->typ=(FileType::ftt)newfitp;
			newfil->named=false;
			fils.push_back(newfil);
			fils.back()->Boot(true);
		}
		if (newfitp==FileType::FT_MUSICSET){
			MusicSet * newfil=new MusicSet();
			newfil->name=unames[newfitp];
			newfil->typ=(FileType::ftt)newfitp;
			newfil->named=false;
			fils.push_back(newfil);
			fils.back()->Boot(true);
		}

		if (newfitp==FileType::FT_INSTRUMENTSET){
			InstrumentSet * newfil=new InstrumentSet();
			newfil->name=unames[newfitp];
			newfil->typ=(FileType::ftt)newfitp;
			newfil->named=false;
			fils.push_back(newfil);
			fils.back()->Boot(true);
		}

		newfi=false;
		redomenu=true;
		cts[W_TABS]->value[1]=fils.size()-1;
	}

	if (cts[W_TABS]->value[5]>0 && !edgedel && fils.size()>0){
		edgedel=true;
		int ded=cts[W_TABS]->value[1];
		int typ = fils[ded]->typ;
		if (typ==FileType::FT_TILESET){
			delete static_cast<BGtileF*>(fils[ded]); 
		}
		if (typ==FileType::FT_OBJECTSET){
			delete static_cast<OBtileF*>(fils[ded]); 
		}
		if (typ == FileType::FT_LEVEL) {
			delete static_cast<LVFile*>(fils[ded]); 
		}
		if (typ == FileType::FT_PROGRAMSET) {
			delete static_cast<ProgramFile*>(fils[ded]); 
		}
		if (typ == FileType::FT_MUSICSET) {
			delete static_cast<MusicSet*>(fils[ded]); 
		}
		if (typ == FileType::FT_INSTRUMENTSET) {
			delete static_cast<InstrumentSet*>(fils[ded]); 
		}

		fils.erase(fils.begin()+ded);
		redomenu=true;
	}

	if (cts[W_TABS]->value[5]==0){edgedel=false;}
	if (redomenu){
		std::string assemble;
		for (int a=0;a<fils.size();a++){
			assemble.append(fils[a]->name);assemble.append("|");
		}
		cts[W_TABS]->SetText(assemble.c_str());
		if (cts[W_TABS]->value[1]>fils.size()-1){cts[W_TABS]->value[1]=fils.size()-1;}
	}
}


void RunPalette(ivec2 pos,Window * winin){
	ivec2 ppos=pos+winin->pos;
	winin->DrawBorderFilled(ppos,ivec2(520,40),2,color(192));
	for (int a=0;a<16;a++){Box(ppos+ivec2((a*32)+4,4),ppos+ivec2((a*32)+36,36),pals[0].col[a]);}
	winin->DrawBorderFilled(ppos+ivec2(520,0),ivec2(40,40),1,color(192));
	Print(ppos+ivec2(7+(selL*32),9),color(0),"L");Print(ppos+ivec2(5+(selL*32),7),color(255),"L");
	Print(ppos+ivec2(7+(selR*32),9),color(0),"R");Print(ppos+ivec2(5+(selR*32),7),color(255),"R");
	int mix=selL|selR;
	Print(ppos+ivec2(21+(mix*32),19),color(0),"M");Print(ppos+ivec2(19+(mix*32),17),color(255),"M");
	if (MouseTest(ppos+ivec2(4,4),ivec2(520,40))){
		int selcol=(mouse_x-(ppos.x+4))/32;
		if (mouse_bt==1){selL=selcol;}
		if (mouse_bt==4){selR=selcol;}
		if (mouse_dbl>0){popmode=1;}
	}
}

void UpdateFiles(){

}
