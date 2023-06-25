#include "data.h"

bgtile bgtiles[256];
int blmaps[256][4],clmaps[256][4];
int blpar[256];
Coursehd Courses[32];
palette pals[16];
UObject uobs[128];
int numcor=0;
char flexmenu[100];
int flexmenuout=-1;
OBtileF* basegfx;

std::vector <FileType*> fils;
int curfile=0;

int bmfilemode=0,bmwi=0,bmhe=0,bmbytes=0;
unsigned char bitmem[4000000];
bool bmdone=false;

void FileType::Boot(){

}
void FileType::Run(int status){
	//0 normal run, 1 switching out, 2 switching in
} 

void DecodeLevel(unsigned char * data, int old){
	int addr=32; //Skip the memory addresses that are only useful for the MSX side
	int mul;
    for (int a=0;a<256;a++){
		for (int b=0;b<8;b++){
            bgtiles[a].plines[b][0]=data[addr+2048] & 15;
            bgtiles[a].plines[b][1]=(data[addr+2048]/16) & 15;
            mul=1;
            for (int c=7;c>-1;c--){
				if ((data[addr] & mul)>0){bgtiles[a].tmap[c+(b*8)]=1;}else{bgtiles[a].tmap[c+(b*8)]=0;}
				mul*=2;
            }
			addr++;
		}
    }
    addr+=2048;
	for (int a=0;a<128;a++){for (int b=0;b<4;b++){blmaps[a][b]=data[addr++];}}
	for (int a=0;a<128;a++){for (int b=0;b<4;b++){clmaps[a][b]=data[addr++];}}
	for (int a=0;a<128;a++){blpar[a]=data[addr++];}
	numcor=data[addr++];
	for (int a=0;a<numcor;a++){
		addr+=2;//Skip address as this is just for the MSX
		Courses[a].numscreens=data[addr++];
		Courses[a].exitside=data[addr++];
		addr+=4;
	}
	for (int a=0;a<numcor;a++){
		Courses[a].course=new unsigned char[200000];
		memset(Courses[a].course,0,200000);
		int cc=0;
		for (int c=0;c<Courses[a].numscreens;c++){
			for (int b=0;b<64;b++){
				Courses[a].course[cc++]=data[addr++];
			}
		}
	}
	int v1=0,v2=0;
	for (int a=0;a<8;a++){
		for (int b=0;b<16;b++){
			v1=data[addr++];v2=data[addr++];
			pals[a].col[b].r=((v1/16) & 15)*36;
			pals[a].col[b].g=(v2 & 15)*36;
			pals[a].col[b].b=(v1 & 15)*36;
		}
	}
    int spritepointer=data[addr++]+(data[addr++]*256);
    int spritecount=data[addr++];
	int w,h;
	int framect=0;
	int nbtiles=0;
	addr+=spritecount*2; //Just skip the sprite pointers
	int siztb[]={1,2,2,4};
	for (int a=0;a<spritecount;a++){
		uobs[a].type=data[addr++];
		uobs[a].subtype=data[addr++];
		if (old==1){
			w=data[addr++];h=data[addr++];
			if (w==1 && h==1){uobs[a].siz=0;} //Converting from the old, very flexible format to the new, boring (but lighter on the CPU) one
			if (w==2 && h==1){uobs[a].siz=1;}
			if (w==1 && h==2){uobs[a].siz=2;}
			if (w==2 && h==2){uobs[a].siz=3;}
			framect=data[addr++];
			uobs[a].framect=framect;
			uobs[a].multic=1-data[addr++];
		}else{
			uobs[a].siz=data[addr] & 3;
			uobs[a].multic=(data[addr++]&4)/4;addr++;
			framect=data[addr++];
			uobs[a].framect=framect;
			addr++; //Skipping another dead byte
		}

		nbtiles=data[addr++];
		for (int b=0;b<framect;b++){
			for (int c=0;c<siztb[uobs[a].siz];c++){
				uobs[a].frames[b][c]=data[addr++];
			}
		}
		if (uobs[a].multic==0){
			for (int b=0;b<nbtiles;b++){
				for (int c=0;c<16;c++){
					mul=128;
					uobs[a].sptiles[b].plines[c][0]=(data[addr+c+32]) & 15;
					for (int d=0;d<8;d++){
						if ((mul & data[addr+c])>0){uobs[a].sptiles[b].tmap[d+(c*16)]=1;}else{uobs[a].sptiles[b].tmap[d+(c*16)]=0;}
						if ((mul & data[addr+c+16])>0){uobs[a].sptiles[b].tmap[d+8+(c*16)]=1;}else{uobs[a].sptiles[b].tmap[d+8+(c*16)]=0;}
						mul/=2;
					}
				}
				addr+=48;
			}
		}else{
			for (int b=0;b<nbtiles;b++){
				for (int c=0;c<16;c++){
					mul=128;
					uobs[a].sptiles[b].plines[c][0]=(data[addr+c+64]) & 15;
					uobs[a].sptiles[b].plines[c][1]=(data[addr+c+64+16]) & 15;
					uobs[a].sptiles[b].plines[c][2]=uobs[a].sptiles[b].plines[c][0]|uobs[a].sptiles[b].plines[c][1];

					for (int d=0;d<8;d++){
						if ((mul & data[addr+c])>0){uobs[a].sptiles[b].tmap[d+(c*16)]=1;}else{uobs[a].sptiles[b].tmap[d+(c*16)]=0;}
						if ((mul & data[addr+c+16])>0){uobs[a].sptiles[b].tmap[d+8+(c*16)]=1;}else{uobs[a].sptiles[b].tmap[d+8+(c*16)]=0;}
						if ((mul & data[addr+c+32])>0){uobs[a].sptiles[b].tmap[d+(c*16)]|=2;}else{uobs[a].sptiles[b].tmap[d+(c*16)]|=0;}
						if ((mul & data[addr+c+48])>0){uobs[a].sptiles[b].tmap[d+8+(c*16)]|=2;}else{uobs[a].sptiles[b].tmap[d+8+(c*16)]|=0;}
						mul/=2;
					}
				}
				addr+=96;
			}
		}
	}

	for (int a=0;a<32;a++){
		if (Courses[a].numscreens>0){
            Courses[a].linkct=data[addr++];
            for (int b=0;b<Courses[a].linkct;b++){
                Courses[a].lks[b].dir=data[addr++];
                Courses[a].lks[b].sscreen=data[addr++];
                Courses[a].lks[b].dcourse=data[addr++];
                Courses[a].lks[b].dscreen=data[addr++];
            }
			Courses[a].objct=data[addr++];
			if (Courses[a].objct>192){Courses[a].objct=0;} //Fail
            for (int b=0;b<Courses[a].objct;b++){
                Courses[a].obj[b].type=data[addr++];
                Courses[a].obj[b].subtype=data[addr++];
                Courses[a].obj[b].x=(data[addr++])+(data[addr++]*256);
                Courses[a].obj[b].y=data[addr++];
                Courses[a].obj[b].p1=data[addr++];
                Courses[a].obj[b].p2=data[addr++];
                Courses[a].obj[b].p3=data[addr++];
            }
		}
	}


}


int Encodelevel(unsigned char * data,int old){
	int addrs[16],addc=0;
	int hallct=0,screenct=0;
	int addr=0;
	int siztb[]={1,2,2,4};
	int spripts[128],spsize=0,spcount=0,det=0,maxtile=0;
	int mul=0,vl=0;
	addr=0x20;
	addrs[0]=addr; //Color Block address
	for (int a=0;a<256;a++){
		for (int c=0;c<8;c++){
			mul=1;vl=0;
			data[addr+2048]=(bgtiles[a].plines[c][0]&15)+(bgtiles[a].plines[c][1]&15)*16;

			for (int b=7;b>-1;b--){
				vl+=(bgtiles[a].tmap[b+(c*8)] &1)*mul;mul*=2;
			}
            data[addr++]=vl;
		}
	}
	addr+=2048;
	addrs[1]=addr; //Blockmap pointer

	for (int a=0;a<128;a++){for (int b=0;b<4;b++){data[addr++]=blmaps[a][b];}}
	for (int a=0;a<128;a++){for (int b=0;b<4;b++){data[addr++]=clmaps[a][b];}}
	addrs[5]=addr; //Collision mask pointer

	for (int a=0;a<128;a++){data[addr++]=blpar[a];}
	addrs[2]=addr; //Hall pointers

	for (int a = 0; a < 32; a++) { if (Courses[a].numscreens > 0) { hallct++; screenct += Courses[a].numscreens; } }

	data[addr++]=hallct;
	int caddr=addr+hallct*8;
	int courpt[64];
	int courct=0;
    for (int a=0;a<32;a++){
		if (Courses[a].numscreens>0){
			data[addr++]=caddr & 255;data[addr++]=(caddr/256) & 255;
			data[addr++]=Courses[a].numscreens;data[addr++]=Courses[a].exitside;
			courpt[courct++]=addr;
			addr+=4;
			caddr+=Courses[a].numscreens*64;
		}
    }
    for (int a=0;a<32;a++){
		if (Courses[a].numscreens>0){
			for (int b=0;b<Courses[a].numscreens*64;b++){
                data[addr++]=Courses[a].course[b];
			}
		}
    }

	addrs[3]=addr;//Palette pointer
	for (int a=0;a<8;a++){
		for (int b=0;b<16;b++){
			data[addr++]=(((pals[a].col[b].r/36)&15)*16)+((pals[a].col[b].b/36)&15);data[addr++]=((pals[a].col[b].g/36)&15);
		}
	}

	addrs[4]=addr;//Sprite data
    int spsizept=addr;
    addr+=2;
    for (int a=0;a<128;a++){if (uobs[a].framect>0){spcount++;}}
	data[addr++]=spcount;
	int sppointerpt=addr;
	addr+=(spcount*2);
	int inipt=addr;
	spcount=0;
    for (int a=0;a<128;a++){
        if (uobs[a].framect>0){
			spripts[spcount++]=addr-inipt;
			data[addr++]=uobs[a].type;data[addr++]=uobs[a].subtype;
			if (old==0){
				data[addr++]=uobs[a].siz+(uobs[a].multic*4);
				data[addr++]=1;//Was height
			}else{
				if (uobs[a].siz==0){data[addr++]=1;data[addr++]=1;}
				if (uobs[a].siz==1){data[addr++]=2;data[addr++]=1;}
				if (uobs[a].siz==2){data[addr++]=1;data[addr++]=2;}
				if (uobs[a].siz==3){data[addr++]=2;data[addr++]=2;}
			}
			data[addr++]=uobs[a].framect;data[addr++]=1-uobs[a].multic;

			for (int b=63;b>-1;b--){
				det=0;for (int c=0;c<256;c++){det+=uobs[a].sptiles[b].tmap[c];}
				if (det!=0){maxtile=b+1;break;}
			}
			data[addr++]=maxtile;
			for (int c=0;c<uobs[a].framect;c++){
				for (int b=0;b<siztb[uobs[a].siz];b++){data[addr++]=uobs[a].frames[c][b];}
			}

			if (uobs[a].multic==0){
				for (int b=0;b<maxtile;b++){
					for (int c=0;c<16;c++){
						mul=128;
						data[addr+c+32]=uobs[a].sptiles[b].plines[c][0] & 15;
						data[addr+c]=0;data[addr+c+16]=0;
						for (int d=0;d<8;d++){
							if (uobs[a].sptiles[b].tmap[d+(c*16)]>0){data[addr+c]|=mul;}
							if (uobs[a].sptiles[b].tmap[d+8+(c*16)]>0){data[addr+c+16]|=mul;}
							mul/=2;
						}
					}
					addr+=48;
				}
			}else{
				int isoverlap=0;
				for (int b=0;b<maxtile;b++){
					for (int c=0;c<16;c++){
						mul=128;
						//Do not forget to encode the "is overlapping" byte
						data[addr+c]=0;data[addr+c+16]=0;data[addr+c+32]=0;data[addr+c+48]=0;
						for (int d=0;d<8;d++){
							if ((uobs[a].sptiles[b].tmap[d+(c*16)]&1)>0){data[addr+c]|=mul;}
							if ((uobs[a].sptiles[b].tmap[d+8+(c*16)]&1)>0){data[addr+c+16]|=mul;}
							if ((uobs[a].sptiles[b].tmap[d+(c*16)]&2)>0){data[addr+c+32]|=mul;}
							if ((uobs[a].sptiles[b].tmap[d+8+(c*16)]&2)>0){data[addr+c+48]|=mul;}
							mul/=2;
						}
						isoverlap=0;
						for (int d=0;d<16;d++){if (uobs[a].sptiles[b].tmap[d+(c*16)]==3){isoverlap=64;}}
						data[addr+c+64]=uobs[a].sptiles[b].plines[c][0] & 15;
						data[addr+c+64+16]=(uobs[a].sptiles[b].plines[c][1] & 15)+isoverlap;

					}
					addr+=96;
				}
			}
        }
    }
	data[spsizept]=(addr-inipt) & 255;
	data[spsizept+1]=((addr-inipt)/256) & 255;
	int spricalc;
	for (int a=0;a<spcount;a++){
		spricalc=spripts[a]+(spcount*2);
		data[(a*2)+sppointerpt]=spricalc & 255;
        data[(a*2)+sppointerpt+1]=(spricalc/256) & 255;
	}
	//Next session should encode the corridor TRUE data
	//1 byte, number of exits of the corridor
	//per exit: direction,entry screen, exit course, exit screen
	//1 byte, number of objects of the corridor
	//per object: type, subtype, x. y. par1, par2
	addrs[6]=addr;//New stuff
	courct=0;
	for (int a=0;a<32;a++){
		if (Courses[a].numscreens>0){
			data[courpt[courct]]=addr & 255; data[courpt[courct++]+1]=(addr/256) & 255; //Save the address on the table
            data[addr++]=Courses[a].linkct;
            for (int b=0;b<Courses[a].linkct;b++){
                data[addr++]=Courses[a].lks[b].dir;
                data[addr++]=Courses[a].lks[b].sscreen;
                data[addr++]=Courses[a].lks[b].dcourse;
                data[addr++]=Courses[a].lks[b].dscreen;
            }
			if (Courses[a].objct<0){Courses[a].objct=0;} //Heh
			data[addr++]=Courses[a].objct;
            for (int b=0;b<Courses[a].objct;b++){
                data[addr++]=Courses[a].obj[b].type;
                data[addr++]=Courses[a].obj[b].subtype;
                data[addr++]=Courses[a].obj[b].x&255;
                data[addr++]=Courses[a].obj[b].x/256;
                data[addr++]=Courses[a].obj[b].y;
                data[addr++]=Courses[a].obj[b].p1;
                data[addr++]=Courses[a].obj[b].p2;
                data[addr++]=Courses[a].obj[b].p3;
            }
		}
	}

	for (int a=0;a<16;a++){data[(a*2)]=addrs[a]&255;data[(a*2)+1]=(addrs[a]/256)&255;}

	return addr;
}
bool stcmp(char * st1,char * st2,int siz){
	bool ret=true;
	for (int a=0;a<siz;a++){
		if ((st1[a]&223)!=(st2[a]&223)){ret=false;break;}
	}
	return ret;
}


int FileType::Encode(unsigned char * data){
	printf("Encoding nothing!");
	return 0;
}
void FileType::Decode(unsigned char * data){
//Empty
	printf("Decoding nothing!");
}
FileType* fileoutld;

bool LoadFile(const char* file,const char* filemini,bool nolevel){return LoadFile((char*)file,(char*)filemini,nolevel);}

void ucaseit(char * text){
	for (int a = 0;a < strlen(text);a++){
		if (text[a] > 96 && text[a] < 123){text[a] = text[a] & 223;}
	}
}
bool LoadFile(char* file,char * filemini, bool nolevel) {
	FILE* of;
	bool filevalid = false;
	of = fopen(file, "rb");
	if (of) {
		fseek(of, 0, SEEK_END); int siz = ftell(of); fseek(of, 0, SEEK_SET);
		bool program = false;
		char ext[3];
		strcpy(ext,&filemini[strlen(filemini)-3]);ucaseit(ext);
		if (strcmp(ext,"UPR")==0){program = true;}
		unsigned char* tbuff = new unsigned char[siz + 10];
		
		char upte[5];
		if (!program){fread(upte, 1, 4, of);}
		upte[4] = 0;
		if (strcmp(upte, "upen") == 0 || program) {
			int ftyp = 0;
			if (!program){fread(&ftyp, 1, 4, of);}
			fileoutld = new FileType;
			fread(tbuff, 1, siz, of);
			if (program){ftyp=FileType::FT_PROGRAMSET;}
			bool acfil = false;
			if (ftyp == FileType::FT_TILESET) { delete fileoutld; fileoutld = new BGtileF; fileoutld->Decode(tbuff); acfil = true; }
			if (ftyp == FileType::FT_OBJECTSET) { delete fileoutld; fileoutld = new OBtileF; fileoutld->Decode(tbuff); acfil = true; }
			if (ftyp == FileType::FT_LEVEL && !nolevel) { delete fileoutld; fileoutld = new LVFile; fileoutld->Decode(tbuff); acfil = true; }
			if (ftyp == FileType::FT_PROGRAMSET) { delete fileoutld; fileoutld = new ProgramFile; fileoutld->Decode(tbuff); acfil = true; }
			if (ftyp == FileType::FT_MUSICSET) { delete fileoutld; fileoutld = new MusicSet; fileoutld->Decode(tbuff); acfil = true; }
			if (ftyp == FileType::FT_INSTRUMENTSET) { delete fileoutld; fileoutld = new InstrumentSet; fileoutld->Decode(tbuff); acfil = true; }

			if (acfil) {
				fileoutld->name = filemini;
				fileoutld->typ = (FileType::ftt)ftyp;
				fileoutld->named = true;
				fileoutld->fpath = file;
				//fils.push_back(newfil);
				//curfile = fils.size() - 1;
				//if (ftyp == FileType::FT_OBJECTSET) { LoadObjControls(); }
				filevalid = true;
			}
		}
		char fai[4];
		strcpy(fai, &file[strlen(file) - 3]);
		for (int a = 0; a < 3; a++) { fai[a] |= 32; }
		if (strcmp(fai, "prg") == 0) {
			char ficut[64];
			strcpy(ficut, filemini); ficut[strlen(filemini) - 4] = 0;
			fileoutld = new ProgramFile;
			fileoutld->name = filemini;
			fileoutld->typ = (FileType::FT_PROGRAMSET);
			fileoutld->named = true;
			fileoutld->fpath = file;
			ProgramFile* pr = (ProgramFile*)fileoutld;
			char* dbuff = new char[siz + 10];
			fseek(of, 0, SEEK_SET); fread(dbuff, 1, siz, of);
			dbuff[siz] = 0;
			pr->prg.push_back(dbuff);
			pr->nameclip = ficut;
			delete[] dbuff;
			filevalid = true;
		}

		fclose(of);
		delete[] tbuff;
	}
	return filevalid;
}

int getcol(color cin) {
	int dis = 100000, sel = 0, di2 = 0;
	for (int b = 0; b < 16; b++) {
		di2 = abs(pals[0].col[b].r - cin.r);
		di2 += abs(pals[0].col[b].g - cin.g);
		di2 += abs(pals[0].col[b].b - cin.b);
		if (di2 < dis) { dis = di2; sel = b; }
	}
	return sel;
}

void ImportOldDataFile() {
	unsigned short addrs[16];
	unsigned char colors[512];
	color dpal[16];
	FILE* of;
	of = fopen("level.upn", "rb");
	int remap[32];
	if (of) {
		fread(addrs, 2, 16, of);
		fseek(of, addrs[3], SEEK_SET);
		fread(colors, 2, 16 * 8, of);
		for (int a = 0; a < 32; a+=2) {
			dpal[a/2].r =(((colors[a + 0] / 16) & 15) * 36);
			dpal[a/2].g = ((colors[a + 1] & 15) * 36);
			dpal[a/2].b = ((colors[a + 0] & 15) * 36);
		}

		fseek(of, addrs[4]+2, SEEK_SET);
		unsigned char spcount = 0,data = 0;
		fread(&spcount, 1, 1, of);
		fseek(of, addrs[4] + 3 + (spcount*2), SEEK_SET);
		fils.push_back(new OBtileF());
		OBtileF* moo = (OBtileF*)fils.back();
		moo->objct = spcount;
		moo->typ = FileType::FT_OBJECTSET;
		int tb[] = { 1, 2, 2, 4 };
		for (int a = 0; a < spcount; a++) {
			for (int b = 0; b < 512; b++) {
				for (int c = 0; c < 256; c++) {
					moo->objs[a].objtls[b].pix[c] = 0;
				}
			}
			int siz = 0;
			sprintf(moo->objs[a].name, "Import %i", a);
			fread(&data, 1, 1, of);moo->objs[a].type = data;
			fread(&data, 1, 1, of); moo->objs[a].subtype = data;
			fread(&siz, 1, 1, of); moo->objs[a].siz = siz & 3;moo->objs[a].multic =(siz & 4)/4;
			fread(&data, 1, 1, of); //Garbage presumably
			fread(&data, 1, 1, of); moo->objs[a].framect = data; fread(&data, 1, 1, of); //More garbage
			fread(&data, 1, 1, of); moo->objs[a].tilct = data;
			for (int b = 0; b < moo->objs[a].framect; b++) {
				for (int c = 0; c < tb[siz & 3]; c++) {
					fread(&data,  1, 1, of); moo->objs[a].frames[b][c] = data;
				}
			}
			unsigned char tiledata[100];
			for (int b = 0; b < moo->objs[a].tilct; b++) {
				if (moo->objs[a].multic == 0) {
					fread(&tiledata, 48, 1, of);
					for (int c = 0; c < 32; c++) {
						int mul = 128,y = c & 15;
						int c1 = tiledata[32 + y] & 15;
						for (int d = 0; d < 8; d++) {
							int x = d + ((c & 16) / 2),cc = 0;
							if (tiledata[c] & mul) { cc = getcol(dpal[c1]) + 1; }
							moo->objs[a].objtls[b].pix[x + (y * 16)] = cc;
							mul /= 2;
						}
					}
				} else {
					fread(&tiledata, 96, 1, of);
					for (int c = 0; c < 32; c++) {
						int mul = 128, y = c & 15;
						int cl[3];
						cl[0] = tiledata[64 + y] & 15;
						cl[1] = tiledata[64 + 16 + y] & 15;
						cl[2] = cl[0] | cl[1];
						for (int d = 0; d < 8; d++) {
							int x = d + ((c & 16) / 2), cc = 0;
							if (tiledata[c] & mul) { cc = 1; }
							if (tiledata[c+32] & mul) { cc |= 2; }
							if (cc > 0) { cc = getcol(dpal[cl[cc-1]]) + 1; }
							moo->objs[a].objtls[b].pix[x + (y * 16)] = cc;
							mul /= 2;
						}
					}

				}
			}
		}

		fclose(of);
	}
}

std::string get_current_dir() {
   char buff[FILENAME_MAX]; //create string buffer to hold path
   GetCurrentDir( buff, FILENAME_MAX );
   std::string current_working_dir(buff);
   return current_working_dir;
}

std::string ucaser(std::string in){
	std::string out;
	int nu=0;
	for (int a =0; a < in.size();a++){
		nu = in[a];
		if (nu > 96 && nu < 123){nu = nu & 223;}
		out+=nu;
	}
	return out;
}