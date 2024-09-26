#include "MainWindow.h"
#include "data.h"

//Window * tiwin;
//Control * penmode[4];
int curtl=0,curbl=0,curcl=0;
SDL_Texture * tbuffers[3];
void LoadBMP();
void SaveBMP();
fb fbs[3];

void BGtileF::Boot(bool newf){
	tiwin=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	tiwin->AddControl("sc1",Control::SCROLL,ivec2(420+256+8,16),ivec2(20,256+8));
	tiwin->AddControl("sc2",Control::SCROLL,ivec2(120+256+8,16+280),ivec2(20,256+8));
	tiwin->AddControl("sc3",Control::SCROLL,ivec2(420+256+8,16+280),ivec2(20,256+8));
	tiwin->AddControl("tiletype",Control::COMBO,ivec2(10,300),ivec2(105,20));

    tiwin->SetText("tiletype","Air|Solid|Stairs|Stairs top|Kill|LeftRoll|RightRoll|");
		//tbuffers[a]=SDL_CreateTexture(rd,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,256,256);
		fbs[0].Init(rd,ivec2(64,64));
		fbs[1].Init(rd,ivec2(64,64));
		fbs[2].Init(rd,ivec2(128,128));
		//fbs[3].Init(rd,ivec2(256,256));

	tiwin->radioch[0]=0;
	penmode[0]=new Control(tiwin,Control::RADIO,ivec2(10,50),ivec2(100,30));penmode[0]->SetText("Pen");
	penmode[1]=new Control(tiwin,Control::RADIO,ivec2(10,50+(25*1)),ivec2(100,30));penmode[1]->SetText("Line");
	penmode[2]=new Control(tiwin,Control::RADIO,ivec2(10,50+(25*2)),ivec2(100,30));penmode[2]->SetText("Fill");
	penmode[3]=new Control(tiwin,Control::RADIO,ivec2(10,50+(25*3)),ivec2(100,30));penmode[3]->SetText("Copy");
	for (int a=0;a<4;a++){
		penmode[a]->setValue(2,a);
	}
	if (newf){
		for (int a = 0; a < 512;a++){
			for (int b=0;b < 256;b++){
				tiles[a].pix[b]=0;
			}
			for (int b=0;b < 4;b++){
				groups[a].box[b]=0;
				clusters[a].box[b]=0;
			}

		}
	}
}

void BGtileF::Run(int status){
	RunPalette(ivec2(100,600),tiwin);
	tiwin->RunWindow();
	ivec2 pos=tiwin->pos;
	HandlePaint();
	HandleTiles();
	HandleBlocks();
	HandleClusters();
	if (breath<4){breath++;}
	if (mouse_bt>0 && breath > 3){RepaintTextures();breath=0;}
	strcpy(flexmenu,"Tiles|Import|Export|Import BMP|Export BMP");

}
bool mohold=false;
int colhold=0;
int tilclip[64],colclip[16];
int omouse_bt,colorer;
ivec2 stline;
void BGtileF::HandlePaint(){
	ivec2 pos=tiwin->pos;
	int col,posp;
	int pmode=penmode[0]->getValue(0);
	Print(pos+ivec2(120,0),color(0),"Drawing area:");
	tiwin->DrawBorderFilled(pos+ivec2(120,16),ivec2(256+8,256+8),2,color(0));

	if (MouseTest(pos+ivec2(124,20),ivec2(252,252))){
		ivec2 mpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124,20)))/16;
		int pmode=penmode[0]->getValue(0);
		posp=((mpos.x&8)/8)+(mpos.y&8)+curtl;
		if (pmode==0){
			if (mouse_bt>0){
				if (mouse_bt==1){tiles[posp].pix[(mpos.x&7)+((mpos.y&7)*8)]=selL;}
				if (mouse_bt==4){tiles[posp].pix[(mpos.x&7)+((mpos.y&7)*8)]=selR;}
				if (mouse_bt==2 || (mouse_bt==1 && keyshift>0)){tiles[posp].pix[(mpos.x&7)+((mpos.y&7)*8)]=selL| selR;}
    		}
		}
		if (pmode==1){
			ivec2 clmo=ivec2(mpos.x&15,mpos.y&15);
			if (mouse_bt>0 && omouse_bt==0){
				if (mouse_bt==1){colorer=selL;}
				if (mouse_bt==4){colorer=selR;}
				if (mouse_bt==2 || (mouse_bt==1 && keyshift>0)){colorer=selL | selR;}
				stline=clmo;
			}
			if (mouse_bt==0 && omouse_bt>0){
				if (abs(stline.x-clmo.x)>abs(stline.y-clmo.y)){
					int ini=stline.x,ed=clmo.x;
					float vec=float(clmo.y-stline.y)/float(abs(ini-ed)),stvec=stline.y;
					if (ed<ini){int sw=ed;ed=ini;ini=sw;vec*=-1.0;stvec=clmo.y;}
					for (int a=ini;a<ed+1;a++){
						int psp=((a&8)/8)+(int(stvec)&8)+curtl;
						tiles[psp].pix[(a&7)+((int(stvec)&7)*8)]=colorer;
						stvec+=vec;
					}
				}else{
					int ini=stline.y,ed=clmo.y;
					float vec=float(clmo.x-stline.x)/float(abs(ini-ed)),stvec=stline.x;
					if (ed<ini){int sw=ed;ed=ini;ini=sw;vec*=-1.0;stvec=clmo.x;}
					for (int a=ini;a<ed+1;a++){
						int psp=((int(stvec)&8)/8)+(a&8)+curtl;
						tiles[psp].pix[(int(stvec)&7)+((a&7)*8)]=colorer;
						stvec+=vec;
					}
				}
			}

		}
		if (pmode==2){
			if (mouse_bt>0 && omouse_bt==0){

				ivec2 c1[128],c2[128];
				ivec2 rot[]={ivec2(0,-1),ivec2(1,0),ivec2(0,1),ivec2(-1,0)};
				int cc1=1,cc2=0;
				int tacol=tiles[posp].pix[(mpos.x&7)+((mpos.y&7)*8)],recol;
				if (mouse_bt==1){recol=selL;}
				if (mouse_bt==4){recol=selR;}
				if (mouse_bt==2 || (mouse_bt==1 && keyshift>0)){recol=selL | selR;}
				tiles[posp].pix[(mpos.x&7)+((mpos.y&7)*8)]=recol;
				c1[0]=ivec2(mpos.x&7,mpos.y&7);
				bool filled=false;
				while (!filled){
					cc2=0;
					filled=true;
					for (int a=0;a<cc1;a++){
						for (int b=0;b<4;b++){
							ivec2 tt=rot[b]+c1[a];
							if (tt.x>-1 && tt.x<8 && tt.y>-1 && tt.y<8){
								if (tiles[posp].pix[tt.x+(tt.y*8)]==tacol){
									filled=false;tiles[posp].pix[tt.x+(tt.y*8)]=recol;c2[cc2++]=tt;
								}
							}
						}
					}
					if (recol==tacol){filled=true;}
					for (int a=0;a<cc2;a++){
						c1[a]=c2[a];
					}
					cc1=cc2;
				}
			}
		}
		if (pmode==3){
            if (mouse_bt==4){
				for (int a=0;a<64;a++){tilclip[a]=tiles[posp].pix[a];}
            }
            if (mouse_bt==1){
				for (int a=0;a<64;a++){tiles[posp].pix[a]=tilclip[a];}
            }
		}
	}

	if (pmode==0){strcpy(st_text,"LMB/RMB paints a color pixel");}
	if (pmode==1){strcpy(st_text,"LMB/RMB draw a line of this color");}
	if (pmode==2){strcpy(st_text,"LMB/RMB fill with color");}
	if (pmode==3){strcpy(st_text,"RMB copy 8x8 sector from pixel editor, LMB paste");}

	ivec2 ps;
	int a1=0,a2=0;
	int ac1[8],ac2[8];
	for (int b=0;b<16;b++){
		a1=0;a2=0;
		for (int a=0;a<8;a++){
			bool haveit=false;
			int ccol=tiles[(b&8)+curtl].pix[(a&7)+((b&7)*8)];
			for (int c=0;c<a1;c++){if (ccol==ac1[c]){haveit=true;break;}}
			if (!haveit){ac1[a1++]=ccol;}
		}

		for (int a=7;a<16;a++){
			bool haveit=false;
			int ccol=tiles[(b&8)+1+curtl].pix[(a&7)+((b&7)*8)];
			for (int c=0;c<a2;c++){if (ccol==ac2[c]){haveit=true;break;}}
			if (!haveit){ac2[a2++]=ccol;}
		}

		for (int a=0;a<16;a++){
			posp=((a&8)/8)+((b&8))+curtl;
			col=tiles[posp].pix[(a&7)+((b&7)*8)];
			Box(pos+ivec2((a*16)+124,(b*16)+20),pos+ivec2((a*16)+140,(b*16)+36),pals[0].col[col]);
			if (a<8 && a1>2){Box(pos+ivec2((a*16)+124,(b*16)+27),pos+ivec2((a*16)+140,(b*16)+29),color(192,0,0));}
			if (a>7 && a2>2){Box(pos+ivec2((a*16)+124,(b*16)+27),pos+ivec2((a*16)+140,(b*16)+29),color(192,0,0));}

		}
	}
	for (int a=0;a<16;a++){
			int col=128;
			if (a==8){col=255;}
			ps=pos+ivec2(124,20);Line(ps+ivec2(a*16,0),ps+ivec2(a*16,255),color(col));Line(ps+ivec2(0,a*16),ps+ivec2(255,a*16),color(col));
	}
	if (pmode==1 && mouse_bt>0 && MouseTest(pos+ivec2(124,20),ivec2(252,252))){
		ivec2 mpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124,20)))/16;
		ps=pos+ivec2(124,20);
		Line(ps+ivec2((stline.x*16)+8,(stline.y*16)+8),ps+ivec2((mpos.x*16)+8,(mpos.y*16)+8),color(192));
	}
	if (flexmenuout==3){
		LoadBMP();
	}
	if (flexmenuout==4){
		bmwi=64;
		bmhe=512;
		bmbytes=4;
		for (int a=0;a<512;a++){
			for (int b=0;b<64;b++){
				int tiadd=(b/8)+int(a/8)*8;
                int piadd=(b & 7)+((a & 7)*8);
				int bb=b*bmbytes;
				int d=tiles[tiadd].pix[piadd];
				bitmem[bb+0+(a*bmwi*bmbytes)]=pals[0].col[d].r;
				bitmem[bb+1+(a*bmwi*bmbytes)]=pals[0].col[d].g;
				bitmem[bb+2+(a*bmwi*bmbytes)]=pals[0].col[d].b;
			}
		}
		SaveBMP();
	}
	if (bmdone){
		bmdone=false;
		int he=bmhe;
		int wi=bmwi;
		if (he>512){he=512;}
		if (wi>64){wi=64;}
        for (int a=0;a<he;a++){
			for (int b=0;b<wi;b++){
				int selcol=0,score=100000;
				for (int d=0;d<16;d++){
					int bb=b*bmbytes;
					int misc=abs(pals[0].col[d].r-bitmem[bb+2+(a*bmwi*bmbytes)]);
					misc+=abs(pals[0].col[d].g-bitmem[bb+1+(a*bmwi*bmbytes)]);
					misc+=abs(pals[0].col[d].b-bitmem[bb+0+(a*bmwi*bmbytes)]);
					if (misc<score){score=misc;selcol=d;}
				}
				int tiadd=(b/8)+int(a/8)*8;
                int piadd=(b & 7)+((a & 7)*8);
                tiles[tiadd].pix[piadd]=selcol;
			}
        }
	}
	omouse_bt=mouse_bt;
}

void BGtileF::HandleTiles(){
	ivec2 pos=tiwin->pos;
	ivec2 ps;
	int col,posp;
	Print(pos+ivec2(120+300,0),color(0),"8x8 tiles:");
	tiwin->DrawBorderFilled(pos+ivec2(120+300,16),ivec2(256+8,256+8),2,color(0));
	ivec2 pps=pos+ivec2(124+300,20);
	//SDL_Rect sc,prs;sc.x=0;sc.y=0;sc.w=64;sc.h=64;prs.x=pps.x;prs.y=pps.y;prs.w=256;prs.h=256;
	//SDL_RenderCopy(rd,tbuffers[0],&sc,&prs);
	fbs[0].draw(pps,4.0);
	for (int a=0;a<8;a++){ps=pos+ivec2(124+300,20);Line(ps+ivec2(a*32,0),ps+ivec2(a*32,255),color(192));Line(ps+ivec2(0,a*32),ps+ivec2(255,a*32),color(192));}

	if (MouseTest(pos+ivec2(124+300,20),ivec2(252,252))){
		if (mouse_bt==1){
			ivec2 mpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124+300,20)))/32;int spos=tiwin->GetValue("sc1",0)/84;mpos.y+=(spos*2);
			curtl=(mpos.x&254)+((mpos.y&254)*8);
		}
	}
	ivec2 ctlpos=ivec2((curtl&7)*32,((curtl/8))*32);
	int spos=tiwin->GetValue("sc1",0)/84;ctlpos.y-=spos*64;
	if (ctlpos.y>-1 && ctlpos.y<256){
		Contour(pos+ivec2(124+300,20)+ctlpos,ivec2(64,64),color(255,0,0));
	}
}
int selblm=0,selbls=0;
void BGtileF::HandleBlocks(){
	ivec2 pos=tiwin->pos;
	Print(pos+ivec2(120,280),color(0),"16x16 blocks:");
	tiwin->DrawBorderFilled(pos+ivec2(120,16+280),ivec2(256+8,256+8),2,color(0));

	ivec2 pps=pos+ivec2(124,20+280);
	SDL_Rect sc,prs;sc.x=0;sc.y=0;sc.w=64;sc.h=64;prs.x=pps.x;prs.y=pps.y;prs.w=256;prs.h=256;
	//SDL_RenderCopy(rd,tbuffers[1],&sc,&prs);
	fbs[1].draw(pps,4.0);
	ivec2 ps;
	for (int a=0;a<8;a++){
		ps=pos+ivec2(124,20+280);Line(ps+ivec2(a*32,0),ps+ivec2(a*32,255),color(64));Line(ps+ivec2(0,a*32),ps+ivec2(255,a*32),color(64));
	}
	for (int a=0;a<4;a++){
		ps=pos+ivec2(124,20+280);Line(ps+ivec2(a*64,0),ps+ivec2(a*64,255),color(192));Line(ps+ivec2(0,a*64),ps+ivec2(255,a*64),color(192));
	}
	int spos=tiwin->GetValue("sc2",0)/36;
	for (int a=0;a<4;a++){
		for (int b=0;b<4;b++){
			if (bltype[a + ((b + spos) * 4)] < 0) { bltype[a + ((b + spos) * 4)] = 0; }
			if (bltype[a + ((b + spos) * 4)] > 7) { bltype[a + ((b + spos) * 4)] = 7; }

			if (bltype[a+((b+spos)*4)]==1){Contour(pos+ivec2(127+(a*64),19+284+(b*64)),ivec2(59,59),color(255,128,64));}
			if (bltype[a+((b+spos)*4)]>1){Contour(pos+ivec2(127+(a*64),19+284+(b*64)),ivec2(59,59),color(128,128,128));}
			if (bltype[a+((b+spos)*4)]==4){Contour(pos+ivec2(127+(a*64),19+284+(b*64)),ivec2(59,59),color(255,0,0));}
			if (bltype[a+((b+spos)*4)]==5){Contour(pos+ivec2(127+(a*64),19+284+(b*64)),ivec2(59,59),color(128,128,0));}
			if (bltype[a+((b+spos)*4)]==6){Contour(pos+ivec2(127+(a*64),19+284+(b*64)),ivec2(59,59),color(255,255,0));}

		}
	}

	ivec2 ctlpos=ivec2((selblm&3)*64,((selblm/4))*64)+ivec2((selbls&1)*32,(selbls&2)*16);
	ctlpos.y-=spos*64;
	if (ctlpos.y>-1 && ctlpos.y<256){
		Contour(pos+ivec2(124,20+280)+ctlpos,ivec2(32,32),color(255,0,0));
	}
	if (MouseTest(pos+ivec2(124+300,20),ivec2(252,252))){
		if (mouse_bt>1){
			ivec2 mpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124+300,20)))/32;int spos=tiwin->GetValue("sc1",0)/84;mpos.y+=(spos*2);
			int cc=(mpos.x)+((mpos.y)*8);
			//blmaps[selblm][selbls]=cc;
			groups[selblm].box[selbls]=cc;
		}
	}
	if (tiwin->GetValue("tiletype",0)!=bltype[selblm]){
		bltype[selblm]=tiwin->GetValue("tiletype",0);
	}

	if (MouseTest(pos+ivec2(124,20+280),ivec2(253,253)) && mouse_bt==1){
		ivec2 mpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124,20+280)))/64;int spos=tiwin->GetValue("sc2",0)/18;mpos.y+=(spos/2);
		ivec2 mmpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124,20+280)))/32;
		selblm=(mpos.x)+((mpos.y)*4);
		selbls=(mmpos.x&1)+((mmpos.y&1)*2);
		tiwin->SetValue("tiletype",0,bltype[selblm]);
	}
}
int selclm=0,selcls=0;
void BGtileF::HandleClusters(){
	ivec2 pos=tiwin->pos;
	Print(pos+ivec2(120+300,280),color(0),"32x32 clusters:");
	tiwin->DrawBorderFilled(pos+ivec2(120+300,16+280),ivec2(256+8,256+8),2,color(0));

	ivec2 pps=pos+ivec2(124+300,20+280);
	SDL_Rect sc,prs;sc.x=0;sc.y=0;sc.w=128;sc.h=128;prs.x=pps.x;prs.y=pps.y;prs.w=256;prs.h=256;
	//SDL_RenderCopy(rd,tbuffers[2],&sc,&prs);
	fbs[2].draw(pps,2.0);
	ivec2 ps;
	for (int a=0;a<8;a++){ps=pos+ivec2(124+300,20+280);Line(ps+ivec2(a*32,0),ps+ivec2(a*32,255),color(64));Line(ps+ivec2(0,a*32),ps+ivec2(255,a*32),color(64));}
	for (int a=0;a<4;a++){ps=pos+ivec2(124+300,20+280);Line(ps+ivec2(a*64,0),ps+ivec2(a*64,255),color(192));Line(ps+ivec2(0,a*64),ps+ivec2(255,a*64),color(192));}
	ivec2 ctlpos=ivec2((selclm&3)*64,((selclm/4))*64)+ivec2((selcls&1)*32,(selcls&2)*16);
	int spos=tiwin->GetValue("sc3",0)/36;ctlpos.y-=spos*64;

	if (ctlpos.y>-1 && ctlpos.y<256){
		Contour(pos+ivec2(124+300,20+280)+ctlpos,ivec2(32,32),color(255,0,0));
	}
	if (MouseTest(pos+ivec2(124,20+280),ivec2(252,252))){
		if (mouse_bt>1){
			ivec2 mpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124,20+280)))/64;int spos=tiwin->GetValue("sc2",0)/18;mpos.y+=(spos/2);
			int cc=(mpos.x)+((mpos.y)*4);
			//clmaps[selclm][selcls]=cc;
			clusters[selclm].box[selcls]=cc;

			RepaintTextures();
		}
	}
	if (MouseTest(pos+ivec2(124+300,20+280),ivec2(253,253)) && mouse_bt==1){
		ivec2 mpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124+300,20+280)))/64;int spos=tiwin->GetValue("sc3",0)/18;mpos.y+=(spos/2);
		ivec2 mmpos=(ivec2(mouse_x,mouse_y)-(pos+ivec2(124+300,20+280)))/32;
		selclm=(mpos.x)+((mpos.y)*4);
		selcls=(mmpos.x&1)+((mmpos.y&1)*2);
	}
	for (int a=0;a<256;a++){
		for (int b=0;b<64;b++){if (tiles[a].pix[b]>0){tict=a;break;}}
	}
	for (int a=0;a<256;a++){
		for (int b=0;b<4;b++){if (groups[a].box[b]>0){grct=a;break;}}
	}
	for (int a=0;a<256;a++){
		for (int b=0;b<4;b++){if (clusters[a].box[b]>0){clct=a;break;}}
	}

}

char tiletex[262144],blocktex[262144],clustertex[262144];

void BGtileF::RepaintTextures(){
	SDL_Rect tsize;
	//blmaps clmaps
	int sc=(tiwin->GetValue("sc1",0)/84)*2;

	for (int a=0;a<8;a++){
		for (int b=0;b<8;b++){
			int cur=a+((b+sc)*8);
			if (cur<=tict){
				tiles[cur].draw(&fbs[0],ivec2(a*8,b*8));
			}else{https://youtu.be/CfTsvWTtoiQ
				tiles[cur].fill(&fbs[0],ivec2(a*8,b*8),color(64,64,64));
			}
  		}
	}
	fbs[0].update();
	//tsize.x=0;tsize.y=0;tsize.w=256;tsize.h=256;SDL_UpdateTexture(tbuffers[0],&tsize,tiletex,1024);
	int cc=0;
	sc=(tiwin->GetValue("sc2",0)/36)*2;
	for (int a=0;a<8;a++){
		for (int b=0;b<8;b++){
			int cur=(a/2)+(((b+sc)/2)*4);
			if (cur<=grct){
				cc=(a&1)+(((b+sc)&1)*2);
				cc=groups[cur].box[cc];
				tiles[cc].draw(&fbs[1],ivec2(a*8,b*8));
			}else{
				tiles[cc].fill(&fbs[1],ivec2(a*8,b*8),color(64,64,64));
			}
		}
	}
	//tsize.x=0;tsize.y=0;tsize.w=256;tsize.h=256;SDL_UpdateTexture(tbuffers[1],&tsize,blocktex,1024);
	fbs[1].update();
	cc=0;
	int cd=0;
	sc=(tiwin->GetValue("sc3",0) /36)*2;
	for (int a=0;a<8;a++){
		for (int b=0;b<8;b++){
			int cur=(a/2)+(((b+sc)/2)*4);
			if (cur<=clct){
				cc=(a&1)+(((b+sc)&1)*2);
				cc=clusters[cur].box[cc];
				cd=groups[cc].box[0];tiles[cd].draw(&fbs[2],ivec2((a*16),(b*16)));
				cd=groups[cc].box[1];tiles[cd].draw(&fbs[2],ivec2((a*16)+8,(b*16)));
				cd=groups[cc].box[2];tiles[cd].draw(&fbs[2],ivec2((a*16),(b*16)+8));
				cd=groups[cc].box[3];tiles[cd].draw(&fbs[2],ivec2((a*16)+8,(b*16)+8));
			}else{
				cd=groups[cc].box[0];tiles[cd].fill(&fbs[2],ivec2((a*16),(b*16)),color(64));
				cd=groups[cc].box[1];tiles[cd].fill(&fbs[2],ivec2((a*16)+8,(b*16)),color(64));
				cd=groups[cc].box[2];tiles[cd].fill(&fbs[2],ivec2((a*16),(b*16)+8),color(64));
				cd=groups[cc].box[3];tiles[cd].fill(&fbs[2],ivec2((a*16)+8,(b*16)+8),color(64));

			}
		}
	}
	//tsize.x=0;tsize.y=0;tsize.w=256;tsize.h=256;SDL_UpdateTexture(tbuffers[2],&tsize,blocktex,1024);
	fbs[2].update();
	for (int a = 0; a < 511; a++) {
		if (groups[a].box[0] < 0 || groups[a].box[0] > 255) {groups[a].box[0] = 0; groups[a].box[1] = 0; groups[a].box[2] = 0; groups[a].box[3] = 0;}
		if (clusters[a].box[0] < 0 || clusters[a].box[0] > 255) {clusters[a].box[0] = 0; clusters[a].box[1] = 0; clusters[a].box[2] = 0; clusters[a].box[3] = 0;}
	}
}

int BGtileF::Encode(unsigned char * data){
	DH dat;dat.data=data;dat.pos=0;
	dat.wshort(tict);dat.wshort(grct);dat.wshort(clct);
	for (int a=0;a<tict+1;a++){
		for (int b=0;b<64;b++){dat.wbyte(tiles[a].pix[b]);}
	}
	for (int a=0;a<grct+1;a++){
		for (int b=0;b<4;b++){dat.wbyte(groups[a].box[b]);}
	}
	for (int a=0;a<clct+1;a++){
		for (int b=0;b<4;b++){dat.wbyte(clusters[a].box[b]);}
	}
	for (int a = 0; a < grct + 1; a++) {dat.wbyte(bltype[a]);}
	return dat.pos;
}
void BGtileF::Decode(unsigned char * data){
	DH dat;dat.data=data;dat.pos=0;
	tict=dat.rshort();grct=dat.rshort();clct=dat.rshort();
	for (int a=0;a<512;a++){
		memset(tiles[a].pix,0,64);tiles[a].tilesiz = ivec2(8,8);
		for (int b=0; b <4;b++){clusters[a].box[b]=0;groups[a].box[b]=0;}
	}
	for (int a=0;a<tict+1;a++){
		for (int b=0;b<64;b++){tiles[a].pix[b]=dat.rbyte();}
	}
	for (int a=0;a<grct+1;a++){
		for (int b=0;b<4;b++){groups[a].box[b]=dat.rbyte();}
	}
	for (int a=0;a<clct+1;a++){
		for (int b=0;b<4;b++){clusters[a].box[b]=dat.rbyte();}
	}
	for (int a = 0; a < grct + 1; a++) { bltype[a] = dat.rbyte(); }

}
