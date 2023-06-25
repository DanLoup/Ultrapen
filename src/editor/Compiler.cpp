#include "compiler.h"
int revmap[256][256];

int copyit=0;
std::string copyto;

std::string Compile() {
	std::string debtx = "Compiling|";
	int addrs[16];
	LVFile* cts = (LVFile*)fils[curfile];
	cts->ReloadFiles(); //This allows me to change the files outside of the editor and build and still get the most current one
	//Basically just for the asm stuff
	std::string fina;
	DH lk,lk2,lk3;
	lk.data = new unsigned char[1000000];
	memset(lk.data, 0, 100000);
	memset(revmap,0,65535*4);
	lk2.data = lk.data;	lk3.data = lk.data;
	lk.pos = 32; //Skip pointers for now
	//Level tile pointer
	int tistack = 0, clstack = 0,grstack = 0,tpstack = 0, otistack = 0, oclstack = 0, ogrstack = 0, otpstack = 0;
	int tilstack = 0, revstack = 0;
	int c1, c2,mul,vout=0;
	addrs[0] = lk.pos;
	otistack = tistack; oclstack = clstack; ogrstack = grstack; otpstack = tpstack;
	debtx += "Building tileset"; debtx += "|";
	for (int b = 0; b < cts->tiles.size(); b++) {
		bool toocol = false;
		for (int d = 0; d < 8; d++) {
			c1 = cts->tiles[b].pix[d*8]; c2 = c1;
			for (int c = 0; c < 8; c++) {
				if (c1 != cts->tiles[b].pix[(d * 8) + c]) { c2 = cts->tiles[b].pix[(d * 8) + c]; }
				if (c1 != cts->tiles[b].pix[(d * 8) + c] && c2 != cts->tiles[b].pix[(d * 8) + c]) {toocol = true;}
			}
			lk.data[0x820 + tilstack] = (c1 & 15) + ((c2 & 15) * 16);
			vout = 0;mul = 128;
			for (int c = 0; c < 8; c++) {
				if (cts->tiles[b].pix[(d * 8) + c] == c2) { vout += mul; }
				mul /= 2;
			}
			lk.data[0x20 + tilstack] = vout;
			tilstack++;
		}
		if (toocol){debtx += "Warning: tile "; debtx += b; debtx += " has too many colors! |";}
		tistack++;
		if (tistack > 255) { debtx += "Error: more than 256 tiles"; delete[] lk.data; return debtx;} 
	}
	
	for (int b = 0; b < cts->groups.size(); b++) { for (int c = 0; c < 4; c++) { lk.data[0x1020 + grstack] = cts->groups[b].box[c] + otistack; grstack++; } }
	for (int b = 0; b < cts->clusters.size(); b++) { for (int c = 0; c < 4; c++) { lk.data[0x1220 + clstack] = cts->clusters[b].box[c] + ogrstack/4; clstack++;}}
	for (int b = 0; b < cts->groups.size(); b++) { lk.data[0x1420 + tpstack] = cts->bltypes[b]; tpstack++; }
		

	addrs[1] = 0x1020;addrs[2] = 0x14A0; lk.pos = 0x14A0;
	debtx += "Writing palettes|";
	for (int a = 0; a < 8; a++) {
		for (int b = 0; b < 16; b++) {
			lk.wbyte((((pals[a].col[b].r / 36) & 15) * 16) + ((pals[a].col[b].b / 36) & 15)); lk.wbyte((pals[a].col[b].g / 36) & 15);
		}
	}

	debtx += "Writing halls|";
	int hallct = 0;
	for (int a = 31; a > -1; a--) {
		if (cts->cds[a].blocks.size() > 0) {hallct = a+1; break;}
	}
	addrs[3] = lk.pos;
	lk.wbyte(hallct);
	int ptt = lk.pos;
	int addr1[256], addr2[256];
	lk.pos += hallct * 8;
	for (int a = 0; a < hallct; a++) {
		addr1[a] = lk.pos;
		int map;
		for (int b = 0; b < (cts->cds[a].blocks.size() & 65472); b++) {
			map = 0;
			if (cts->cds[a].blfiles[b] < 256) { map = cts->cds[a].blocks[b]; }
			lk.wbyte(map);
		}
	}
	//	lk.data[ptt] = lk.pos & 255; lk.data[ptt+1] = (lk.pos/256) & 255;
	for (int a = 0; a < hallct; a++) {
		addr2[a] = lk.pos;
		for (int b = 0; b < cts->cds[a].objects.size(); b++) {
			lk.wbyte(cts->programs[cts->cds[a].objects[b].type].trueid);
			lk.wshort(cts->cds[a].objects[b].x);
			lk.wbyte(cts->cds[a].objects[b].y);
			lk.wbyte(cts->cds[a].objects[b].p1);lk.wbyte(cts->cds[a].objects[b].p2);
			lk.wbyte(cts->cds[a].objects[b].p3);lk.wbyte(cts->cds[a].objects[b].p4);
		}
	}
	for (int a = 0; a < hallct; a++) {
		lk.data[ptt++]=(cts->cds[a].blocks.size() / 64);
		lk.data[ptt++] = addr1[a] & 255;lk.data[ptt++] = (addr1[a]/256) & 255;
		lk.data[ptt++] = cts->cds[a].objects.size();
		lk.data[ptt++] = addr2[a] & 255; lk.data[ptt++] = (addr2[a] / 256) & 255;
		ptt += 2;
	}

	addrs[4] = lk.pos;
	int tbl[] = { 1,4 };
	int obcounter = 0;
	for (int a = 1; a < cts->files.size(); a++) {
		if (cts->files[a]->typ == FileType::FT_OBJECTSET) {	OBtileF* fi = (OBtileF*)cts->files[a];obcounter += fi->objct;}
	}
	lk.wbyte(obcounter);
	int posp = lk.pos;
	int inip = lk.pos;
	lk.pos += obcounter * 2;

	debtx += "Building sprite graphics"; debtx += "|";
	for (int b = 0; b < cts->objgfx.size(); b++) {
		lk.data[posp++] = (lk.pos- inip) & 255; lk.data[posp++] = ((lk.pos - inip) / 256) & 255;
			int szn = cts->objgfx[b].siz;
		if (szn>0){szn=1;}
		lk.wbyte(cts->objgfx[b].type);
		lk.wbyte(szn | (cts->objgfx[b].multic*2));
		lk.wbyte(cts->objgfx[b].framect);
		lk.wbyte(cts->objgfx[b].tilct);
		
		for (int c = 0; c < cts->objgfx[b].framect; c++) {
			for (int d = 0; d < tbl[szn] ; d++) {
				lk.wbyte(cts->objgfx[b].frames[c][d]);
			}
		}
		int col[16],colct,mix,mid,mul,dt1,dt2,dt3,dt4,ovl;
		int tcache[96];
		bool nonew;
		for (int c = 0; c < cts->objgfx[b].tilct; c++) {
			if (cts->objgfx[b].multic > 0) {
				//Multicolor
				bool toocol = false;
				for (int d = 0; d < 16; d++) {
					colct = 0;col[0]=0;col[1]=0;col[2]=0;
					for (int e = 0; e < 16; e++) {
						nonew = false; for (int f = 0; f < colct; f++) { if (cts->objgfx[b].objtls[c].pix[(d * 16) + e] == (col[f] + 1)) { nonew = true; break; } }
						if (!nonew && cts->objgfx[b].objtls[c].pix[(d * 16) + e] > 0) {col[colct++] = cts->objgfx[b].objtls[c].pix[(d * 16) + e] - 1;}
					}
					if (colct > 3) { toocol = true; }
					if (colct < 2){col[1]=col[0];col[2]=col[0];colct = 1;}
					if (colct>2){ //you should not run this if there's no mixed colors, or terrible things happen
						mid = 3;
						mix = col[0] | col[1] | col[2]; for (int e = 0; e < colct; e++) { if (col[e] == mix) { mid = e; } }
						if (mid == 3 && colct > 2) { toocol = true; }
						if (mid != 3){ 
							mix = col[2]; col[2] = col[mid]; col[mid] = mix; //Swap mixed color to make it always be the color 2
						}
					}
					mul = 128; dt1 = 0; dt2 = 0; dt3 = 0; dt4 = 0; ovl = 0;
					for (int e = 0; e < 8; e++) {
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e] == col[0] + 1) { dt1 += mul; }
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e] == col[1] + 1) { dt2 += mul; }
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e] == col[2] + 1) { dt1 += mul; dt2 += mul; ovl = 64; }
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e + 8] == col[0] + 1) { dt3 += mul; }
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e + 8] == col[1] + 1) { dt4 += mul; }
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e + 8] == col[2] + 1) { dt3 += mul; dt4 += mul; ovl = 64; }
						mul /= 2;
					}
					tcache[d] = dt1; tcache[d + 16] = dt3; tcache[d + 32] = dt2; tcache[d + 48] = dt4;
					tcache[d + 64] = col[0]; tcache[d + 64 + 16] = col[1] + ovl;
				}
				char tex[50]; sprintf(tex, "%i", c);
				if (toocol){debtx += "Warning: tile "; debtx += tex; debtx += " has too many colors! |";}
				for (int b = 0; b < 96; b++) { lk.wbyte(tcache[b]); }
			
			} else {
			//Monocolor
				bool toocol = false;
				for (int d = 0; d < 16; d++) {
					colct = 0;
					for (int e = 0; e < 16; e++) {
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e] > 0) {
							if (colct == 0) { colct = cts->objgfx[b].objtls[c].pix[(d * 16) + e]; break; } else { toocol = true; break; }
						}
					}
					mul = 128; dt1 = 0; dt2 = 0;
					for (int e = 0; e < 16; e++) {
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e] > 0) { dt1 += mul; }
						if (cts->objgfx[b].objtls[c].pix[(d * 16) + e + 8] > 0) { dt2 += mul; }
						mul /= 2;
					}
					tcache[d] = dt1; tcache[d + 16] = dt2; 
					tcache[d + 32] = colct - 1;
				}
				char tex[50]; sprintf(tex, "%i", c);
				if (toocol) { debtx += "Warning: tile "; debtx += tex; debtx += " has too many colors! |"; }
				for (int b = 0; b < 48; b++) { lk.wbyte(tcache[b]); }
			}
		}
	}

	addrs[5] = lk.pos;
	int pr = 0;
	for (int a = 0; a < cts->files.size(); a++) {if (cts->files[a]->typ == FileType::FT_PROGRAMSET) {pr++;}}
	lk.wbyte(pr);
	int ptb = lk.pos;
	lk.pos += pr * 2;
	for (int a = 0; a < cts->files.size(); a++) {
		std::string proc;
		if (cts->files[a]->typ == FileType::FT_PROGRAMSET) {
			char addrtx[100];
			ProgramFile* pr = (ProgramFile*)cts->files[a];
			int add = 0x8400 + lk.pos;
			sprintf(addrtx," at %#04x",add);
			debtx += "Building:"; debtx += cts->files[a]->name;debtx +=addrtx; debtx += "|";
			lk.data[ptb] = add & 255;lk.data[ptb+1] = (add/256) & 255;ptb += 2;
			std::vector <int> build = AssembleProgramNew(pr->prg,add,cts->macros);
			debtx+=asmerr;
			for (int a = 0; a < build.size();a++){lk.wbyte(build[a]);}

		}
	}
	for (int a = 0; a < 16; a++) {
		lk.data[(a * 2)] = addrs[a] & 255;
		lk.data[(a * 2)+1] = (addrs[a]/256) & 255;
	}

	fina = cts->name;
	fina.resize(cts->name.size() - 3); fina += "lvl";
	debtx += "writing: " + fina+ " to disk|";
	fina = cts->fpath;
	fina.resize(cts->fpath.size() - 3); fina += "lvl";
	FILE* lvlout;
	lvlout= fopen(fina.c_str(), "wb");
	fwrite(lk.data, lk.pos, 1, lvlout);
	fclose(lvlout);
	if (copyit>0){
		fina = copyto+"/"+cts->name;
		fina.resize(fina.size() - 3); fina += "lvl";
		debtx += "also writing: " + fina+ " to disk|";
		lvlout= fopen(fina.c_str(), "wb");
		fwrite(lk.data, lk.pos, 1, lvlout);
		fclose(lvlout);
	}

error:;
	delete[] lk.data;
	return debtx;
}

std::string CompilePack() {
	//New format thing
	//Adress, size, 4 bytes each, upper 0x80 on size tell program if we go ram or vram
	
	int addrs[16],gaddrs[16],gcaddrs[16];
	std::string fina;
	DH lk[100];
	int tpblock[100],destbl[100];
	int nud = 5;

	tpblock[0] = 0;destbl[0]=0x5c00;lk[0].data = new unsigned char[1000000];
	memset(lk[0].data, 0, 100000);lk[0].pos = 0; // Sprite definitions
	tpblock[1] = 1;destbl[1]=0x0300;lk[1].data = new unsigned char[1000000];
	memset(lk[1].data, 0, 100000);lk[1].pos = 0; // Sprite graphics
	tpblock[2] = 2;destbl[2]=0x8200; lk[2].data = new unsigned char[1000000];
	memset(lk[2].data, 0, 100000);lk[2].pos = 0; // Sprite addr table
	tpblock[3] = 1;destbl[3]=0x0220; lk[3].data = new unsigned char[1000000];
	memset(lk[3].data, 0, 100000);lk[3].pos = 0; // bitmap
	tpblock[4] = 1;destbl[4]=0x0230;lk[4].data = new unsigned char[1000000];
	memset(lk[4].data, 0, 100000);lk[4].pos = 0; // Sprite color data
	

	std::string debtx;
	int tbl[] = { 1,4 };
	LoadFile("./base.uob", "base.uob",false);
	OBtileF* fi = (OBtileF*)fileoutld;
	
	debtx += "Building: base.bin|";

	for (int b = 0; b < fi->objct; b++) {
		//lk[0].data[posp++] = (lk[0].pos - inip) & 255; lk[0].data[posp++] = ((lk[0].pos - inip) / 256) & 255;
		int szn = fi->objs[b].siz;
		if (szn>0){szn=1;}
		addrs[b] = lk[0].pos;
		lk[0].wbyte(fi->objs[b].type);
		lk[0].wbyte(szn | (fi->objs[b].multic * 2));
		lk[0].wbyte(fi->objs[b].framect);
		lk[0].wbyte(fi->objs[b].tilct);
		for (int c = 0; c < fi->objs[b].framect; c++) {
			for (int d = 0; d < tbl[szn]; d++) {
				lk[0].wbyte(fi->objs[b].frames[c][d]);
			}
		}
	}
	for (int b = 0; b < fi->objct; b++) {
		int col[16], colct, mix, mid, mul, dt1, dt2, dt3, dt4, dt1s, dt2s, dt3s, dt4s, ovl;
		int tcache[196];
		int ccache[196];
		bool nonew;
		int nv = (lk[1].pos | 127)+1;
		if ((nv-lk[1].pos)<32){
			int p2=nv-lk[1].pos;
			for (int a = 0; a < (p2);a++){lk[1].wbyte(0);}
		}
		nv = (lk[4].pos | 127)+1;
		if ((nv-lk[4].pos)<32){
			int p2=nv-lk[4].pos;
			for (int a = 0; a < (p2);a++){lk[4].wbyte(0);}
		}

		gaddrs[b] = lk[1].pos;
		gcaddrs[b] = lk[4].pos;
		for (int c = 0; c < fi->objs[b].tilct; c++) {
			if (fi->objs[b].multic > 0) {
				//Multicolor
				bool toocol = false;
				if (c==1){
					int paatytime=1;
				}
				for (int d = 0; d < 16; d++) {
					colct = 0;
					col[1]=0;col[0]=0;col[2]=0;
					for (int e = 0; e < 16; e++) {
						nonew = false; 
						for (int f = 0; f < colct; f++) { 
							if (fi->objs[b].objtls[c].pix[(d * 16) + e] == (col[f] + 1)) { nonew = true; break; } 
						}
						if (!nonew && fi->objs[b].objtls[c].pix[(d * 16) + e] > 0) { col[colct++] = fi->objs[b].objtls[c].pix[(d * 16) + e] - 1; }
					}
					if (colct > 3) { toocol = true; }
					if (colct < 2){col[1]=col[0];col[2]=255;colct = 1;}
					if (colct>2){ //you should not run this if there's no mixed colors, or terrible things happen
						mid = 3;
						mix = col[0] | col[1] | col[2]; for (int e = 0; e < colct; e++) { if (col[e] == mix) { mid = e; } }
						if (mid == 3 && colct > 2) { toocol = true; }
						if (mid != 3){ 
							mix = col[2]; col[2] = col[mid]; col[mid] = mix; //Swap mixed color to make it always be the color 2
						}
					}
					mul = 128; dt1 = 0; dt2 = 0; dt3 = 0; dt4 = 0; ovl = 0; dt1s = 0; dt2s = 0; dt3s = 0; dt4s = 0;
					for (int e = 0; e < 8; e++) {
						if (fi->objs[b].objtls[c].pix[(d * 16) + e] == col[0] + 1) { dt1 += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + e] == col[1] + 1) { dt2 += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + e] == col[2] + 1) { dt1 += mul; dt2 += mul; ovl = 64; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + e + 8] == col[0] + 1) { dt3 += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + e + 8] == col[1] + 1) { dt4 += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + e + 8] == col[2] + 1) { dt3 += mul; dt4 += mul; ovl = 64; }
						int re = 7 - e;
						if (fi->objs[b].objtls[c].pix[(d * 16) + re + 8] == col[0] + 1) { dt1s += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + re + 8] == col[1] + 1) { dt2s += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + re + 8] == col[2] + 1) { dt1s += mul; dt2s += mul; ovl = 64; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + re] == col[0] + 1) { dt3s += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + re] == col[1] + 1) { dt4s += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + re] == col[2] + 1) { dt3s += mul; dt4s += mul; ovl = 64; }
						mul /= 2;
					}

					tcache[d] = dt1; tcache[d + 16] = dt3; tcache[d + 32] = dt2; tcache[d + 48] = dt4;
					tcache[d + 64] = dt1s; tcache[d + 16 + 64] = dt3s; tcache[d + 32 + 64] = dt2s; tcache[d + 48 + 64] = dt4s;
					ccache[d] = col[0]; ccache[d + 16] = col[1] + ovl;
				}
				char tex[50]; sprintf(tex, "%i from object %i",c, b);
				if (toocol) { debtx += "Warning: tile "; debtx += tex; debtx += " has too many colors! |"; }
				for (int b = 0; b < 128; b++) { lk[1].wbyte(tcache[b]); }
				for (int b = 0; b < 32; b++) { lk[4].wbyte(ccache[b]); }

			}
			else {
				//Monocolor
				bool toocol = false;
				for (int d = 0; d < 16; d++) {
					colct = 0;
					for (int e = 0; e < 16; e++) {
						if (fi->objs[b].objtls[c].pix[(d * 16) + e] > 0) {
							if (colct == 0) { colct = fi->objs[b].objtls[c].pix[(d * 16) + e]; break; }
							else { toocol = true; break; }
						}
					}
					mul = 128; dt1 = 0; dt2 = 0; dt1s = 0; dt2s = 0;
					for (int e = 0; e < 8; e++) {
						if (fi->objs[b].objtls[c].pix[(d * 16) + e] > 0) { dt1 += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + e + 8] > 0) { dt2 += mul; }
						int re = 7 - e;
						if (fi->objs[b].objtls[c].pix[(d * 16) + re] > 0) { dt1s += mul; }
						if (fi->objs[b].objtls[c].pix[(d * 16) + re + 8] > 0) { dt2s += mul; }
						mul /= 2;
					}
					tcache[d] = dt1; tcache[d + 16] = dt2;
					tcache[d + 32] = dt2s; tcache[d + 32 + 16] = dt1s;
					ccache[d] = colct - 1;
				}
				char tex[50]; sprintf(tex, "%i from object %i", c, b);
				if (toocol) { debtx += "Warning: tile "; debtx += tex; debtx += " has too many colors! |"; }
				for (int b = 0; b < 64; b++) { lk[1].wbyte(tcache[b]); }
				for (int b = 0; b < 16; b++) { lk[4].wbyte(ccache[b]); }
			}
		}
	}
	int cct = fi->objct;
	//lk[2].wbyte(cct & 255); lk[2].wbyte((cct/256) & 255);
	for (int a = 0;a < fi->objct; a++){
		cct = addrs[a]+ 0x5c00;
		lk[2].wbyte(cct & 255); lk[2].wbyte((cct/256) & 255);
		cct = gaddrs[a]+ 0x3000;
		lk[2].wbyte(cct & 255); lk[2].wbyte((cct/256) & 255);
		cct = gcaddrs[a]+ 0x3000;
		lk[2].wbyte(cct & 255); lk[2].wbyte((cct/256) & 255);
		lk[2].wbyte(0x12); lk[2].wbyte(0x34);
	}
	SDL_Surface * silva;
	int palcol[16][3];
	int pict=0;
	silva = SDL_LoadBMP("palette.bmp");
	SDL_LockSurface(silva);
	unsigned char * me = (unsigned char*)silva->pixels;
	for (int x = 0; x < 16; x++){
		palcol[x][0] = me[(x*96)];
		palcol[x][1] = me[(x*96)+1];
		palcol[x][2] = me[(x*96)+2];
	}
	SDL_UnlockSurface(silva);

	silva = SDL_LoadBMP("bitmap.bmp");
	me = (unsigned char*)silva->pixels;
	SDL_LockSurface(silva);
	int bat = 0;
	int dis = 0,mdis = 0, selc = 0;
	for (int y = 0; y < 24; y++){
		for (int x = 0; x < 256; x+=2){
			mdis = 100000;selc = 0;bat = 0;
			for (int c = 0; c < 16; c++){
				dis = abs(palcol[c][0] - me[((x+(y*256))*3)+0])+abs(palcol[c][1] - me[((x+(y*256))*3)+1])+abs(palcol[c][2] - me[((x+(y*256))*3)+2]);	
				if (mdis > dis){mdis=dis;selc=c;}
			}
			bat = selc * 16; mdis = 100000;selc = 0;
			for (int c = 0; c < 16; c++){
				dis = abs(palcol[c][0] - me[((x+1+(y*256))*3)+0])+abs(palcol[c][1] - me[((x+1+(y*256))*3)+1])+abs(palcol[c][2] - me[((x+1+(y*256))*3)+2]);	
				if (mdis > dis){mdis=dis;selc=c;}
			}
			lk[3].wbyte(bat + selc);
		}
	}
	SDL_UnlockSurface(silva);

	FILE* lvlout;
	lvlout = fopen("bose.bin", "wb");
	int posr = 0x8401 + nud*7;
	int sz;
	unsigned char bt;
	bt = nud; fwrite(&bt,1,1,lvlout);
	for (int a = 0; a < nud; a++){
		bt = tpblock[a];fwrite(&bt,1,1,lvlout);
		bt = posr & 255;fwrite(&bt,1,1,lvlout);bt = (posr/256) & 255;fwrite(&bt,1,1,lvlout);
		sz = lk[a].pos;
		bt = sz & 255;fwrite(&bt,1,1,lvlout);bt = (sz/256) & 255;fwrite(&bt,1,1,lvlout);
		posr+=lk[a].pos;
		sz = destbl[a];
		bt = sz & 255;fwrite(&bt,1,1,lvlout);bt = (sz/256) & 255;fwrite(&bt,1,1,lvlout);

	}
	for (int a = 0; a < nud; a++){	
		fwrite(lk[a].data, lk[a].pos, 1, lvlout);
	}
	fclose(lvlout);
	delete fi;
	return debtx;
}