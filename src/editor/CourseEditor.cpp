#include "MainWindow.h"
#include "data.h"
#include "popup.h"

void LoadCourseControls();
void SaveCourseControls();
void UpdateLinkWindow();

int ldfile = 0;

int curblock = 0, curfileblock = 0;
int heldobj=-1;

char obtnames[][15] = { "ultrapen","powerup","set exit","set palette" };
char stpic[10000];

SDL_Texture * cbuffers[2];
fb efbs[4];
bool loadedbuffs=false;

void LVFile::Boot(bool newf){
	cowin=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	subblo=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	subobj=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	subfile=new Window(rd,50,ivec2(0,60),ivec2(1024,600));

	coboxes[0]=new Control(cowin,Control::EPICTURE,ivec2(20,20),ivec2(768+8,224+8));
	coboxes[1]=new Control(cowin,Control::SCROLL,ivec2(20,244+8),ivec2(768+8,20));
	lscour=new Control(cowin,Control::BUTTON,ivec2(795,20),ivec2(20,232));lscour->SetText("<");
	fwcour=new Control(cowin,Control::BUTTON,ivec2(1004,20),ivec2(20,232));fwcour->SetText(">");
	courlab=new Control(cowin,Control::LABEL,ivec2(820,20),ivec2(100,100));courlab->SetText("Course:0");
	//glabels[0]=new Control(cowin,Control::LABEL,ivec2(820,40),ivec2(150,20));glabels[0]->SetText("num screens:");
	//gtexbox[0]=new Control(cowin,Control::TEXT,ivec2(820,60),ivec2(150,24));gtexbox[0]->SetText("0");

	coboxes[2]=new Control(subblo,Control::EPICTURE,ivec2(20,300),ivec2(512+8,256+8));
	subblo->AddControl("LinkDirection",Control::COMBO,ivec2(550,300),ivec2(100,24));subblo->SetText("LinkDirection","RIGHT|LEFT|DOWN|UP|");
	subblo->AddControl("LinkSourceScreen",Control::TEXT,ivec2(650,300),ivec2(50,24));subblo->SetText("LinkSourceScreen","0");
	subblo->AddControl("LinkDestCourse",Control::TEXT,ivec2(700,300),ivec2(50,24));subblo->SetText("LinkDestCourse","0");
	subblo->AddControl("LinkDestScreen",Control::TEXT,ivec2(750,300),ivec2(50,24));subblo->SetText("LinkDestScreen","0");
	subblo->AddControl("AddLink",Control::BUTTON,ivec2(810,300),ivec2(100,24));subblo->SetText("AddLink","Add Link");
	subblo->AddControl("RemoveLink",Control::BUTTON,ivec2(910,300),ivec2(100,24));subblo->SetText("RemoveLink","Rem Link");
	subblo->AddControl("LinkLabel",Control::LABEL,ivec2(550,280),ivec2(400,24));subblo->SetText("LinkLabel","Dir:      SSCRE: DCOURS: DSCRE:");
	subblo->AddControl("LinkList",Control::LIST,ivec2(550,330),ivec2(440,250));subblo->SetText("LinkList","");

	subobj->AddControl("ObjAdd",Control::BUTTON,ivec2(20,300),ivec2(150,24));subobj->SetText("ObjAdd","Add Object");
	subobj->AddControl("ObjRem",Control::BUTTON,ivec2(170,300),ivec2(150,24));subobj->SetText("ObjRem","Remove Object");
	subobj->AddControl("ObjGrid",Control::TOGGLE,ivec2(340,300),ivec2(150,24));subobj->SetText("ObjGrid","Lock to grid");

	subobj->AddControl("ObjType",Control::COMBO,ivec2(20,330),ivec2(150,24));subobj->SetText("ObjType","ultrapen|powerup|set exit|set palette|");

	subobj->AddControl("ObjP1", Control::TEXT, ivec2(120, 360), ivec2(50,24));subobj->SetText("ObjP1","0");
	subobj->AddControl("ObjP2", Control::TEXT, ivec2(120, 390), ivec2(50,24));subobj->SetText("ObjP2","0");
	subobj->AddControl("ObjP3", Control::TEXT, ivec2(120, 420), ivec2(50,24));subobj->SetText("ObjP3","0");
	subobj->AddControl("ObjP4", Control::TEXT, ivec2(120, 450), ivec2(50, 24)); subobj->SetText("ObjP4", "0");

	subobj->AddControl("ObjLP1", Control::LABEL, ivec2(20, 364), ivec2(90, 24)); subobj->SetText("ObjLP1", "param1:");
	subobj->AddControl("ObjLP2", Control::LABEL, ivec2(20, 394), ivec2(90, 24)); subobj->SetText("ObjLP2", "param2:");
	subobj->AddControl("ObjLP3", Control::LABEL, ivec2(20, 424), ivec2(90, 24)); subobj->SetText("ObjLP3", "param3:");
	subobj->AddControl("ObjLP4", Control::LABEL, ivec2(20, 454), ivec2(90, 24)); subobj->SetText("ObjLP4", "param4:");


	subfile->AddControl("Inslabel", Control::LABEL, ivec2(20, 304), ivec2(300, 24)); subfile->SetText("Inslabel", "Files Loaded:");
	subfile->AddControl("FileList",Control::LIST,ivec2(20,324),ivec2(300,246)); 
	//subfile->AddControl("ChangeFolder",Control::BUTTON,ivec2(320,304),ivec2(32,24));subfile->SetText("ChangeFolder","...");
	subfile->AddControl("Reload",Control::BUTTON,ivec2(20,572),ivec2(100,24));subfile->SetText("Reload","Reload");

	cowin->AddControl("edmodetile",Control::RADIO,ivec2(20,275),ivec2(120,25));cowin->SetText("edmodetile","Tiles");
	cowin->AddControl("edmodeobj",Control::RADIO,ivec2(140,275),ivec2(120,25));cowin->SetText("edmodeobj","Objects");
	cowin->AddControl("edmodefile", Control::RADIO, ivec2(260, 275), ivec2(120, 25)); cowin->SetText("edmodefile", "Labels");
	cowin->SetValue("edmodetile",2,0);cowin->SetValue("edmodeobj",2,1);cowin->SetValue("edmodefile",2,2);
	cowin->radioch[0]=0;
	if (!loadedbuffs){
		cbuffers[0]=SDL_CreateTexture(rd,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,768,224);
		cbuffers[1]=SDL_CreateTexture(rd,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,512,256);
		efbs[0].Init(rd,ivec2(768,224));
		efbs[1].Init(rd,ivec2(512,256));
		LoadFile("./base.uob", "base.uob", true);
		basegfx = (OBtileF*) fileoutld;
		SDL_Surface * sr=SDL_LoadBMP("genobj.bmp");
		SDL_Surface * sur=SDL_ConvertSurfaceFormat(sr,SDL_PIXELFORMAT_RGBA8888,0);
		SDL_LockSurface(sur);
		unsigned char * pax=(unsigned char*)sur->pixels;memcpy(stpic,pax,16*16*4);
		SDL_UnlockSurface(sur);
		loadedbuffs = true;
	}
}
int curcourse=0,ocourse=0;
int curobject=-1;



void LVFile::Run(int status){
	ivec2 pos=cowin->pos;
	int mode=cowin->radioch[0];
	bool writehead=false;
	bool notiles = true;
	char tex[100];
	ocourse=curcourse;
	for (int a = 0; a < files.size(); a++) {
		if (files[a]->typ == FileType::FT_TILESET) {notiles = false;}
	}

	if ((lscour->getValue(0)==1 || lscour->getValue(0)>15) && curcourse>0){curcourse--;writehead=true;curobject=-1;}
	if ((fwcour->getValue(0)==1 || fwcour->getValue(0)>15) && curcourse<32){curcourse++;writehead=true;curobject=-1;}
	if (!writehead){
		SaveCourseControls();
	}else{
		LoadCourseControls();
	}
	char text[100];snprintf(text,100,"Course:%i",curcourse);
	courlab->SetText(text);
	RunPalette(ivec2(100,600),cowin);
	cowin->RunWindow();

	ivec2 pps=ivec2(24,24)+pos;
	efbs[0].draw(pps,1.0);
	int scro=coboxes[1]->getValue(0)/10;
	int cc=0,cd=0,cval=0;
	SDL_Rect tsize;

	int hallsz = cds[curcourse].blocks.size()/64;


	char nb[32];
	for (int a=0;a<32;a++){
		if (a<hallsz+1){
			int lips=(a*256)-int(coboxes[1]->getValue(0)/10)*32;
			if (lips>-1 && lips<768){
				Line(pps+ivec2(lips+4,4),pps+ivec2(lips+4,224+4),color(255,0,0));
				if (a<hallsz){snprintf(nb,32,"%i",a);Print(pps+ivec2(lips+7,4),color(255,0,0),nb);}
			}
		}
	}
	int fval = 0;
	tile blank;
	for (int a = 0; a < 64; a++) { blank.pix[a] = 0; }
	if (!notiles && curcourse < 32) {
		efbs[0].clear(color(0));
		for (int a = 0; a < 24; a++) {
			for (int b = 0; b < 7; b++) {
				if (cds[curcourse].blocks.size() > ((a + scro) * 8) + b) {
					cval = cds[curcourse].blocks[((a + scro) * 8) + b]; 
					for (int c = 0; c < 2; c++) {
						for (int d = 0; d < 2; d++) {
							cc = clusters[cval].box[c + (d * 2)];
							cd = groups[cc].box[0]; tiles[cd].draw(&efbs[0],ivec2((a * 32) + (c * 16), (b * 32) + (d * 16)));
							cd = groups[cc].box[1]; tiles[cd].draw(&efbs[0],ivec2((a * 32) + (c * 16) + 8, (b * 32) + (d * 16)));
							cd = groups[cc].box[2]; tiles[cd].draw(&efbs[0],ivec2((a * 32) + (c * 16), (b * 32) + (d * 16) + 8));
							cd = groups[cc].box[3]; tiles[cd].draw(&efbs[0],ivec2((a * 32) + (c * 16) + 8, (b * 32) + (d * 16) + 8));
						}
					}
				}
			
			}
		}
	}

	for (int a = 0; a < cds[curcourse].objects.size(); a++) {
		int owi=16,ohe=16;
		int osel=0;

		if (named){

			int sb = cds[curcourse].objects[a].type;

			//if (acc->objs[sb].siz==1){owi=32;} if (acc->objs[sb].siz==2){ohe=32;} if (acc->objs[sb].siz==3){owi=32;ohe=32;}
			if (programs.size()>0){owi = programs[sb].wid;ohe = programs[sb].hei;}
			ivec2 tpos=ivec2(cds[curcourse].objects[a].x, cds[curcourse].objects[a].y);
			int lips=tpos.x-int(coboxes[1]->getValue(0)/10)*32;
			if (lips > 0 && lips < 768) {
				//snprintf(nb, 32, "%s", acc->objs[sb].name); 
				Print(ivec2(lips, tpos.y - 20) + pps, color(0, 0, 255), nb);
				Contour(ivec2(lips, tpos.y) + pps, ivec2(owi, ohe), color(0, 0, 255));
				
				if (tpos.y > 4 && tpos.y < 224 && lips>4 && lips < 768) {
					if (programs.size()>sb){programs[sb].blit((char*)efbs[0].buff.data(),768,ivec2(lips, tpos.y) - ivec2(0, 0));}
				}
				
				if (a == curobject) { Contour(ivec2(lips - 1, tpos.y - 1) + pps, ivec2(owi + 2, ohe + 2), color(128, 128, 255)); }
			}
		}
	}

	efbs[0].update();
	if (mode==0){
		subblo->RunWindow();
		pps=ivec2(24,304)+pos;efbs[1].draw(pps,1.0);

		if (MouseTest(ivec2(20,300+60),ivec2(512,256)) && mouse_bt>0 && !notiles){
			int mbx=(mouse_x-20)/32,mby=(mouse_y-(300+60))/32;
			curblock=mbx+(mby*16);
			if (curblock>clusters.size()-1){curblock=clusters.size()-1;}
		}
		
		if (MouseTest(ivec2(20,20+60),ivec2(768,224)) && mouse_bt>0 && !notiles){
			int mbx=(mouse_x-20)/32,mby=(mouse_y-(20+60))/32;

			while (cds[curcourse].blocks.size() < ((mbx + scro) * 8) + mby + 1){
				for (int a = 0; a < 64; a++) {
					cds[curcourse].blocks.push_back(0);cds[curcourse].blfiles.push_back(0);
				}
			}
			cds[curcourse].blocks[((mbx + scro) * 8) + mby] = curblock;
			cds[curcourse].blfiles[((mbx + scro) * 8) + mby] = curfileblock;
		}
		//Kludge disgusting way to do it
		int sz = cds[curcourse].blocks.size();
		if (sz>63){
		bool killbl=true;
		for (int a = 0; a <64 ;a++){
			if (cds[curcourse].blocks[(sz-64)+a]>0){killbl=false; break;}
		}
		if (killbl){
			for (int a=0;a<64;a++){cds[curcourse].blocks.pop_back();}
		}
		}

		int tcc = 0;
		if (!notiles) {
			for (int a = 0;a < clusters.size();a++){
				int x = tcc & 15;
				int y = tcc / 16;
				for (int c = 0; c < 2; c++) {
					for (int d = 0; d < 2; d++) {
						cc = clusters[a].box[c + (d * 2)];
						cd = groups[cc].box[0]; tiles[cd].draw(&efbs[1], ivec2((x * 32) + (c * 16), (y * 32) + (d * 16)));
						cd = groups[cc].box[1]; tiles[cd].draw(&efbs[1], ivec2((x * 32) + (c * 16) + 8, (y * 32) + (d * 16)));
						cd = groups[cc].box[2]; tiles[cd].draw(&efbs[1], ivec2((x * 32) + (c * 16), (y * 32) + (d * 16) + 8));
						cd = groups[cc].box[3]; tiles[cd].draw(&efbs[1], ivec2((x * 32) + (c * 16) + 8, (y * 32) + (d * 16) + 8));
					}
				}
				tcc++;
			}
		}

		int px=curblock % 16,py= curblock /16;
		Contour(ivec2(20+4+px*32,300+60+4+py*32),ivec2(32,32),color(255,0,0));
		//tsize.x=0;tsize.y=0;tsize.w=512;tsize.h=256;SDL_UpdateTexture(cbuffers[1],&tsize,cotilesbl,2048);
		efbs[1].update();
			if (subblo->GetValue("AddLink",0)==1){
			char tenu[100];
			if (Courses[curcourse].linkct<16){
				Courses[curcourse].lks[Courses[curcourse].linkct].dir=subblo->GetValue("LinkDirection",0);
				subblo->GetText("LinkSourceScreen",tenu);Courses[curcourse].lks[Courses[curcourse].linkct].sscreen=atoi(tenu);
				subblo->GetText("LinkDestCourse",tenu);Courses[curcourse].lks[Courses[curcourse].linkct].dcourse=atoi(tenu);
				subblo->GetText("LinkDestScreen",tenu);Courses[curcourse].lks[Courses[curcourse].linkct].dscreen=atoi(tenu);
				Courses[curcourse].linkct++;
			}
			UpdateLinkWindow();
		}
		if (subblo->GetValue("RemoveLink",0)==1){
			if (Courses[curcourse].linkct>0){
				int sl=subblo->GetValue("LinkList",0);
				for (int a=sl;a<Courses[curcourse].linkct;a++){
					Courses[curcourse].lks[a]=Courses[curcourse].lks[a+1];
				}
				Courses[curcourse].linkct--;
				UpdateLinkWindow();
			}
		}
	}
	//ObjAdd,ObjRem,ObjType,ObjSubType
	if (mode==1){
		/*
		int fil=-1, subfil = 0;
		int select = subobj->GetValue("ObjType", 0);
		char tex[1000]; tex[0] = 0;
		bool ff = false;
		int ct = 0,selsub=subobj->GetValue("ObjName",0);
		for (int a = 0; a < files.size(); a++) {
			if (files[a]->typ == FileType::FT_OBJECTSET) {
				OBtileF* acc = (OBtileF*)files[a];
				for (int b = 0; b < acc->objct; b++) {
					if (acc->objs[b].type == select) {
						ff = true;
						strcat_s(tex, 900, acc->objs[b].name); strcat_s(tex, 900, "|");
						if (ct == selsub) { fil = a; subfil = b;}
						ct++;
					}
				}
			}
		}
		if (!ff) { strcpy_s(tex,100, "none|"); }
		subobj->SetText("ObjName",tex);
		*/


		int cb=int(coboxes[1]->getValue(0)/10)*32;
		int fil = 0,sfil = 0,typ = subobj->GetValue("ObjType", 0);
		subobj->RunWindow();
		if (subobj->GetValue("ObjAdd",0)==1 && fil>-1){
			/*
			for (int a = 0; a < files.size(); a++) {
				if (files[a]->typ == FileType::FT_OBJECTSET) {
					OBtileF* ob = (OBtileF*) files[a];
					for (int b = 0; b < ob->objct; b++) {
						if (typ < 16) {
							if (strcmp(ob->objs[b].name, obtnames[typ]) == 0) { fil = a; sfil = b; }
						} else {
							//ProgramFile * pf = (ProgramFile*)files[typ];
							//if (strcmp(ob->objs[b].name, pf->nameclip.c_str()) == 0) { fil = a; sfil = b; }
						}
					}
				}
			}
			*/
			cds[curcourse].objects.push_back(object());
			cds[curcourse].objects.back().x = 50 + cb;
			cds[curcourse].objects.back().y = 50;
			cds[curcourse].objects.back().file = fil;
			cds[curcourse].objects.back().fpos = sfil;
			cds[curcourse].objects.back().type = typ;
			cds[curcourse].objects.back().intern = true;
			cds[curcourse].objects.back().name = programs[typ].name;
		}
		if (subobj->GetValue("ObjRem",0)==1){
			if (curobject>-1){
				cds[curcourse].objects.erase(cds[curcourse].objects.begin() + curobject);
			}
			curobject=-1;
		}
	if (curobject>-1){
		if (cds[curcourse].objects[curobject].type!= subobj->GetValue("ObjType",0)){ 
			cds[curcourse].objects[curobject].type = subobj->GetValue("ObjType",0);}
		//if (cds[curcourse].objects[curobject].subtype!=subobj->GetValue("ObjName",0)){ cds[curcourse].objects[curobject].subtype=subobj->GetValue("ObjName",0);}
		if (cds[curcourse].objects[curobject].p1!=subobj->GetTextAsNumber("ObjP1")){cds[curcourse].objects[curobject].p1=subobj->GetTextAsNumber("ObjP1");}
		if (cds[curcourse].objects[curobject].p2!=subobj->GetTextAsNumber("ObjP2")){cds[curcourse].objects[curobject].p2=subobj->GetTextAsNumber("ObjP2");}
		if (cds[curcourse].objects[curobject].p3!=subobj->GetTextAsNumber("ObjP3")){ cds[curcourse].objects[curobject].p3=subobj->GetTextAsNumber("ObjP3");}

	}
	if (mouse_bt==0){heldobj=-1;}
	if (MouseTest(ivec2(20,20+60),ivec2(768,224)) && mouse_bt>0){
		int mtx=mouse_x-20,mty=mouse_y-80,xx,yy,owi=16,ohe=16,osel=0;
		if (heldobj<0){
			curobject=-1;
			for (int a=0;a<cds[curcourse].objects.size();a++){
				xx= cds[curcourse].objects[a].x-cb;yy= cds[curcourse].objects[a].y;
				owi = programs[cds[curcourse].objects[a].type].wid;
				ohe = programs[cds[curcourse].objects[a].type].hei;

				if (mtx>xx && mty>yy && mtx<xx+owi && mty<yy+ohe){heldobj=a;curobject=a;break;}
			}
			if (curobject>-1){
				subobj->SetValue("ObjType",0,cds[curcourse].objects[curobject].type);
				//subobj->SetValue("ObjName",0, cds[curcourse].objects[curobject].subtype);
				subobj->SetNumberAsText("ObjP1", cds[curcourse].objects[curobject].p1);
				subobj->SetNumberAsText("ObjP2", cds[curcourse].objects[curobject].p2);
				subobj->SetNumberAsText("ObjP3", cds[curcourse].objects[curobject].p3);
			}
		}else{
			int px=(mtx-8)+cb,py=mty-8;
			if (subobj->GetValue("ObjGrid",0)>0){
				px=int(px/8)*8;py=int(py/8)*8;
			}
			cds[curcourse].objects[heldobj].x = px;
			cds[curcourse].objects[heldobj].y = py;
		}
	}
	}

	if (mode==2){
		subfile->RunWindow();
		if (!named) {
			subfile->SetText("FileList", "No folder set|Save file in a folder to set|the files used in this level|");
		}
		else {
			if (files.size() < 1) {
				subfile->SetText("FileList", "No files in the folder|");
			}
			else {
				std::string stra;
				for (int a = 0; a < macros.size(); a++) {
					stra += macros[a].name;
					stra += "|";
				}
				char straia[8192];
				strcpy(straia,stra.c_str());
				subfile->SetText("FileList", straia);

			}
		}
	}
	if (outdated && named) {
		outdated = false;
		ReloadFiles();
	}
	
	strcpy(flexmenu,"Course|Import Cur Course|Export Cur Course");
}

void LVFile::ReloadFiles(){
	for (int a = 0; a < files.size(); a++) {
		if (files[a]->typ == FileType::FT_TILESET) { delete static_cast<BGtileF*>(files[a]); }
		if (files[a]->typ == FileType::FT_OBJECTSET) { delete static_cast<OBtileF*>(files[a]); }
	}
	filt[0].label = "Upen Files"; filt[0].all = false;
	filt[0].extens.clear(); filt[0].extens.push_back(".utl"); filt[0].extens.push_back(".uob"); filt[0].extens.push_back(".ulv"); filt[0].extens.push_back(".upr");
	filt[0].extens.push_back(".uis");filt[0].extens.push_back(".uss");
	filtct = 1; 
	files.clear();
	int pos = fpath.rfind('/');
	int opos = fpath.rfind('\\');
	if (opos > pos) { pos = opos; }
	std::string dir = fpath;dir.resize(pos+1);
	char odir[1024];
	strcpy(odir, dir.c_str());
	if (LoadDir(odir, false)) {
		char fairu[2048];
		std::vector <mapel> oldmap = blomap;
		blomap.clear();
		tiles.clear();groups.clear();clusters.clear();
		objgfx.clear(); macros.clear();
		for (int a = 0; a < df.size(); a++) {
			strcpy(fairu, odir); strcat(fairu, "/"); strcat(fairu, df[a].fname);
			if (LoadFile(fairu, df[a].fname,true)) {
				if (fileoutld->typ == FileType::FT_TILESET) { 
					BGtileF * f= (BGtileF*)fileoutld;
					blomap.push_back(mapel());
					blomap.back().name=fileoutld->name;
					blomap.back().itile = tiles.size();blomap.back().etile = tiles.size()+f->tict;
					blomap.back().igro = groups.size();blomap.back().egro = groups.size()+f->grct;
					blomap.back().iclu = clusters.size();blomap.back().eclu = clusters.size()+f->clct;
					for (int b = 0;b < f->tict+1;b++){tiles.push_back(f->tiles[b]);}
					for (int b = 0;b < f->grct+1;b++){
						groups.push_back(f->groups[b]);
						for (int c = 0;c < 4;c++){groups.back().box[c]+=blomap.back().itile;}
						bltypes.push_back(f->bltype[b]);
					}
					for (int b = 0; b < f->clct+1;b++){
						clusters.push_back(f->clusters[b]);
						for (int c = 0;c < 4;c++){clusters.back().box[c]+=blomap.back().igro;}
					}
				}
				if (fileoutld->typ == FileType::FT_OBJECTSET) { 
					OBtileF * f = (OBtileF*)fileoutld;
					for (int b = 0; b < f->objct;b++){
						objgfx.push_back(f->objs[b]);
						std::string mcn = f->objs[b].name;mcn+="_gfx";
						macros.push_back(o_macro(mcn,objgfx.size()+15));
					}
				}					
				if (fileoutld->typ != FileType::FT_LEVEL) { //It wouln't be very smart to load the level itself
					files.push_back(fileoutld);
				}
			}
		}
		remap(oldmap);
		std::vector <int> ttable;
		for (int a = 0; a < fitable.size(); a++) {
			int newid = -1;
			for (int b = 0; b < files.size(); b++) {
				if (files[b]->name == fitable[a].flname) {
					newid = b;
				}
			}
			ttable.push_back(newid);
		}
		for (int a = 0; a < 32; a++) {
			for (int b = 0; b < cds[a].blfiles.size(); b++) {
				if (cds[a].blfiles[b] > -1 && cds[a].blfiles[b]< fitable.size()) { cds[a].blfiles[b] = ttable[cds[a].blfiles[b]]; } //Translate file table
			}
			for (int b = 0; b < cds[a].objects.size(); b++) {
				if (!cds[a].objects[b].intern) { cds[a].objects[b].file = ttable[cds[a].objects[b].file]; }
			}
		}
		fitable.clear();
		for (int a = 0; a < files.size(); a++) { //File translation complete, time to update the table
			fitable.push_back(fileindex());
			fitable.back().flname = files[a]->name;
		}
	}
	fmapt.clear(); fmapf.clear(); 
	for (int a = 0; a < files.size(); a++) {
		if (files[a]->typ == FileType::FT_TILESET) {
			BGtileF* acc = (BGtileF * )files[a];
			for (int b = 0; b < acc->clct+1; b++) {
				fmapt.push_back(b); fmapf.push_back(a);
			}
		}
	}
	char prgs[1024]; prgs[0] = 0;
	programs.clear();

	for (int a = 0; a < files.size(); a++) {
		if (files[a]->typ == FileType::FT_PROGRAMSET) {
			ProgramFile* pr = (ProgramFile*)files[a];
		    std::string naclip=pr->name.substr(0,pr->name.size()-4);
		    strcat(prgs, naclip.c_str());strcat(prgs, "|");
			programs.push_back(prgtpl());
			programs.back().trueid = a + 16;
			programs.back().name = naclip;
			FillIcon(&programs.back());
		}
	}

	//"ultrapen","powerup","set exit","set palette"
	for (int a = 0; a < 16; a++){
		programs.push_back(prgtpl());programs.back().trueid = a;
		if (a < 4){strcat(prgs,obtnames[a]);strcat(prgs,"|"); programs.back().name = obtnames[a];FillIcon(&programs.back());}
	}
	//strcat(prgs, "ultrapen|powerup|set exit|set palette|");
	subobj->SetText("ObjType", prgs);
	//And with this loop, i use the names to keep the program consistency
	for (int a = 0;a < 32; a++){
		for (int b = 0; b < cds[a].objects.size(); b++){
			for (int c = 0; c < programs.size();c++){
				if (programs[c].name == cds[a].objects[b].name){
					cds[a].objects[b].type = c; break; 
				}
			}
		}
	}

}



void LVFile::FillIcon(prgtpl * pr){
	for (int b = 0; b < objgfx.size(); b++){
		if (strcmp(objgfx[b].name,pr->name.c_str()) == 0){
			if (objgfx[b].siz == 0){pr->wid = 16; pr->hei = 16;}
			if (objgfx[b].siz == 1){pr->wid = 32; pr->hei = 16;}
			if (objgfx[b].siz == 2){pr->wid = 16; pr->hei = 32;}
			if (objgfx[b].siz == 3){pr->wid = 32; pr->hei = 32;}
			objgfx[b].objtls[0].drawlc(pr->pix,64,ivec2(0,0));
			int ct = 0;
			for (int y = 0; y < pr->hei; y+=16){
				for (int x = 0; x < pr->wid; x+=16){
					objgfx[b].objtls[objgfx[b].frames[0][ct++]].drawlc(pr->pix,pr->wid,ivec2(x,y));
				}
			}
			return;
		}
	}
	OBtileF * obf = (OBtileF*)basegfx;
	for (int b = 0; b < obf->objct; b++){
		if (strcmp(obf->objs[b].name,pr->name.c_str()) == 0){
			if (obf->objs[b].siz == 0){pr->wid = 16; pr->hei = 16;}
			if (obf->objs[b].siz == 1){pr->wid = 32; pr->hei = 16;}
			if (obf->objs[b].siz == 2){pr->wid = 16; pr->hei = 32;}
			if (obf->objs[b].siz == 3){pr->wid = 32; pr->hei = 32;}
			int ct = 0;
			for (int y = 0; y < pr->hei; y+=16){
				for (int x = 0; x < pr->wid; x+=16){
					obf->objs[b].objtls[obf->objs[b].frames[0][ct++]].drawlc(pr->pix,pr->wid,ivec2(x,y));
				}
			}
			return;
		}
	}

}


void LoadCourseControls(){
/*
	char tex[100];
	snprintf(tex,100,"%i",Courses[curcourse].numscreens);
	gtexbox[0]->SetText(tex);
	*/
	UpdateLinkWindow();
}
void SaveCourseControls(){
	char tex[100];
//	gtexbox[0]->GetText(tex);
//	Courses[ocourse].numscreens=atoi(tex);
}

void UpdateLinkWindow(){
}

int LVFile::Encode(unsigned char* data) {
	DH dat; dat.data = data; dat.pos = 0;
	int cdct = 0;
	//std::vector <mapel> lmap;
	dat.wshort(blomap.size());
	for (int a = 0;a < blomap.size();a++){
		for (int b = 0; b < 32; b++) {
			if (b<blomap[a].name.size()){
				dat.wbyte(blomap[a].name.c_str()[b]);
			}else{
				dat.wbyte(0);
			}
		}
		dat.wshort(blomap[a].iclu);dat.wshort(blomap[a].eclu); //This is only used to remap the tiles themselves, so no need for tile/group maps
	}

	for (int a = 0; a < 32; a++) {
		dat.wshort(cds[a].blocks.size());
		for (int b = 0; b < cds[a].blocks.size(); b++) {dat.wshort(cds[a].blocks[b]);dat.wshort(cds[a].blfiles[b]);}
		dat.wshort(cds[a].exitside);dat.wshort(cds[a].linkct);
		for (int b = 0; b < cds[a].linkct; b++) {
			dat.wshort(cds[a].lks[b].dcourse);dat.wshort(cds[a].lks[b].dir);dat.wshort(cds[a].lks[b].dscreen);dat.wshort(cds[a].lks[b].sscreen);
		}
		dat.wshort(cds[a].objects.size());
		for (int b = 0; b < cds[a].objects.size(); b++) {
			for (int c =0 ;c < 16; c++){
				if (c < cds[a].objects[b].name.size()){
					dat.wbyte(cds[a].objects[b].name[c]);
				}else{
					dat.wbyte(0);
				}
			}
			dat.wshort(cds[a].objects[b].type);dat.wshort(cds[a].objects[b].subtype);
			dat.wshort(cds[a].objects[b].file);dat.wshort(cds[a].objects[b].fpos);
			dat.wshort(cds[a].objects[b].x);dat.wshort(cds[a].objects[b].y);
			dat.wshort(cds[a].objects[b].p1);dat.wshort(cds[a].objects[b].p2);
			dat.wshort(cds[a].objects[b].p3);dat.wshort(cds[a].objects[b].p4);
			dat.wshort(cds[a].objects[b].intern); 
		}
	}
	dat.wbyte(1); //Number of extensions
	for (int a = 0; a < 16; a++){
		for (int b = 0; b < 16; b++){
			dat.wbyte(pals[a].col[b].r);
			dat.wbyte(pals[a].col[b].g);
			dat.wbyte(pals[a].col[b].b);
			dat.wbyte(pals[a].col[b].a);
		}
	}

	return dat.pos;
}
void LVFile::Decode(unsigned char* data) {
	DH dat; dat.data = data; dat.pos = 0;
	int cdct = 0;
	char texto[64];
	std::vector <mapel> lmap;
	lmap.resize(dat.rshort());
	for (int a = 0;a < lmap.size();a++){
		for (int b = 0; b < 32; b++) {texto[b] = dat.rbyte();}
		texto[32] = 0;lmap[a].name = texto;
		lmap[a].iclu = dat.rshort(); lmap[a].eclu = dat.rshort();
	}
	for (int a = 0; a < 32; a++) {
		int csize = dat.rshort();
		cds[a].blocks.resize(csize);
		cds[a].blfiles.resize(csize);
		for (int b = 0; b < cds[a].blocks.size(); b++) { cds[a].blocks[b] = dat.rshort(); cds[a].blfiles[b] = dat.rshort(); }
		cds[a].exitside = dat.rshort(); cds[a].linkct = dat.rshort();
		for (int b = 0; b < cds[a].linkct; b++) {
			cds[a].lks[b].dcourse = dat.rshort(); cds[a].lks[b].dir = dat.rshort(); cds[a].lks[b].dscreen = dat.rshort(); cds[a].lks[b].sscreen = dat.rshort();
		}
		cds[a].objects.resize(dat.rshort());
		char nm[20];
		for (int b = 0; b < cds[a].objects.size(); b++) {
			for (int c = 0; c < 16 ; c++){nm[c] = dat.rbyte();}
			cds[a].objects[b].name=nm;
			cds[a].objects[b].type = dat.rshort(); cds[a].objects[b].subtype = dat.rshort();
			cds[a].objects[b].file = dat.rshort(); cds[a].objects[b].fpos = dat.rshort();
			cds[a].objects[b].x = dat.rshort(); cds[a].objects[b].y = dat.rshort();
			cds[a].objects[b].p1 = dat.rshort(); cds[a].objects[b].p2 = dat.rshort();
			cds[a].objects[b].p3 = dat.rshort(); cds[a].objects[b].p4 = dat.rshort();
			cds[a].objects[b].intern = dat.rshort();
		}
	}
	remap(lmap);
	int ext = dat.rbyte();
	if (ext > 0){
	for (int a = 0; a < 16; a++){
		for (int b = 0; b < 16; b++){
			//pals[a].col[b].r = dat.rbyte();
			//pals[a].col[b].g = dat.rbyte();
			//pals[a].col[b].b = dat.rbyte();
			//pals[a].col[b].a = dat.rbyte();
		}
	}

	}
}

void LVFile::remap(std::vector<mapel> oblo){
	for (int a = 0;a < oblo.size();a++){
		for (int b = 0; b < blomap.size();b++){
			if (oblo[a].name == blomap[b].name){
				for (int c = 0;c < 32;c++){
					for (int d = 0;d < cds[c].blocks.size();d++){
						int vl = cds[c].blocks[d];
						if (vl >= oblo[b].iclu && vl < oblo[b].etile){
							vl = (vl - oblo[b].iclu)+ blomap[a].iclu; 
							cds[c].blocks[d] = vl;
						}
					}
				}
			}
		}
	}
}
