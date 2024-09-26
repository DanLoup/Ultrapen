#include "MainWindow.h"
#include "data.h"

void LoadBMP();
void SaveBMP();

int curobj=0,curtile=0,curfrtile=0,curframe=0;
void OBtileF::Boot(bool newf){
	obwin=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	lsobj=new Control(obwin,Control::BUTTON,ivec2(100,20+380),ivec2(20,20));lsobj->SetText("<");
	fwobj=new Control(obwin,Control::BUTTON,ivec2(100+100,20+380),ivec2(20,20));fwobj->SetText(">");
	obtype=new Control(obwin,Control::COMBO,ivec2(100,442),ivec2(150,24));obtype->SetText("Pen|Enemy|Pickables|Shots|Mv platforms|Robot Master|Big Boss|Effect|");
	namebox =new Control(obwin,Control::TEXT,ivec2(260,442),ivec2(150,24)); namebox->SetText("Object");
	obsize=new Control(obwin,Control::COMBO,ivec2(420,442),ivec2(150,24));obsize->SetText("1x1|2x1|1x2|2x2|");
	multicolor=new Control(obwin,Control::TOGGLE,ivec2(590,442),ivec2(150,24));multicolor->SetText("Multicolor");
	framesc=new Control(obwin, Control::SCROLL,ivec2(120,368),ivec2(840,20));
	tilesc=new Control(obwin, Control::SCROLL,ivec2(120+288+256,16),ivec2(20,256+8));
	numfra=new Control(obwin,Control::TEXT,ivec2(100,486),ivec2(150,24));numfra->SetText("1");
	delobj = new Control(obwin, Control::BUTTON, ivec2(940, -5), ivec2(100, 20)); delobj->SetText("delete");
	obwin->radioch[0]=0;
	penmodo[0]=new Control(obwin,Control::RADIO,ivec2(10,50),ivec2(100,30));penmodo[0]->SetText("Pen");
	penmodo[1]=new Control(obwin,Control::RADIO,ivec2(10,50+(25*1)),ivec2(100,30));penmodo[1]->SetText("Line");
	penmodo[2]=new Control(obwin,Control::RADIO,ivec2(10,50+(25*2)),ivec2(100,30));penmodo[2]->SetText("Fill");
	penmodo[3]=new Control(obwin,Control::RADIO,ivec2(10,50+(25*3)),ivec2(100,30));penmodo[3]->SetText("Copy");
	for (int a=0;a<4;a++){penmodo[a]->setValue(2,a);}
	for (int a=0;a<2;a++){ofbs[a].Init(rd,ivec2(256,256));}ofbs[2].Init(rd,ivec2(1024,64));
	LoadObjControls();
}

void OBtileF::Run(int mode){
	ivec2 pos=obwin->pos;
	RunPalette(ivec2(100,600),obwin);
	HandleObjDraw();
	HandleFrames();
	RefreshBuffs();
	char text[100];
	obwin->DrawBorderFilled(pos+ivec2(120,20+380),ivec2(80,20),1,color(192));

	if (lsobj->getValue(0)==1 && curobj>0){SaveObjControls();curobj--;LoadObjControls();}
	if (fwobj->getValue(0)==1 && curobj<64){SaveObjControls();curobj++;LoadObjControls();}
	if (multicolor->getValue(0)!=objs[curobj].multic || obsize->getValue(0)!=objs[curobj].siz){SaveObjControls();}
	numfra->GetText(text);
	if (atoi(text)!=objs[curobj].framect){SaveObjControls();}
	Print(pos+ivec2(123,403),color(0),"Object:%i",curobj);
	Print(pos+ivec2(100,425),color(0),"Type:");
	Print(pos+ivec2(260,425),color(0),"Name:");
	Print(pos+ivec2(420,425),color(0),"Size:");
	Print(pos+ivec2(100,470),color(0),"Num frames:");

	obwin->RunWindow();
	strcpy(st_text,"Ctrl + LMB transparent color, shift+LMB third color");
	strcpy(flexmenu,"Object|Import Cur OBJ|Export Cur OBJ|Import BMP|Export BMP");


	if (flexmenuout==3){
		LoadBMP();
	}
	if (flexmenuout==4){
		bmwi=128;
		bmhe=512;
		bmbytes=4;
		for (int a=0;a<512;a++){
			for (int b=0;b<128;b++){
				int tiadd=(b/16)+int(a/16)*8;
                int piadd=(b & 15)+((a & 15)*16);
				int bb=b*bmbytes;
				int d=objs[curobj].objtls[tiadd].pix[piadd]-1;
				if (d>-1){
					bitmem[bb+0+(a*bmwi*bmbytes)]=pals[0].col[d].r;
					bitmem[bb+1+(a*bmwi*bmbytes)]=pals[0].col[d].g;
					bitmem[bb+2+(a*bmwi*bmbytes)]=pals[0].col[d].b;
				}else{
					bitmem[bb+0+(a*bmwi*bmbytes)]=255;
					bitmem[bb+1+(a*bmwi*bmbytes)]=0;
					bitmem[bb+2+(a*bmwi*bmbytes)]=255;
				}
			}
		}
		SaveBMP();
	}
	if (bmdone){
		bmdone=false;
		int he=bmhe;
		int wi=bmwi;
		if (he>512){he=512;}
		if (wi>128){wi=128;}
        for (int a=0;a<he;a++){
			for (int b=0;b<wi;b++){
				int selcol=0,score=100000;
				int bb=b*bmbytes;
				if (!(bitmem[bb+0+(a*bmwi*bmbytes)]==255 && bitmem[bb+1+(a*bmwi*bmbytes)]==0 && bitmem[bb+2+(a*bmwi*bmbytes)]==255)){
					for (int d=0;d<16;d++){
						int misc=abs(pals[0].col[d].r-bitmem[bb+2+(a*bmwi*bmbytes)]);
						misc+=abs(pals[0].col[d].g-bitmem[bb+1+(a*bmwi*bmbytes)]);
						misc+=abs(pals[0].col[d].b-bitmem[bb+0+(a*bmwi*bmbytes)]);
						if (misc<score){score=misc;selcol=d;}
					}
					int tiadd=(b/16)+int(a/16)*8;
					int piadd=(b & 15)+((a & 15)*16);
					objs[curobj].objtls[tiadd].pix[piadd]=selcol+1;
				}else{
					int tiadd=(b/16)+int(a/16)*8;
					int piadd=(b & 15)+((a & 15)*16);
					objs[curobj].objtls[tiadd].pix[piadd]=0;
				}
			}
        }
	}


}

ivec2 stline2;
int colorer2=0,omouse_bt2=0;
void OBtileF::HandleObjDraw(){
	ivec2 pos=obwin->pos;
	ivec2 ps;
	Print(pos+ivec2(120,0),color(0),"Drawing area:");
	obwin->DrawBorderFilled(pos+ivec2(120,16),ivec2(256+8,256+8),2,color(0));
	ivec2 mpos=ivec2((mouse_x-124+(pos.x))/16,(mouse_y-(20+pos.y))/16);

	if (MouseTest(pos+ivec2(124,20),ivec2(252,252))){
		int pmode=penmodo[0]->getValue(0);
		//posp=((mpos.x&8)/8)+(mpos.y&8)+curtl;
		if (pmode==0){
			if (mouse_bt>0){
			int mx=mpos.x,my=mpos.y;
				if (mouse_bt==1){objs[curobj].objtls[curtile].pix[mx+(my*16)]=selL+1;}
				if (mouse_bt==4){objs[curobj].objtls[curtile].pix[mx+(my*16)]=selR+1;}
				if (mouse_bt==2 || (mouse_bt==1 && keyshift>0)){objs[curobj].objtls[curtile].pix[mx+(my*16)]=(selR|selL)+1;}
				if (mouse_bt>0 && keyctrl>0){objs[curobj].objtls[curtile].pix[mx+(my*16)]=0;}
			}
		}

		if (pmode==1){
			ivec2 clmo=ivec2(mpos.x&15,mpos.y&15);
			if (mouse_bt>0 && omouse_bt2==0){
				if (mouse_bt==1){colorer2=selL+1;}
				if (mouse_bt==4){colorer2=selR+1;}
				if (mouse_bt==2 || (mouse_bt==1 && keyshift>0)){colorer2=(selL | selR)+1;}
				if (mouse_bt>0 && keyctrl>0){colorer2=0;}
				stline2=clmo;
			}
			if (mouse_bt==0 && omouse_bt2>0){
				if (abs(stline2.x-clmo.x)>abs(stline2.y-clmo.y)){
					int ini=stline2.x,ed=clmo.x;
					float vec=float(clmo.y-stline2.y)/float(abs(ini-ed)),stvec=stline2.y;
					if (ed<ini){int sw=ed;ed=ini;ini=sw;vec*=-1.0;stvec=clmo.y;}
					for (int a=ini;a<ed+1;a++){
						//int psp=((a&8)/8)+(int(stvec)&8)+curtl;
						objs[curobj].objtls[curtile].pix[(a)+((int(stvec))*16)]=colorer2;
						stvec+=vec;
					}
				}else{
					int ini=stline2.y,ed=clmo.y;
					float vec=float(clmo.x-stline2.x)/float(abs(ini-ed)),stvec=stline2.x;
					if (ed<ini){int sw=ed;ed=ini;ini=sw;vec*=-1.0;stvec=clmo.x;}
					for (int a=ini;a<ed+1;a++){
						//int psp=((int(stvec)&8)/8)+(a&8)+curtl;
						objs[curobj].objtls[curtile].pix[(int(stvec))+((a)*16)]=colorer2;
						stvec+=vec;
					}
				}
			}

		}
		if (pmode==2){
			if (mouse_bt>0 && omouse_bt2==0){

				ivec2 c1[512],c2[512];
				ivec2 rot[]={ivec2(0,-1),ivec2(1,0),ivec2(0,1),ivec2(-1,0)};
				int cc1=1,cc2=0;
				int tacol=objs[curobj].objtls[curtile].pix[(mpos.x)+((mpos.y)*16)],recol;
				if (mouse_bt==1){recol=selL+1;}
				if (mouse_bt==4){recol=selR+1;}
				if (mouse_bt==2 || (mouse_bt==1 && keyshift>0)){recol=(selL | selR)+1;}
				if (mouse_bt>0 && keyctrl>0){recol=0;}

				objs[curobj].objtls[curtile].pix[(mpos.x)+((mpos.y)*16)]=recol;
				c1[0]=ivec2(mpos.x,mpos.y);
				bool filled=false;
				while (!filled){
					cc2=0;
					filled=true;
					for (int a=0;a<cc1;a++){
						for (int b=0;b<4;b++){
							ivec2 tt=rot[b]+c1[a];
							if (tt.x>-1 && tt.x<16 && tt.y>-1 && tt.y<16){
								if (objs[curobj].objtls[curtile].pix[tt.x+(tt.y*16)]==tacol){
									filled=false;objs[curobj].objtls[curtile].pix[tt.x+(tt.y*16)]=recol;c2[cc2++]=tt;
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
		omouse_bt2=mouse_bt;
	}



	int col=0;
	int mc=multicolor->getValue(0);
	for (int a=0;a<16;a++){
		for (int b=0;b<16;b++){
			if (objs[curobj].objtls[curtile].pix[a+(b*16)]>0){
				col=objs[curobj].objtls[curtile].pix[a+(b*16)]-1;
				Box(pos+ivec2((a*16)+124,(b*16)+20),pos+ivec2((a*16)+140,(b*16)+36),pals[0].col[col]);
			}else{
				Box(pos+ivec2((a*16)+128,(b*16)+24),pos+ivec2((a*16)+136,(b*16)+32),color(64));
			}
		}
	}
	for (int a=0;a<16;a++){ps=pos+ivec2(124,20);Line(ps+ivec2(a*16,0),ps+ivec2(a*16,255),color(128));Line(ps+ivec2(0,a*16),ps+ivec2(255,a*16),color(128));}
	int ccs[24],ctt=0;
	for (int b=0;b<16;b++){
		ctt=0;
		bool strike=false;
		for (int a=0;a<16;a++){
			bool newcol=true;
			int cc=objs[curobj].objtls[curtile].pix[a+(b*16)]-1;
			if (ctt>0){
				for (int c=0;c<ctt;c++){
                    if (ccs[c]==cc){newcol=false;break;}
				}
			}
			if (newcol && cc!=-1){ccs[ctt++]=cc;}
		}
		if (mc==0 && ctt>1){strike=true;}
		if (mc>0){
			if (ctt>3){strike=true;}
			if (ctt==3){
				strike=true;
				if ((ccs[0]|ccs[1])==ccs[2]){strike=false;}
				if ((ccs[1]|ccs[2])==ccs[0]){strike=false;}
				if ((ccs[0]|ccs[2])==ccs[1]){strike=false;}
			}
		}
		if (strike){
			Box(pos+ivec2(124,(b*16)+27),pos+ivec2(124+256,(b*16)+29),color(192,0,0));
		}
	}

	bool filled=false;
	for (int a=511;a>-1;a--){
		filled=false;
		for (int x=0;x<256;x++){
			if (objs[curobj].objtls[a].pix[x]>0){filled=true;break;}
		}
        if (filled){objs[curobj].tilct=a+1;break;}
	}
	if (delobj->value[0] == 1) {
		for (int a = curobj; a < objct;a++) {objs[a] = objs[a + 1];}
		objct--;
	}

}

void OBtileF::HandleFrames(){
	ivec2 pos=obwin->pos;
	ivec2 ps;
	Print(pos+ivec2(120+280,0),color(0),"16x16 tiles:");
	obwin->DrawBorderFilled(pos+ivec2(120+280,16),ivec2(256+8,256+8),2,color(0));

	ivec2 pps=pos+ivec2(124+280,20);
	//SDL_Rect sc,prs;sc.x=0;sc.y=0;sc.w=128;sc.h=128;prs.x=pps.x;prs.y=pps.y;prs.w=256;prs.h=256;
	//SDL_RenderCopy(rd,obuffers[0],&sc,&prs);
	ofbs[0].drawclipped(pps,2.0,ivec2(128,128));
	for (int a=0;a<8;a++){ps=pos+ivec2(124+280,20);Line(ps+ivec2(a*32,0),ps+ivec2(a*32,255),color(128));Line(ps+ivec2(0,a*32),ps+ivec2(255,a*32),color(128));}
	ivec2 ctlpos=ivec2((curtile&7)*32,((curtile/8))*32);
	int spos=tilesc->getValue(0)/128;ctlpos.y-=spos*32;
	if (ctlpos.y>-1 && ctlpos.y<256){
		Contour(pos+ivec2(124+280,20)+ctlpos,ivec2(32,32),color(255,0,0));
	}
	if (MouseTest(pos+ivec2(124+280,14),ivec2(252,252))){
		int mx=(mouse_x-(124+280+pos.x))/32,my=(mouse_y-(20+pos.y))/32;
		if (mouse_bt==1){curtile=mx+(my*8);}
		if (mouse_bt>1){objs[curobj].frames[curframe][curfrtile]=mx+(my*8);}
	}

	Print(pos+ivec2(120+570,0),color(0),"Frame Assembler:");
	obwin->DrawBorderFilled(pos+ivec2(120+570,16),ivec2(256+8,256+8),2,color(0));
	pps=pos+ivec2(124+570,20);

	//sc.x=0;sc.y=0;sc.w=32;sc.h=32;prs.x=pps.x;prs.y=pps.y;prs.w=256;prs.h=256;
	//if (objs[curobj].siz==0){sc.w=16;sc.h=16;}
	if (objs[curobj].siz==0){
		ofbs[1].drawclipped(pps,16.0,ivec2(16,16));
	}else{
		ofbs[1].drawclipped(pps,8.0,ivec2(32,32));
	}
	
	//SDL_RenderCopy(rd,obuffers[1],&sc,&prs);

	int vx=curfrtile & 1,vy=(curfrtile/2)&1;
	Contour(pos+ivec2(124+570+(vx*128),20+(vy*128)),ivec2(128,128),color(255,0,0));
	if (MouseTest(pos+ivec2(124+570,14),ivec2(252,252))){
		if (mouse_bt==1){int mx=(mouse_x-(124+570+pos.x))/128,my=(mouse_y-(20+pos.y))/128;curfrtile=mx+(my*2);}
	}


	Print(pos+ivec2(120+300,0+280),color(0),"frames:");
	obwin->DrawBorderFilled(pos+ivec2(120,16+280),ivec2(832+8,64+8),2,color(0));
	pps=pos+ivec2(124,284+16);
	//sc.x=0;sc.y=0;sc.w=416;sc.h=32;prs.x=pps.x;prs.y=pps.y;prs.w=832;prs.h=64;
	//if (objs[curobj].siz==0){sc.w=416/2;sc.h=16;}
	//SDL_RenderCopy(rd,obuffers[2],&sc,&prs);
	if (objs[curobj].siz==0){
		ofbs[2].drawclipped(pps,4,ivec2(416/2,16));
	}else{
		ofbs[2].drawclipped(pps,2,ivec2(416,32));
	}
	for (int a=0;a<16;a++){ps=pos+ivec2(124,20+280);Line(ps+ivec2(a*64,0),ps+ivec2(a*64,64),color(128));}

	int pvs=(curframe-(framesc->getValue(0)/64));
	if (pvs>-1 && pvs<13){
		Contour(pos+ivec2(124+(pvs*64),20+280),ivec2(64,64),color(255,0,0));
	}
	if (MouseTest(pos+ivec2(124,14+280),ivec2(892,64))){
		if (mouse_bt==1){
			int mx=(mouse_x-(124+pos.x))/64;
			curframe=mx+(framesc->getValue(0)/64);
			if (curframe>63){curframe=63;}
		}
	}
}

void OBtileF::RefreshBuffs(){
	OBtileF * cts=(OBtileF*)fils[curfile];
	SDL_Rect tsize;
	int sc=(tilesc->getValue(0)/128)*2;
	int sll=0;
	for (int a=0;a<8;a++){
		for (int b=0;b<8;b++){
			sll=a+((b+sc)*8);
			if (sll<objs[curobj].tilct){
				objs[curobj].objtls[sll].draw(&ofbs[0],ivec2(a*16,b*16));
			}else{
				objs[curobj].objtls[sll].fill(&ofbs[0],ivec2(a*16,b*16),color(64));
			}
		}
	}
	int xsiz=2,ysiz=2;
	xsiz=1+(objs[curobj].siz &1);
	ysiz=1+((objs[curobj].siz &2)/2);

	for (int a=0;a<2;a++){
		for (int b=0;b<2;b++){
			if (a>xsiz-1 || b>ysiz-1){
				objs[curobj].objtls[63].draw(&ofbs[1],ivec2(a*16,b*16));
			}else{
				int pt=objs[curobj].frames[curframe][a+(b*2)];
				objs[curobj].objtls[pt].draw(&ofbs[1],ivec2(a*16,b*16));
			}
		}
	}

	for (int c=0;c<16;c++){
		int ccf=c+(framesc->getValue(0)/64);
		int cc=c*2;
		xsiz=1+(objs[curobj].siz &1);
		ysiz=1+((objs[curobj].siz &2)/2);
		if (objs[curobj].siz==0){cc=c;}
		for (int a=0;a<2;a++){
			for (int b=0;b<2;b++){
				if (a>xsiz-1 || b>ysiz-1 || ccf>=objs[curobj].framect){

					objs[curobj].objtls[63].fill(&ofbs[2],ivec2((a+(cc))*16,b*16),color(64));

				}else{
					int pt=objs[curobj].frames[ccf][a+(b*2)];
					objs[curobj].objtls[pt].draw(&ofbs[2],ivec2((a+(cc))*16,b*16));

				}
			}
		}
	}
	ofbs[0].update();ofbs[1].update();ofbs[2].update();

	//tsize.x=0;tsize.y=0;tsize.w=256;tsize.h=256;SDL_UpdateTexture(obuffers[0],&tsize,tltex,1024);
	//tsize.x=0;tsize.y=0;tsize.w=256;tsize.h=256;SDL_UpdateTexture(obuffers[1],&tsize,astex,1024);
	//tsize.x=0;tsize.y=0;tsize.w=1024;tsize.h=64;SDL_UpdateTexture(obuffers[2],&tsize,fratex,4096);
}

void OBtileF::LoadObjControls(){
	char tex[100];

	//snprintf(tex,100,"%i",objs[curobj].subtype);
	obtype->setValue(0,objs[curobj].type);
	namebox->SetText(objs[curobj].name);
	//obsubtype->SetText(tex);
	obsize->setValue(0,objs[curobj].siz);
	multicolor->setValue(0,objs[curobj].multic);
	snprintf(tex,100,"%i",objs[curobj].framect);
	numfra->SetText(tex);
}
void OBtileF::SaveObjControls(){
	OBtileF * cts=(OBtileF*)fils[curfile];
	char tex[100];
	objs[curobj].type=obtype->getValue(0);
	namebox->GetText(objs[curobj].name);
	objs[curobj].siz=obsize->getValue(0);
	objs[curobj].multic=multicolor->getValue(0) & 1;
	numfra->GetText(tex);
	objs[curobj].framect=atoi(tex);
}

int OBtileF::Encode(unsigned char* data){
	DH dat;dat.data=data;dat.pos=0;
	unsigned short nuob=objct;
	nuob=0;
	for (int a=0;a<512;a++){
		if (objs[a].tilct>0){nuob++;}
	}
	dat.wshort(nuob);

	for (int a=0;a<nuob;a++){
		if (objs[a].tilct>0){
			for (int b = 0; b < 64; b++) {dat.wbyte(objs[a].name[b]);}
			dat.wshort(objs[a].type);
			dat.wshort(objs[a].subtype);
			dat.wshort(objs[a].siz);
			dat.wshort(objs[a].multic);
			dat.wshort(objs[a].framect);
			dat.wshort(objs[a].tilct);
			for (int b=0;b<objs[a].tilct+1;b++){
				for (int c=0;c<256;c++){
					dat.wbyte(objs[a].objtls[b].pix[c]);
				}
			}
			for (int b=0;b<objs[a].framect;b++){
				for (int c=0;c<4;c++){
					dat.wbyte(objs[a].frames[b][c]);
				}
			}
		}
	}
	return dat.pos;
}

void OBtileF::Decode(unsigned char* data){
	DH dat;dat.data=data;dat.pos=0;
	objct=dat.rshort();
	for (int a=0;a<objct;a++){
		for (int b = 0; b < 64; b++) { objs[a].name[b]=dat.rbyte(); }
		objs[a].type=dat.rshort();
		objs[a].subtype=dat.rshort();
		objs[a].siz=dat.rshort();
		objs[a].multic=dat.rshort();
		objs[a].framect=dat.rshort();
		objs[a].tilct=dat.rshort();

		for (int b=0;b<512;b++){memset(objs[a].objtls[b].pix,0,256);}

		for (int b=0;b<objs[a].tilct+1;b++){
			for (int c=0;c<256;c++){objs[a].objtls[b].pix[c]=dat.rbyte();}
		}
		for (int b=0;b<objs[a].framect;b++){
			for (int c=0;c<4;c++){objs[a].frames[b][c]=dat.rbyte();}
		}
		if (objs[a].framect == 0) { objs[a].framect = 1; objs[a].frames[0][0]=0; }
	}
	for (int a = objct; a < 511; a++) {
		objs[a].framect = 0; objs[a].multic= 0; objs[a].tilct = 0; objs[a].type = 0; objs[a].subtype= 0; objs[a].siz = 0;
		memset(objs[a].frames, 0, 256);
		sprintf(objs[a].name,"Obj%i", a);
	}
}
