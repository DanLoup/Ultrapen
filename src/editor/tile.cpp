#include "data.h"
int selL=0,selR=1;

bgtile::bgtile(){
    memset(tmap,1,64);
}
void bgtile::GetBits(char * data){
	int multi;
	for (int a=0;a<8;a++){
		multi = 128;data[a]=0;
		for (int b =0;b<8;b++){
			data[a]+=tmap[b+(a*8)]*multi;multi/=2;
		}
	}
}
void bgtile::GetPals(char * data){
	for (int a=0;a<8;a++){
		data[a]=plines[a][0]+(plines[a][1]*16);
	}
}
void bgtile::SetBits(char * data){
	int multi;
	for (int a=0;a<8;a++){
		multi=128;
		for (int b=0;b<8;b++){
			if ((data[a]&multi)>0){tmap[b+(a*8)]=1;}else{tmap[b+(a*8)]=0;}
			multi/=2;
		}
	}
}
void bgtile::SetPals(char * data){
	for (int a=0;a<8;a++){
		plines[a][0]=data[a]&15;
		plines[a][1]=(data[a]/16)&15;
	}
}
void bgtile::BlitTile(char *fb,int fbwi,ivec2 pos){
	int d1,cc,xa,ya;
	for (int b=0;b<8;b++){
		for (int a=0;a<8;a++){
			xa=a+pos.x;ya=b+pos.y;
			d1=tmap[a+(b*8)];cc=plines[b][d1];
			fb[((xa+(ya*fbwi))*4)]=255;
			fb[((xa+(ya*fbwi))*4)+1]=pals[0].col[cc].b;
			fb[((xa+(ya*fbwi))*4)+2]=pals[0].col[cc].g;
			fb[((xa+(ya*fbwi))*4)+3]=pals[0].col[cc].r;
		}
	}
}
sptile::sptile(){
    memset(tmap,0,256);
}
void sptile::GetBits(char * data){
	int multi,dtc=0;
	for (int d=1;d<3;d++){
		for (int c=0;c<2;c++){
			for (int a=0;a<16;a++){
				multi=128;data[dtc]=0;
				for (int b=0;b<8;b++){
					if ((tmap[b+(a*16)+(c*8)]&d)>0){data[dtc]+=multi;}
					multi/=2;
				}
				dtc++;
			}
		}
	}
}
void sptile::GetPals(char * data){
	for (int a=0;a<16;a++){data[a]=plines[a][0];}
	for (int a=0;a<16;a++){data[a+16]=plines[a][1]+limix[a]*64;}
}

void sptile::SetBits(char * data){
	int multi,dtc=0;
	memset(tmap,0,64);
	memset(limix,0,16);
	for (int d=1;d<3;d++){
		for (int c=0;c<2;c++){
			for (int a=0;a<16;a++){
				multi=128;
				for (int b=0;b<8;b++){
					if ((data[dtc]&multi)>0){tmap[b+(a*16)+(c*8)]+=d;}
					multi/=2;
				}
				dtc++;
			}
		}
	}
	Refreshlimix();
}
void sptile::SetPals(char * data){
	for (int a=0;a<16;a++){
		plines[a][0]=data[a] & 15;
		plines[a][1]=(data[a]/16) & 15;
		plines[a][2]=plines[a][0] | plines[a][1];
	}
}
void sptile::BlitTile(char *fb,int fbwi,ivec2 pos,int mc){
	int d1,cc,xa,ya;
	for (int b=0;b<16;b++){
		for (int a=0;a<16;a++){
			xa=a+pos.x;ya=b+pos.y;
			d1=tmap[a+(b*16)];
			if (d1>0){
				if (mc==0){d1=1;}
				cc=plines[b][d1-1];
				fb[((xa+(ya*fbwi))*4)]=255;
				fb[((xa+(ya*fbwi))*4)+1]=pals[0].col[cc].b;
				fb[((xa+(ya*fbwi))*4)+2]=pals[0].col[cc].g;
				fb[((xa+(ya*fbwi))*4)+3]=pals[0].col[cc].r;
			}else{
				fb[((xa+(ya*fbwi))*4)]=255;
				fb[((xa+(ya*fbwi))*4)+1]=0;
				fb[((xa+(ya*fbwi))*4)+2]=0;
				fb[((xa+(ya*fbwi))*4)+3]=0;
			}
		}
	}
}
void sptile::Refreshlimix(){
	for (int a=0;a<16;a++){
		limix[a]=0;
		for (int b=0;b<16;b++){if (tmap[b+(a*16)]==3){limix[a]=1;break;}}
	}
}

void bgtileunit::draw(char *fb,int fbwi,ivec2 pos){
	int cc=0,xa,ya;
	for (int b=0;b<8;b++){
		for (int a=0;a<8;a++){
			xa=a+pos.x;ya=b+pos.y;
			cc=pix[a+(b*8)];
			fb[((xa+(ya*fbwi))*4)+1]=pals[0].col[cc].b;
			fb[((xa+(ya*fbwi))*4)+2]=pals[0].col[cc].g;
			fb[((xa+(ya*fbwi))*4)+3]=pals[0].col[cc].r;
		}
	}
}

void bgtileunit::fill(char *fb,int fbwi,ivec2 pos,color cl){
	int cc=0,xa,ya;
	for (int b=0;b<8;b++){
		for (int a=0;a<8;a++){
			xa=a+pos.x;ya=b+pos.y;
			fb[((xa+(ya*fbwi))*4)+1]=cl.b;
			fb[((xa+(ya*fbwi))*4)+2]=cl.g;
			fb[((xa+(ya*fbwi))*4)+3]=cl.r;
		}
	}
}

void objtileunit::draw(char *fb,int fbwi,ivec2 pos){
	int cc=0,xa,ya;
	for (int b=0;b<16;b++){
		for (int a=0;a<16;a++){
			xa=a+pos.x;ya=b+pos.y;
			cc=pix[a+(b*16)]-1;
			int chess=48+(((a & 2)>>1) ^ ((b&2)>>1))*32;
			if (cc>-1){
				fb[((xa+(ya*fbwi))*4)+1]=pals[0].col[cc].b;
				fb[((xa+(ya*fbwi))*4)+2]=pals[0].col[cc].g;
				fb[((xa+(ya*fbwi))*4)+3]=pals[0].col[cc].r;
			}else{
				fb[((xa+(ya*fbwi))*4)+1]=chess;
				fb[((xa+(ya*fbwi))*4)+2]=chess;
				fb[((xa+(ya*fbwi))*4)+3]=chess;
			}
		}
	}
}

void objtileunit::fill(char *fb,int fbwi,ivec2 pos,color cl){
	int cc=0,xa,ya;
	for (int b=0;b<16;b++){
		for (int a=0;a<16;a++){
			xa=a+pos.x;ya=b+pos.y;
			fb[((xa+(ya*fbwi))*4)+1]=cl.b;
			fb[((xa+(ya*fbwi))*4)+2]=cl.g;
			fb[((xa+(ya*fbwi))*4)+3]=cl.r;
		}
	}
}

void fb::Init(SDL_Renderer * iren, ivec2 isize){
	ren = iren;
	size = isize;
	bf = SDL_CreateTexture(ren,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,size.x,size.y);
	buff.clear();
	buff.resize(size.x*size.y*4);
}

void fb::draw(ivec2 pos,float scale){
	SDL_Rect sc,prs;
	sc.x=0;sc.y=0;sc.w=size.x;sc.h=size.y;
	prs.x=pos.x;prs.y=pos.y;prs.w=size.x*scale;prs.h=size.y*scale;
	SDL_RenderCopy(ren,bf,&sc,&prs);
}

void fb::drawclipped(ivec2 pos,float scale, ivec2 cut){
	SDL_Rect sc,prs;
	sc.x=0;sc.y=0;sc.w=cut.x;sc.h=cut.y;
	prs.x=pos.x;prs.y=pos.y;prs.w=cut.x*scale;prs.h=cut.y*scale;
	SDL_RenderCopy(ren,bf,&sc,&prs);
}

void fb::update(){
	if (size.x>0){
		SDL_Rect sc;
		sc.x=0;sc.y=0;sc.w=size.x,sc.h=size.y;
		SDL_UpdateTexture(bf,&sc,buff.data(),size.x*4);
	}
}

void fb::clear(color col){
		for (int y = 0; y < size.y;y++){
		for (int x = 0; x < size.x;x++){
			int addr = ((x)+((y)*size.x))*4;
			buff[addr + 1] = col.b;
			buff[addr + 2] = col.g;
			buff[addr + 3] = col.r;
		}
	}
}

void tile::draw(fb * f,ivec2 pos){
	for (int y = 0; y < tilesiz.y;y++){
		for (int x = 0; x < tilesiz.x;x++){
			int addr = ((x+pos.x)+((y+pos.y)*f->size.x))*4;
			int cc = pix[x+(y*tilesiz.x)];
			if (tiletype==0){
				f->buff[addr + 1]=pals[0].col[cc].b;
				f->buff[addr + 2]=pals[0].col[cc].g;
				f->buff[addr + 3]=pals[0].col[cc].r;
			}else{
				int chess=48+(((x & 2)>>1) ^ ((y&2)>>1))*32;
				cc--;
				if (cc>-1){
					f->buff[addr + 1]=pals[0].col[cc].b;
					f->buff[addr + 2]=pals[0].col[cc].g;
					f->buff[addr + 3]=pals[0].col[cc].r;
				}else{
					f->buff[addr + 1]=chess;
					f->buff[addr + 2]=chess;
					f->buff[addr + 3]=chess;
				}
			}
		}
	}
}


void tile::drawlc(char * pic,int wi,ivec2 pos){
	for (int y = 0; y < tilesiz.y;y++){
		for (int x = 0; x < tilesiz.x;x++){
			int addr = ((x+pos.x)+((y+pos.y)*wi))*4;
			int cc = pix[x+(y*tilesiz.x)];
			if (tiletype==0){
				pic[addr + 1]=pals[0].col[cc].b;
				pic[addr + 2]=pals[0].col[cc].g;
				pic[addr + 3]=pals[0].col[cc].r;
			}else{
				int chess=48+(((x & 2)>>1) ^ ((y&2)>>1))*32;
				cc--;
				if (cc>-1){
					pic[addr + 1]=pals[0].col[cc].b;
					pic[addr + 2]=pals[0].col[cc].g;
					pic[addr + 3]=pals[0].col[cc].r;
				}else{
					pic[addr + 1]=chess;
					pic[addr + 2]=chess;
					pic[addr + 3]=chess;
				}
			}
		}
	}
}

void tile::fill(fb * f, ivec2 pos,color col){
	for (int y = 0; y < tilesiz.y;y++){
		for (int x = 0; x < tilesiz.x;x++){
			int addr = ((x+pos.x)+((y+pos.y)*f->size.x))*4;
			f->buff[addr + 1] = col.b;
			f->buff[addr + 2] = col.g;
			f->buff[addr + 3] = col.r;
		}
	}
}