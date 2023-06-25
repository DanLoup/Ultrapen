#include "popup.h"
#include <dirent.h>
#include <unistd.h>
#include "compiler.h"

Window *palwin,*filewin;
Control * pwin_butok,*pwin_butcan;
Control * pwin_RGB[3];
SDL_Texture * rgbtex;
color colorwh[24][8];
int popmode=0;
#ifdef WIN32
char bar[]="\\";
#else
char bar[]="/";
#endif

void InitPaletteWindow(){
	palwin=new Window(rd,50,ivec2(100,100),ivec2(600,400));
	pwin_butok=new Control(palwin,Control::BUTTON,ivec2(530,370),ivec2(60,20));pwin_butok->SetText("Ok");
	pwin_butcan=new Control(palwin,Control::BUTTON,ivec2(10,370),ivec2(60,20));pwin_butcan->SetText("Cancel");
	for (int a=0;a<3;a++){
		pwin_RGB[a]=new Control(palwin,Control::SCROLL,ivec2(10,(300)+a*20),ivec2(130,20));
	}
	rgbtex=SDL_CreateTexture(rd,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,256,256);

	color pricolor[24];
	for (int a=0;a<8;a++){
		pricolor[a]=color((8-a)*2,a*2,0);pricolor[a+8]=color(0,(8-a)*2,a*2);pricolor[a+16]=color(a*2,0,(8-a)*2);
	}
	for (int a=0;a<24;a++){
		if (pricolor[a].r>7){pricolor[a].r=7;}if (pricolor[a].g>7){pricolor[a].g=7;}if (pricolor[a].b>7){pricolor[a].b=7;}
	}
	int mmcr,mmcg,mmcb;
	for (int a=0;a<24;a++){
		for (int b=0;b<8;b++){
			mmcr=((pricolor[a].r*(7-b))+(b*3))/7;mmcg=((pricolor[a].g*(7-b))+(b*3))/7;mmcb=((pricolor[a].b*(7-b))+(b*3))/7;
			if (mmcr>7){mmcr=7;}if (mmcg>7){mmcg=7;}if (mmcb>7){mmcb=7;}
			colorwh[a][b].r=mmcr;colorwh[a][b].g=mmcg;colorwh[a][b].b=mmcb;
		}
	}

}
char rgbpictex[262144];
color curcol;
int palposed=0,curpaled=0;
color colstrip[16];
void RunPaletteWindow(){
AreaUnlock();
	for (int a=0;a<24;a++){
		for (int b=0;b<8;b++){
			int cr=(b+(a*256))*4;
			rgbpictex[cr+3]=colorwh[a][b].r*36;rgbpictex[cr+2]=colorwh[a][b].g*36;rgbpictex[cr+1]=colorwh[a][b].b*36;
		}
	}
	int mmcr,mmcg,mmcb;
	ivec2 ppos=palwin->pos+ivec2(0,0);
	palwin->DrawBorderFilled(ppos,ivec2(600,400),1,color(192));
	palwin->DrawBorderFilled(ppos+ivec2(575,5),ivec2(20,20),0,color(192));
	palwin->DrawBorderLine(ppos+ivec2(2,25),ppos+ivec2(596,25),false,false);
	palwin->DrawBorderFilled(ppos+ivec2(10,30),ivec2(128+8,256+8),2,color(192));
	palwin->DrawBorderFilled(ppos+ivec2(138+8,30),ivec2(64+8,256+8),2,color(192));
	palwin->DrawBorderFilled(ppos+ivec2(230,30),ivec2(320+8,320+8),2,color(192));

	palwin->DrawBorderFilled(ppos+ivec2(138+8,300),ivec2(52+8,52+8),2,pals[curpaled].col[palposed]);

	SDL_Rect sc,prs;sc.x=0;sc.y=0;sc.w=8;sc.h=24;prs.x=ppos.x+14;prs.y=ppos.y+34;prs.w=128;prs.h=256;
	SDL_RenderCopy(rd,rgbtex,&sc,&prs);
	int mcr=curcol.r,mcg=curcol.g,mcb=curcol.b;
	for (int a=0;a<15;a++){
		if (a<8){mmcr=(mcr*a)/7;mmcg=(mcg*a)/7;mmcb=(mcb*a)/7;}
		if (a>7){mmcr=mcr+(a-7);mmcg=mcg+(a-7);mmcb=mcb+(a-7);}
		if (mmcr>7){mmcr=7;}if (mmcg>7){mmcg=7;}if (mmcb>7){mmcb=7;}
        Box(ppos+ivec2(138+8+4,34+(a*16)),ppos+ivec2(138+8+4+64,34+16+(a*16)),color(mmcr*36,mmcg*36,mmcb*36));
		colstrip[a]=color(mmcr*36,mmcg*36,mmcb*36);
	}
	SDL_Rect tsize; tsize.x=0;tsize.y=0;tsize.w=256;tsize.h=256;SDL_UpdateTexture(rgbtex,&tsize,rgbpictex,1024);

	Print(ppos+ivec2(5,5),color(0),"Color Picker");
	palwin->RunWindow();
	ivec2 pps=ppos+ivec2(10,30);
	if (MouseTest(pps,ivec2(128,256)) && mouse_bt==1){
		int cx=(mouse_x-(pps.x))/16,cy=(mouse_y-pps.y)/10.6666;curcol=colorwh[cy][cx];
	}
	pps=ppos+ivec2(138+8,30);
	if (MouseTest(pps,ivec2(64,256)) && mouse_bt==1){
		int selc=(mouse_y-pps.y)/16;pwin_RGB[0]->setValue(0,colstrip[selc].r*4);pwin_RGB[1]->setValue(0,colstrip[selc].g*4);pwin_RGB[2]->setValue(0,colstrip[selc].b*4);
	}
	pals[curpaled].col[palposed]=color((pwin_RGB[0]->getValue(0)/128)*36,(pwin_RGB[1]->getValue(0)/128)*36,(pwin_RGB[2]->getValue(0)/128)*36);
	pps=ppos+ivec2(230+5,30+5);
	for (int b=0;b<16;b++){
		for (int a=0;a<16;a++){
			Box(pps+ivec2(a*20,b*20),pps+ivec2(a*20,b*20)+ivec2(18,18),pals[b].col[a]);
		}
	}
	Contour(pps+ivec2((palposed*20)-1,(curpaled*20)-1),ivec2(19,19),color(255,0,0));
	if (MouseTest(pps,ivec2(320,320)) && mouse_bt==1){
		int cx=(mouse_x-(pps.x))/20,cy=(mouse_y-pps.y)/20;curpaled=cy;palposed=cx;
		pwin_RGB[0]->setValue(0,pals[curpaled].col[palposed].r*4);
		pwin_RGB[1]->setValue(0,pals[curpaled].col[palposed].g*4);
		pwin_RGB[2]->setValue(0,pals[curpaled].col[palposed].b*4);

	}
AreaLock(ivec2(0,0),ivec2(4000,4000));
if (pwin_butok->getValue(0)>0){AreaUnlock();popmode=0;}
if (pwin_butcan->getValue(0)>0){AreaUnlock();popmode=0;}
}

Control * filscr,*fildir,*filnam,*filok,*filcan,*filtype;
std::vector <dirfile> df;
bool foldermode = false;
int ftype[1000];
int curfilesel=0;
char tmpdir[1024];
bool gotfile=false;
char outfile[1024],filter[1024],filtername[1024],outfilemini[1024], curdir[1024];

int oldfilter=0;
ffilter filt[32];
int filtct=0;
bool LoadDir(char *dirin,bool fmode){
	int isvalid=false;
	struct stat statbuff;
	stat(dirin,&statbuff);
	isvalid=S_ISDIR(statbuff.st_mode);

if (isvalid){
	DIR * dire;
	dirent *diro;
	dirfile cfile;
	int isdir=0;
	int dirfilecount=0;
	dire=opendir(dirin);
	while (readdir(dire)!=nullptr){dirfilecount++;}
	rewinddir(dire);
	//df=new dirfile[dirfilecount+10];
	df.clear();
	int cfilt=filtype->getValue(0);
	strcpy(cfile.fname,"..");cfile.type = 1; df.push_back(cfile);
	for (int tp=1;tp>-1;tp--){
		diro=readdir(dire);
		bool skipfile=false,fdfile=false;
		while (diro!=nullptr){
			tmpdir[0]=0;strcat(tmpdir,dirin);strcat(tmpdir,"/");strcat(tmpdir,diro->d_name);
			stat(tmpdir,&statbuff);
			isdir=0;if (S_ISDIR(statbuff.st_mode)){isdir=1;}
			skipfile=false;
			if (tp==0 && isdir==0){
				if (!filt[cfilt].all){
					skipfile=true;
					for (int b=0;b<filt[cfilt].extens.size();b++){
						int fipo=strlen(diro->d_name)-4;
						fdfile=true;
						for (int c=0;c<4;c++){if ((diro->d_name[fipo+c] | 32)!=(filt[cfilt].extens[b][c] | 32)){fdfile=false;}}
						if (fdfile){skipfile=false;break;}
					}
				}
			}
			if (isdir == 1 && diro->d_name[0]=='.'){skipfile = true;}
			if (fmode && isdir == 0) { skipfile = true; }
			if (!skipfile) { if (isdir == tp) { strcpy(cfile.fname,diro->d_name); cfile.type = isdir; df.push_back(cfile); } }
			diro=readdir(dire);
		}
		rewinddir(dire);
	}
	closedir(dire);
}

	return isvalid;
}

void InitFileWindow(){
	filewin=new Window(rd,50,ivec2(100,100),ivec2(800,500));
	filscr=new Control(filewin,Control::SCROLL,ivec2(620,80),ivec2(20,345));
	fildir=new Control(filewin,Control::TEXT,ivec2(20,40),ivec2(600,24));
	filnam=new Control(filewin,Control::TEXT,ivec2(20,430),ivec2(450,24));
	filtype=new Control(filewin,Control::COMBO,ivec2(470,430),ivec2(174,24));filtype->SetText("MapFiles|All files|");
	filcan=new Control(filewin,Control::BUTTON,ivec2(20,470),ivec2(60,20));filcan->SetText("Cancel");
	filok=new Control(filewin,Control::BUTTON,ivec2(580,470),ivec2(60,20));filok->SetText("Ok");
	getcwd(curdir, 1024);
	//LoadDir(curdir);
	fildir->SetText(curdir);
	filnam->SetText("");
}


void RunFileWindow(){
	AreaUnlock();
	ivec2 ppos=palwin->pos+ivec2(0,0);
	palwin->DrawBorderFilled(ppos,ivec2(650,500),1,color(192));
	palwin->DrawBorderFilled(ppos+ivec2(625,5),ivec2(20,20),0,color(192));
	palwin->DrawBorderLine(ppos+ivec2(2,25),ppos+ivec2(646,25),false,false);
	palwin->DrawBorderFilled(ppos+ivec2(20,80),ivec2(600,345),2,color(255));
	Print(ppos+ivec2(5,5),color(0),"File Open");
	int muu = df.size()-25;
	if (muu < 1){muu = 1;}
	float scrollmt=(float(muu))/1024.0f;
	if (scrollmt<0){scrollmt=0;}
	float scrollval=float(filscr->getValue(0))*scrollmt;

	int curfn=scrollval;
	int ccol;
	if (curfn<(int)df.size()){
		for (int a=0;a<26;a++){
			ccol=0;
			if (curfn==curfilesel){Box(ppos+ivec2(24,84+(a*13)),ppos+ivec2(616,97+(a*13)),color(0,0,255));ccol=255;}
			if (df[curfn].type==0){
				Box(ppos+ivec2(28,85+(a*13)),ppos+ivec2(38,96+(a*13)),color(0,0,0));
				Box(ppos+ivec2(29,86+(a*13)),ppos+ivec2(37,95+(a*13)),color(192,192,192));
				Box(ppos+ivec2(29,86+(a*13)),ppos+ivec2(36,94+(a*13)),color(224,224,224));
				for (int b=0;b<3;b++){Box(ppos+ivec2(30,88+(a*13)+(b*2)),ppos+ivec2(36,89+(a*13)+(b*2)),color(128,128,128));}
			}else{
				Box(ppos+ivec2(26,88+(a*13)),ppos+ivec2(38,96+(a*13)),color(0,0,0));
				Box(ppos+ivec2(26,86+(a*13)),ppos+ivec2(35,96+(a*13)),color(0,0,0));
				Box(ppos+ivec2(27,89+(a*13)),ppos+ivec2(37,95+(a*13)),color(192,192,64));
				Box(ppos+ivec2(27,89+(a*13)),ppos+ivec2(36,94+(a*13)),color(255,255,192));
				Box(ppos+ivec2(27,87+(a*13)),ppos+ivec2(34,93+(a*13)),color(255,255,192));
			}
			Print(ppos+ivec2(40,84+(a*13)),color(ccol),"%s",df[curfn].fname);
			curfn++;
			if (curfn> df.size() -1){break;}
		}
	}
	ivec2 pps=ppos+ivec2(40,84);
	bool loadfil=false;
	if (MouseTest(pps,ivec2(600-16,345)) && mouse_bt==1){

		curfilesel=((mouse_y-pps.y)/13)+scrollval;
		if (curfilesel > df.size() - 1) { curfilesel = df.size() - 1; }
		if (df[curfilesel].type==0){
			filnam->SetText(df[curfilesel].fname);
			if (mouse_dbl>0){loadfil=true;}
		}

		if (mouse_dbl>0){
			if (df[curfilesel].type==1){
				if (df[curfilesel].fname[0]=='.'){
					//You have to go back, jack
					char bkp[1024];
					strcpy(bkp,curdir);fildir->GetText(curdir);
					for (int a=strlen(curdir)-2;a>-1;a--){
                        if (curdir[a]=='\\' || curdir[a]=='/'){
							if (a>3){
								curdir[a]=0;
								break;
							}else{
								curdir[a+1]=0;
							}

						}
					}
					if (!LoadDir(curdir,foldermode)){
							strcpy(curdir,bkp);}
					fildir->SetText(curdir);
				}else{
					fildir->GetText(curdir);
					if (!(curdir[strlen(curdir)-1]=='\\' || curdir[strlen(curdir)-1]=='/')){strcat(curdir,bar);}
					strcat(curdir,df[curfilesel].fname);
					LoadDir(curdir, foldermode);
					fildir->SetText(curdir);
				}
			}
		}
	}
	filewin->RunWindow();
	if (fildir->getValue(2)>0){
		char bkp[1024];
		strcpy(bkp,curdir);fildir->GetText(curdir);
		if (!LoadDir(curdir, foldermode)){strcpy(curdir,bkp);fildir->SetText(curdir);}
	}
	AreaLock(ivec2(0,0),ivec2(4000,4000));
	if (filok->getValue(0)>0 || filnam->getValue(2)>0 || loadfil){
		gotfile=true;
		char filefetch[1024];
		filnam->GetText(filefetch);
		filnam->GetText(outfilemini);
		outfile[0]=0;
		strcat(outfile,curdir);strcat(outfile,"/");strcat(outfile,filefetch);
		if (outfile[strlen(outfile)-4]!='.'){strcat(outfile,filt[0].extens[0].c_str());strcat(outfilemini,filt[0].extens[0].c_str());}
        if (foldermode){LoadDir(curdir, false);}
		popmode=0;
		filok->setValue(0,0);

	}
	if (filcan->getValue(0)>0){
		gotfile=false;popmode=0;
		filcan->setValue(0,0);
	}
	if (filtype->getValue(0)!=oldfilter){LoadDir(curdir, foldermode);}
	oldfilter=filtype->getValue(0);
}
void BootFileMenu(){
	popmode=2;
	LoadDir(curdir, foldermode);
	fildir->SetText(curdir);
	filnam->SetText("");
	char fina[200];fina[0]=0;
	for (int a=0;a<filtct;a++){
		strcat(fina,filt[a].label.c_str());strcat(fina,"|");
	}
	filtype->SetText(fina);

}

Window *nfwin;
Control *nftypelist,*nfok,*nfcancel;
void InitNewFileMenu(){
	nfwin=new Window(rd,50,ivec2(100,100),ivec2(800,500));
	nftypelist=new Control(nfwin,Control::LIST,ivec2(10,40),ivec2(600,420));nftypelist->SetText("Level|Objects|TileMap|SongSet|Instrumset|Program|");
	nfcancel=new Control(nfwin,Control::BUTTON,ivec2(20,470),ivec2(60,20));nfcancel->SetText("Cancel");
	nfok=new Control(nfwin,Control::BUTTON,ivec2(580,470),ivec2(60,20));nfok->SetText("Ok");
}

int newfitp=0;
bool newfi=false;

void RunNewFileMenu(){
	ivec2 ppos=nfwin->pos+ivec2(0,0);
	nfwin->DrawBorderFilled(ppos,ivec2(650,500),1,color(192));
	nfwin->DrawBorderFilled(ppos+ivec2(625,5),ivec2(20,20),0,color(192));
	nfwin->DrawBorderLine(ppos+ivec2(2,25),ppos+ivec2(646,25),false,false);
	nfwin->RunWindow();

	if (nfcancel->getValue(0)>0){
		popmode=0;
		nfcancel->setValue(0,0);
	}
	if (nfok->getValue(0)>0){
		popmode=0;
		nfok->setValue(0,0);
		newfi=true;
		newfitp=nftypelist->value[0];
	}

}

void LoadBMP(){
	filt[0].label="Bitmap file";filt[0].extens.clear();filt[0].extens.push_back(".bmp");filtct=1;
	bmfilemode=1;
	BootFileMenu();
}
void SaveBMP(){
	filt[0].label="Bitmap file";filt[0].extens.clear();filt[0].extens.push_back(".bmp");filtct=1;
	bmfilemode=2;
	BootFileMenu();
}

Window* bdwin;
Control* bdtext, * bdbuild, * bdclose, *bdbupack,*bdcopych,*bdcopytx;
bool bdini = false, bdvalfi = false;

void InitBuildMenu() {
	bdwin = new Window(rd, 50, ivec2(100, 100), ivec2(800, 500));
	bdtext = new Control(bdwin, Control::LIST, ivec2(10, 40), ivec2(600, 420)); bdtext->SetText("Ready|");
	bdclose = new Control(bdwin, Control::BUTTON, ivec2(20, 470), ivec2(60, 20)); bdclose->SetText("Close");
	bdbuild = new Control(bdwin, Control::BUTTON, ivec2(580, 470), ivec2(60, 20)); bdbuild->SetText("Build");
	bdbupack = new Control(bdwin, Control::BUTTON, ivec2(480, 470), ivec2(90, 20)); bdbupack->SetText("Build pack");
	bdcopych = new Control(bdwin,Control::TOGGLE,ivec2(90,470),ivec2(60,20));bdcopych->SetText("Copy to");bdcopych->setValue(0,1);
	bdcopytx = new Control(bdwin,Control::TEXT,ivec2(160,470),ivec2(250,20));bdcopytx->SetText("../disk");
}

std::string Compile();
std::string CompilePack();


void RunBuildMenu() {
	ivec2 ppos = bdwin->pos + ivec2(0, 0);
	bdwin->DrawBorderFilled(ppos, ivec2(650, 500), 1, color(192));
	bdwin->DrawBorderFilled(ppos + ivec2(625, 5), ivec2(20, 20), 0, color(192));
	bdwin->DrawBorderLine(ppos + ivec2(2, 25), ppos + ivec2(646, 25), false, false);
	bdwin->RunWindow();
	if (!bdini) {
		bdini = true;
		if (fils.size() > 0) {
			if (fils[curfile]->typ != FileType::FT_LEVEL) {
				bdtext->SetText("Current file is not a level|");
			} else {
				bdvalfi = true;
			}
		}else{ bdtext->SetText("There's no level file open|"); }
	}

	if (bdclose->getValue(0) > 0) {
		bdini = false; popmode = 0; bdclose->setValue(0, 0);
	}
	if (bdbuild->getValue(0) == 1 && bdvalfi) {
		copyit = bdcopych->getValue(0);copyto = bdcopytx->text;
		bdtext->SetText(Compile().c_str());
	}
	if (bdbupack->getValue(0) == 1 ) {
		copyit = bdcopych->getValue(0);copyto = bdcopytx->text;
		bdtext->SetText(CompilePack().c_str());
	}


}

