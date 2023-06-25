#include "Gui.h"

//Helper functions and rasterers
SDL_Texture * LoadFont (const char * ftn){
	SDL_Texture * font=SDL_CreateTexture(rd,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STATIC,144,96);

	SDL_Surface * surf = SDL_LoadBMP(ftn);
	SDL_LockSurface(surf);
	int ct=surf->pitch/surf->w,ctt=0,cdd=0;
	unsigned char fontgfx[256*96*4];
		unsigned char * pixacc=(unsigned char*)surf->pixels;
		for (int b=0;b<surf->h;b++){
			for (int a=0;a<surf->w;a++){
				fontgfx[cdd++]=pixacc[ctt];fontgfx[cdd++]=255;fontgfx[cdd++]=255;fontgfx[cdd++]=255;
				ctt+=ct;
			}
	}
	SDL_UnlockSurface(surf);
	SDL_Rect up;up.x=0;up.y=0;up.w=144;up.h=96;
	SDL_UpdateTexture(font,&up,fontgfx,1024);
	SDL_SetTextureBlendMode(font,SDL_BLENDMODE_BLEND);
	SDL_SetTextureColorMod(font,255,255,255);
	return font;
}

void Line(ivec2 p1,ivec2 p2,color col){
	SDL_SetRenderDrawColor(rd,col.r,col.g,col.b,col.a);
	SDL_RenderDrawLine(rd,p1.x,p1.y,p2.x,p2.y);
}
void Box(ivec2 p1,ivec2 p2,color col){
	SDL_SetRenderDrawColor(rd,col.r,col.g,col.b,col.a);
	SDL_Rect ret;
	ret.x=p1.x;ret.y=p1.y;ret.w=p2.x-p1.x;ret.h=p2.y-p1.y;
	SDL_RenderFillRect(rd,&ret);
}

void Contour(ivec2 pos,ivec2 siz,color col){
	Line(pos,pos+ivec2(siz.x,0),col);
	Line(pos+ivec2(siz.x,0),pos+ivec2(siz.x,siz.y),col);
	Line(pos+ivec2(siz.x,siz.y),pos+ivec2(0,siz.y),col);
	Line(pos+ivec2(0,siz.y),pos,col);
}

ivec2 lcini,lcend;
bool mtlocked=false;
bool MouseTest(ivec2 pos,ivec2 siz){
	bool aloc=true;
	if (mtlocked){
		if (mouse_x>lcini.x && mouse_x<lcend.x && mouse_y>lcini.y && mouse_y<lcend.y){aloc=false;}
	}
    if (mtlocked && mouse_bt==0){mtlocked=false;}
    return (mouse_x>pos.x && mouse_x<pos.x+siz.x) && (mouse_y>pos.y && mouse_y<pos.y+siz.y) & aloc;

}

void AreaLock(ivec2 ini,ivec2 ed){
	lcini=ini;lcend=ed;
	mtlocked=true;
}
void AreaUnlock(){mtlocked=false;}

SDL_Texture * fnt;
void Print(ivec2 pos, color col,char * text,...){
	char ibuff[1024];
    va_list args;
    va_start(args,text);
    vsprintf(ibuff,text,args);
	va_end(args);

	SDL_Rect sc,ps;
	ps.x=pos.x;ps.y=pos.y;
	ps.w=9;ps.h=16;sc=ps;
	char ch;
	SDL_SetTextureColorMod(fnt,col.r,col.g,col.b);

	for (int a=0;a<strlen(ibuff);a++){
		if (ibuff[a]>32){
			ch=ibuff[a]-33;
			sc.x=(ch % 16)*9;sc.y=((ch/16) % 16)*16;
			SDL_RenderCopy(rd,fnt,&sc,&ps);
		}
		ps.x+=8;
	}
}

void Print(ivec2 pos, color col,const char * text,...){
	char ibuff[1024];
    va_list args;
    va_start(args,text);
    vsprintf(ibuff,text,args);
    va_end(args);

	SDL_Rect sc,ps;
	ps.x=pos.x;ps.y=pos.y;
	ps.w=9;ps.h=16;sc=ps;
	char ch;
	SDL_SetTextureColorMod(fnt,col.r,col.g,col.b);

	for (int a=0;a<strlen(ibuff);a++){
		if (ibuff[a]>32){
			ch=ibuff[a]-33;
			sc.x=(ch % 16)*9;sc.y=((ch/16) % 16)*16;
			SDL_RenderCopy(rd,fnt,&sc,&ps);
		}
		ps.x+=8;
	}
}

Control * ctcon;
bool kben=false;
int curpos=0;
std::string * kbkon;
void ConnectKB(Control * pointer){
		if (kben && ctcon){ctcon->textfocus=false;}
		ctcon = pointer;
		kbkon=&pointer->text;
		ctcon->textfocus=true;
		kben=true;
}

void ConnectKBD(std::string * pointer){
		if (kben && ctcon){ctcon->textfocus=false;}
		kbkon=pointer;
		kben=true;
}


void UnconnectKB(){
	if (kben && ctcon){ctcon->textfocus=false;}
	kben=false;
}

void RunKeyboard(){
	char tt[] = "1234567890-=,.;/[]'";
	char tto[] = "!@#$%`&*()_+<>:?{}\"";
	if (kben){
		char t[2];
		//char * kbcon=ctcon->text;
		std::string *kbcon=kbkon;
		int ucase=0;
		if (keys[tsdl(SDLK_LSHIFT)]>0 || keys[tsdl(SDLK_RSHIFT)]>0){ucase=-32;}
		for (int a=32;a<160;a++){
			if (keys[a]>0 && keycombo[a]==15 || keycombo[a]==1){
				if (a<128){
					t[0]=a;t[1]=0;
					if (a>95 && a<124){t[0]+=ucase;}
					if (a==59 && ucase<0){t[0]=58;} //Shift
					if (ucase<0 && a < 65){
						for (int b = 0;b < strlen(tt);b++){
							if (a==tt[b]){t[0]=tto[b];break;}
						}
					}
					kbcon->insert(curpos,t);curpos++;
					if (keycombo[a]==1){keycombo[a]=3;}
				}
				if (a==tsdl(SDLK_BACKSPACE)){int sz=kbcon->size();if (sz>0 && curpos>0){kbcon->erase(kbcon->begin()+curpos-1);curpos--;}}
				if (a==tsdl(SDLK_DELETE) && kbcon->size()>0){kbcon->erase(kbcon->begin()+curpos);}
				if (keycombo[a]>1){keycombo[a]--;}
				if (a==tsdl(SDLK_LEFT) && curpos>0){curpos--;}
				if (a==tsdl(SDLK_RIGHT) && curpos<kbcon->size()){curpos++;}
			}
		}
	}
}

int tsdl(int tsdl){
	int mu=0;
	for (int a=0;a<14;a++){if (spmask[a]==tsdl){mu=a+128;break;}}
	return mu;
}


//Window API
Window::Window(SDL_Renderer * render,int maxct,ivec2 ipos,ivec2 isiz){
   rd=render;
   //controls=new Control*[maxct];
   conct=0;
   int cc=0;
   for (int a = 0;a < 4; a++){shades[a] = color(cc);cc+=64; }
   shades[3]=color(255);
//   for (int a=0;a<8;a++){buffen[a]=0;}
   pos=ipos;siz=isiz;
}

void Window::DrawBorderLine(ivec2 v1,ivec2 v2,bool raised,bool thick){
	ivec2 vect=v2-v1;
	if (abs(vect.x)>abs(vect.y)){vect.y=0;}else{vect.x=0;}
    if (vect.x<0){v1.x=v2.x;}if (vect.y<0){v1.y=v2.y;}
    vect.x=abs(vect.x);vect.y=abs(vect.y);
	ivec2 nrm=ivec2(vect.y,vect.x*1);float siz=sqrt(nrm.x*nrm.x+nrm.y*nrm.y);nrm=nrm/siz;
    int cols[3]={2,0,3};
    if (raised){cols[0]=3;cols[1]=2;cols[2]=1;}
   if (thick){
		Box(v1-nrm*3,v1+vect-nrm,shades[cols[0]]);
		Box(v1-nrm,v1+vect+nrm,shades[cols[1]]);
		Box(v1+nrm,v1+vect+nrm*3,shades[cols[2]]);
   }else{
		Box(v1-nrm,v1+vect,shades[cols[0]]);
		Box(v1,v1+vect+nrm,shades[cols[1]]);
		Box(v1+nrm,v1+vect+nrm*2,shades[cols[2]]);
   }
}

void Window::DrawBorder(ivec2 pos,ivec2 siz,int btype){
    ivec2 corners[]={ivec2(pos.x,pos.y),ivec2(pos.x+siz.x,pos.y),ivec2(pos.x+siz.x,pos.y+siz.y),ivec2(pos.x,pos.y+siz.y)};
    ivec2 nt[]={ivec2(1,1),ivec2(-1,1),ivec2(-1,-1),ivec2(1,-1)};
    int tables[]={
       2,2,3,3,2, 2,0,0,0,0, 2,3,2,2,3, 1,0,0,0,0, 0,0,0,0,0, 2,3,2,2,3,
       1,0,0,0,0, 0,0,0,0,0, 3,2,3,3,2, 2,3,1,1,3, 2,2,2,2,2, 2,1,3,3,1,
    };
    int ct=btype*15;
    for (int a=0;a<3;a++){
        int siz=tables[ct++];
        for (int b=0;b<4;b++){
            Box(corners[b],corners[(b + 1)% 4]+nt[(b + 1) %4]*siz,shades[tables[ct++]]);
        }
        for (int b=0;b<4;b++){corners[b]=corners[b]+nt[b]*siz;}
    }
}
void Window::DrawBorderFilled(ivec2 pos,ivec2 siz,int btype,color fil){
    Box(pos,pos+siz,fil);
    DrawBorder(pos,siz,btype);
}

void Window::AddBuffer(int id,SDL_Texture * bf, ivec2 sizv){
//    buffen[id]=1;buffers[id]=bf;buffsiz[id]=sizv;
}

void Window::RunScroll(ivec2 pos,ivec2 siz,int * value){
	ivec2 block,midbl,lastbl,holest,holeed,shai,shad;
	int moupos,blopo;
	int cpos,p1=0,p2=0;
	float csize;
	if (siz.x>siz.y){
		//Horizontal scroll
		csize=(1023.0/float(siz.x-(siz.y*3)));cpos=value[0]/csize;
		block=ivec2(siz.y,siz.y);midbl=pos+ivec2(siz.y+cpos,0);lastbl=pos+ivec2(siz.x-siz.y,0);
		moupos=mouse_x;blopo=pos.x+siz.y+cpos+(siz.y/2);
		holest=pos+ivec2(siz.y,0);holeed=siz-ivec2(siz.y*2,0);
		shai=midbl+ivec2(0,1);shad=midbl+block+ivec2(3,-1);
	}else{
		//Vertical scroll
		csize=(1023.0/float(siz.y-(siz.x*3)));cpos=value[0]/csize;
		block=ivec2(siz.x,siz.x);midbl=pos+ivec2(0,siz.x+cpos);lastbl=pos+ivec2(0,siz.y-siz.x);
		moupos=mouse_y;blopo=pos.y+siz.x+cpos+(siz.x/2);
		holest=pos+ivec2(0,siz.x);holeed=siz-ivec2(0,siz.x*2);
		shai=midbl+ivec2(1,0);shad=midbl+block+ivec2(-1,3);
	}
	if (mouse_bt==1 && MouseTest(pos,block) && value[0]>0){value[0]-=4;p1=1;}
	if (mouse_bt==1 && MouseTest(lastbl,block) && value[0]<1024){value[0]+=4;p2=1;}
	if (MouseTest(midbl,block)){if (mouse_bt==1 && value[1]==0){value[1]=1;value[2]=moupos;value[3]=value[0];}}
	if (mouse_bt==1 && value[1]==1){value[0]=((moupos-value[2])*csize)+value[3];}
	if (mouse_bt==0 && value[1]==1){value[1]=0;}
   if (mouse_bt==1 && value[1]==0 && MouseTest(holest,holeed)){if (moupos<blopo){value[0]-=32;}else{value[0]+=32;}}
   if (value[0]<0){value[0]=0;}if (value[0]>1023){value[0]=1023;}
   DrawBorderFilled(pos,siz,2,color(160));
   Box(shai,shad,color(128));DrawBorderFilled(midbl,block,1,color (192));
   DrawBorderFilled(pos,block,1+p1,color (192));DrawBorderFilled(lastbl,block,1+p2,color (192));
}
void Window::RunWindow(){
	int mouseowner=0;
	bool mouseen=false;
	for (int b=2;b>-1;b--){
		for (int a=0;a<conct;a++){
			if (controls[a]->priority==b){
				if (MouseTest(controls[a]->pos,controls[a]->siz)){
					mouseowner=a;goto foundowner;
				}
			}
		}
	}
	foundowner:;
	for (int b=0;b<2;b++){
		for (int a=0;a<conct;a++){
			if (a==mouseowner){mouseen=true;}else{mouseen=false;}
			if (controls[a]->priority==b){controls[a]->RunControl(this,mouseen);}
		}
	}
}
void Window::RegisterControl(Control * ct){controls.push_back(ct);conct++;}

int Window::findct(char * name){
	int ret=-1;
	for (int a=0;a<conct;a++){
		if (strcmp(name,controls[a]->name)==0){ret=a;break;}
	}
	return ret;
}
void Window::AddControl(char * name,Control::ctrl cttype,ivec2 pos,ivec2 siz){
	controls.push_back(new Control(cttype,pos,siz));
	strcpy(controls[conct]->name,name);
	conct++;
}
void Window::AddControl(char * name,Control::ctrl cttype,ivec2 pos,ivec2 siz,char * initext){
	AddControl(name,cttype,pos,siz);
	controls[conct-1]->text=initext;
	//strcpy_s(controls[conct-1]->text,controls[conct-1]->texsize,initext);
}
void Window::AddControl(char * name,Control::ctrl cttype,ivec2 pos,ivec2 siz,int inival){
	AddControl(name,cttype,pos,siz);
	controls[conct-1]->value[0]=inival;
}
int Window::GetValue(char * name,int valid){
	int nb=findct(name),rt=-1;
	if (nb>-1){rt=controls[nb]->value[valid];}
	return rt;
}
void Window::SetValue(char *name,int valid,int valnum){
	int nb=findct(name);
	if (nb>-1){controls[nb]->value[valid]=valnum;}
}
void Window::GetText(char *name,char * text){
	int nb=findct(name);if (nb>-1){controls[nb]->text.copy(text,controls[nb]->text.size(),0);text[controls[nb]->text.size()]=0;
	}
}
int Window::GetTextAsNumber(char *name){
	int nb=findct(name),val=0;
	if (nb>-1){val=atoi(controls[nb]->text.c_str());}
	return val;
}
void Window::SetText(char *name,char * text){
	int nb=findct(name);
	if (nb>-1){controls[nb]->SetText(text);}
	//if (nb>-1){strcpy_s(controls[nb]->text,controls[nb]->texsize,text);}
}
void Window::SetNumberAsText(char *name,int number){
	int nb=findct(name);
	char text[100];snprintf(text,100,"%i",number);
	if (nb>-1){controls[nb]->SetText(text);}

}
void Window::SetBitmap(char *name, char *bmp){
	int nb=findct(name);
	if (nb>-1){
		SDL_Rect tsize;
		tsize.x=0;tsize.y=0;tsize.w=controls[nb]->texdim.x;tsize.h=controls[nb]->texdim.y;SDL_UpdateTexture(controls[nb]->texture,&tsize,bmp,tsize.w*4);
	}
}
ivec2 Window::GetSize(char* name) { return ivec2(0, 0); }

std::vector <sstate> Window::SaveState(){
	std::vector <sstate> st;
	for (int a = 0;a < controls.size();a++){
		st.push_back(sstate());
		st.back().text = controls[a]->text;
		for (int b = 0;b < 6; b++){st.back().value[b] = controls[a]->value[b];}
	}
	return st;
}

void Window::LoadState(std::vector<sstate> din){
	for (int a = 0;a < din.size();a++){
		controls[a]->text=din[a].text;
		for (int b = 0;b < 6; b++){controls[a]->value[b]=din[a].value[b];}
	}
}


Control::Control(Window *win,ctrl tp,ivec2 posi,ivec2 size){
    pos=posi;siz=size;
    type=tp;
    priority=0;
    for (int a=0;a<6;a++){value[a]=0;}texsize=0;
    if (tp==Control::MENU){priority=1;}
    if (tp==Control::COMBO){priority=1;}

	textfocus=false;focus=false;
    win->RegisterControl(this);
}

Control::Control(ctrl tp,ivec2 posi,ivec2 sizi){
    pos=posi;siz=sizi;
    type=tp;
    priority=0;
    for (int a=0;a<6;a++){value[a]=0;}texsize=0;
	if (tp==Control::MENU){priority=1;}
    if (tp==Control::COMBO){priority=1;}

	textfocus=false;focus=false;
//    win->RegisterControl(this);
}

Control::~Control(){

}
void Control::RunControl(Window * inwin,bool butenable){
	ivec2 rpos=pos+inwin->pos;
    if (type==Control::BUTTON){
		int press=0;
		if (MouseTest(rpos,siz) && mouse_bt>0){press=1;}
        inwin->DrawBorderFilled(rpos,siz,1+press,color(192));
        int tesiz=(text.size()*8)/2;
        ivec2 mico=siz/2;
        Print(rpos+ivec2(mico.x-tesiz,mico.y-8),color(0),text.c_str());
        value[1]=0;if (mouse_edup==1 && MouseTest(rpos,siz)){value[1]=1;}
		if (value[1]>0 && value[0]<30){value[0]++;}if (value[1]==0){value[0]=0;}
    }

    if (type==Control::SCROLL){
        inwin->RunScroll(rpos,siz,&value[0]);
    }

    if (type==Control::TOGGLE){
        inwin->DrawBorderFilled(rpos,siz,1+value[0],color(192));
        int tesiz=(text.size()*8)/2;
        ivec2 mico=siz/2;
        Print(rpos+ivec2(mico.x-tesiz,mico.y-8),color(0),text.c_str());
        if (mouse_edup==1 && value[1]==0 && MouseTest(rpos,siz)){value[0]=1-value[0];value[1]=1;}
        if (mouse_bt==0){value[1]=0;}
    }

    if (type==Control::RADIO){
        int pressgfx=0;
        if (inwin->radioch[value[1]]==value[2]){pressgfx=2;}
        if (pressgfx>0){
            inwin->DrawBorderFilled(rpos+ivec2(1,1),siz-ivec2(2,2),2,color(192));
        }else{
            inwin->DrawBorderFilled(rpos,siz,0,color(192));
        }
        int tesiz=(text.size()*8)/2;
        ivec2 mico=siz/2;
        Print(rpos+ivec2(mico.x-tesiz,mico.y-8),color(0),text.c_str());
        if (mouse_bt==1 && MouseTest(rpos,siz)){inwin->radioch[value[1]]=value[2];}
        value[0]=inwin->radioch[value[1]];
    }

    if (type==Control::LABEL){Print(rpos,color(0),text.c_str());}

    if (type==Control::MENU){
        int txcount=0,lrpos=0,ttsiz=0;
        ivec2 lsiz;
        char mnutxt[64];int mnurpos[64];
        for (int a=0;a<text.size();a++){
            if (text[a]=='|'){
                if (txcount==0){
                    text.copy(mnutxt,a,0);mnutxt[a]=0;
                    if (value[1]==1){Box(rpos,rpos+ivec2(strlen(mnutxt)*8,20),color(128));}ttsiz=strlen(mnutxt);Print(rpos+ivec2(2,2),color(0),mnutxt);
                }
                mnurpos[txcount++]=lrpos;lrpos=a;
            }
        }
        lsiz=ivec2(ttsiz*16,20);mnurpos[txcount++]=lrpos;mnurpos[txcount]=text.size();
        if (mouse_bt==1 && MouseTest(rpos,ivec2(ttsiz*16,20))){value[1]=2;}
        if (value[1]>0){
            bool exit=false;
            AreaUnlock();
            inwin->DrawBorderFilled(rpos+ivec2(0,20),ivec2(200,(txcount*16)-8),1,color(192));
            lsiz=ivec2(200,(txcount*16)-8);
            int sel=0;
            if (MouseTest(rpos+ivec2(0,20),ivec2(200,(txcount*16)-8))){
                sel=(mouse_y-(rpos.y+24))/16;if (sel>(txcount-2)){sel=txcount-2;}
                Box(rpos+ivec2(2,24+sel*16),rpos+ivec2(197,40+sel*16),color(128));
                sel++;
            }
            if (mouse_edup == 1 && value[1]==1){value[0]=sel;value[1]=0;exit=true;}
            if (mouse_bt==0 && value[1]==2){value[1]=1;}
            for (int a=1;a<txcount;a++){
                int sz=mnurpos[a+1]-mnurpos[a];
                memcpy(mnutxt,&text[mnurpos[a]+1],sz-1);
                mnutxt[sz-1]=0;Print(rpos+ivec2(3,(a*16)+8),color(0),mnutxt);
            }
            siz=lsiz;
			AreaLock(rpos+ivec2(0,20),rpos+ivec2(0,20)+ivec2(200,(txcount*16)-8));
        }
    }

    if (type==Control::PICTURE){
        inwin->DrawBorderFilled(rpos,siz,2,color(192));
		SDL_Rect sc,prs;sc.x=0;sc.y=0;sc.w=texdim.x;sc.h=texdim.y;prs.x=rpos.x+4;prs.y=rpos.y+4;prs.w=texdim.x;prs.h=texdim.y;
		SDL_RenderCopy(rd,texture,&sc,&prs);
        value[3]=0;if (MouseTest(rpos,siz)){value[1]=mouse_x-rpos.x;value[2]=mouse_y-rpos.y;value[3]=mouse_bt;}

    }
    if (type==Control::EPICTURE){
        inwin->DrawBorderFilled(rpos,siz,2,color(192));
    }

    if (type==Control::COMBO){
        inwin->DrawBorderFilled(rpos,ivec2(siz.x,24),2,color(255));
        inwin->DrawBorderFilled(rpos+ivec2(siz.x-24,0),ivec2(24,24),1,color(192));
        ivec2 sizu=ivec2(siz.x,24);
        char cbtxt[64];int listrpos[64],icount=0,sz=0;
        listrpos[icount++]=0;
        for (int a=0;a<text.size();a++){if (text[a]=='|'){listrpos[icount++]=a+1;}}
        listrpos[icount++]=text.size();if (value[0] > icount-3) { value[0] = icount-3; }
		if (value[0] < 0) { value[0] = 0;}
		sz = (listrpos[value[0] + 1] - listrpos[value[0]]) - 1;

		//memcpy_s(cbtxt,64,&text[listrpos[value[0]]],sz);cbtxt[sz]=0;
		text.copy(cbtxt,sz,listrpos[value[0]]);cbtxt[sz]=0;
        Print(rpos+ivec2(6,5),color(0),cbtxt);
        if (MouseTest(rpos+ivec2(siz.x-24,0),ivec2(24,24)) && mouse_edup==1){value[1]=2;}
        if (value[1]>0){
            sizu=ivec2(siz.x,224);
            inwin->DrawBorderFilled(rpos+ivec2(0,24),ivec2(siz.x,200),1,color(255));
            if (MouseTest(rpos+ivec2(0,24),ivec2(siz.x,200))){
                int sl=(mouse_y-(rpos.y+24))/16;
                if (sl>icount-3){sl=icount-3;}
                Box(rpos+ivec2(4,(sl*16)+24),rpos+ivec2(siz.x,(sl*16)+40),color(128));
                if (value[1]==1 && mouse_bt==1){value[0]=sl;value[1]=0;}
            }else{
                if (mouse_bt==1 && value[1]==1){value[1]=0;}
            }
            if (value[1]==2 && mouse_bt==0){value[1]=1;}
            for (int a=0;a<icount-2;a++){
                sz=(listrpos[a+1]-listrpos[a])-1;memcpy(cbtxt,&text[listrpos[a]],sz);cbtxt[sz]=0;Print(rpos+ivec2(6,26+a*16),color(0),cbtxt);
            }
        }
        siz=sizu;
    }
    if (type==Control::LIST){
        inwin->DrawBorderFilled(rpos,siz,2,color(255));
        if (MouseTest(rpos,siz) && mouse_bt==1){
            int sl=(mouse_y-(rpos.y+5))/16;
            value[0]=sl+value[2]/100;
        }
        int ctt=0,olchar=0;
        int offset=value[2]/100;
        char litex[128];
        int vacombo=value[0]-(value[2]/100);
        if(vacombo>-1){Box(rpos+ivec2(4,4+vacombo*16),rpos+ivec2(siz.x-4,21+vacombo*16),color(128));}
        for (int a=0;a<text.size();a++){
            if (text[a]=='|'){
                int sz=a-olchar;
                if (offset<1){
               		text.copy(litex,sz,olchar);litex[sz]=0;
                    Print(rpos+ivec2(5,(ctt*16)+5),color(0),litex);
                    ctt++;
                }else{offset--;}
                olchar=a+1;
            }
        }
		inwin->RunScroll(rpos+ivec2(siz.x,0),ivec2(24,siz.y),&value[2]);
    }
    if (type==Control::TEXT){
		if (MouseTest(rpos,siz) && mouse_edup==1 && !textfocus){
			ConnectKB(this);
			curpos=text.size();
		}
		inwin->DrawBorderFilled(rpos,siz,2,color(255));
		if (textfocus){value[2]=enterpressed;}else{value[2]=0;}
		if (value[1]==1){
			//Multiline
		}else{
			Print(rpos+ivec2(6,6),color(0),text.c_str());
			if (textfocus){Print(rpos+ivec2(6+curpos*8,8),color(0),"|");}

		}
    }
    if (type==Control::TABS){
        int ctt=0,olchar=0;
        int offset=value[2]/100;
        char litex[64];
        int nupars=0;
        for (int a=0;a<text.size();a++){if (text[a]=='|'){nupars++;}}
        int bsize=140;
        if (nupars>7){bsize=150*(float(7.0/nupars));}
        if (nupars>0){
		value[5]=0;
			for (int a=0;a<text.size();a++){
				if (text[a]=='|'){
					int sz=a-olchar;
					if (offset<1){
				        int sele=0;
				        if (value[1]==ctt){sele=1;}
				        inwin->DrawBorderFilled(rpos+ivec2(2+(ctt*bsize),1),ivec2(bsize-(siz.y-2),siz.y-2),1+sele,color(192));
				        inwin->DrawBorderFilled(rpos+ivec2(2+(ctt*bsize)+(bsize-(siz.y-2)),1),ivec2(siz.y-2,siz.y-2),1,color(192));

						text.copy(litex,sz,olchar);litex[sz]=0;
						Print(rpos+ivec2(5+(ctt*bsize),5),color(0),litex);
						Print(rpos+ivec2(10+(ctt*bsize)+(bsize-(siz.y-2)),5),color(0),"X");
						if (MouseTest(rpos+ivec2(2+(ctt*bsize),1),ivec2(bsize,siz.y-2)) && mouse_edup==1){value[1]=ctt;}
						if (MouseTest(rpos+ivec2(2+(ctt*bsize)+(bsize-(siz.y-2)),1),ivec2(siz.y-2,siz.y-2)) && mouse_edup==1){value[5]=1;}

						ctt++;
					}else{offset--;}
					olchar=a+1;
				}
			}
        }
	}



}
void Control::SetText(const char * textin){
	if (strcmp(text.c_str(),textin)!=0){
		text=textin;
		if (type==Control::TEXT && ctcon==this){curpos=text.size();}
	}
}
void Control::GetText(char * textout){
	text.copy(textout,text.size(),0);
	textout[text.size()]=0;
}
int Control::getValue(int id){return value[id];}
void Control::setValue(int id,int val){value[id]=val;}

/*
void strcpy_s(char *out, int nb,const char*in){ //Awful kludge to make it compile
    if (strlen(in)<nb){strcpy(out,in);}
}
void strcat_s(char *out, int nb,const char*in){
	if (strlen(out)+strlen(in)<nb){strcat(out,in);}
}
void memcpy_s(void * dest,int maxsiz,void * src,int siz){
	if (siz>0 && siz<maxsiz){memcpy(dest,src,siz);}
}

*/