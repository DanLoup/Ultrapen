#include "MainWindow.h"
#include "data.h"

unsigned short abuff[20000];
int abuffpos=735*3;
int bucor=0;

struct PSG{
	int cha_freq[3];
	int cha_vol[3];
	int cha_en[8];
	int noifreq;
	int env_freq,env_shape;
	unsigned short buffer[8192];
	void run(int siz);
	void runengine();
    void boot();
	void playnote(int inst,int note,int vol,int cha);
	void stopnote(int cha);
    float chct[3],noisect=0,envct=0;
    signed short chfl[3];
    float voltb[16];
    unsigned int noiseseed=1423435346;
	int freqtopsg(float freq){return (111860.781/freq);};
	note nt[3];
	int nt_s[3];
	int nt_e[3];
};

struct SCC{
	int regs[16];
	int samples[5][32];
	unsigned short buffer[8192];
	void run(int siz);
	void runengine();
};

struct mus_channel{
	note nt;
	int chip;
	float adsr_state,adsrval;
	int arpct=0;
	int effvar[4];
};

struct mus_engine{
	std::vector <instrument> g_insts;
	void runengine(int channel);
	void playnote(int inst,int note,int vol,int cha);
	void stopnote(int cha);
	void boot();
	mus_channel ch[10];
	int pat[4][32];
	int adr[16];
};

SCC scaud;
PSG psaud;
std::vector<mus_engine> eng;
float cosco=0;
float coscac=0;

SDL_AudioSpec afor;

int set=0,totgen=0;
int ts;
int disct=0;

float noteToFreq(int note) {
	double ratio=pow(2.0,1.0/12.0);
    return 16.35 * pow(ratio,note);
}

void AudioCallback(void*  userdata, Uint8* stream, int len);
void bootaudio(){
    SDL_AudioSpec ad;
 	SDL_memset(&ad, 0, sizeof(ad));
	ad.channels=1;
	ad.format=AUDIO_S16;
	ad.freq=44100;
	ad.samples=2048;
	ad.userdata=NULL;
	ad.callback=AudioCallback;
	SDL_OpenAudio(&ad,&afor);
	SDL_PauseAudio(0);
}
void AudioCallback(void*  userdata, Uint8* stream, int len){
	memcpy(stream,abuff,len);
	if (abuffpos>2047){
		abuffpos-=2048;
		for (int a=0;a<abuffpos+1;a++){
			abuff[a]=abuff[a+2048];
		}
	}
	disct++;
	if (abuffpos<735){bucor=735;}
	if (abuffpos>1400){bucor=-735;}
	if (disct>11){
		//printf("bupos:%i\n",abuffpos);
		disct=0;
	}
}
int freqco=0;
int ctm=0,ctn=0;
int nt[]={(4*12)+9,(4*12)+9,(4*12)+9,(4*12)+5,(4*12)+9,(5*12)+0,(4*12)+0};
void RunAudio(){
	//psaud.runengine();
	psaud.run(735+bucor);
    for (int a = 0; a < 735+bucor;a++){abuff[abuffpos++]=psaud.buffer[a];}
	bucor = 0;
}

std::string delist(std::vector<int> el){
	std::string out;
	for (int a = 0;a < el.size();a++){
		char text[100];snprintf(text,100,"%i",el[a]);
		out+=text;
		if (a<el.size()-1){out+=",";}
	}
	return out;
}
std::vector<int> relist(std::string ti){
	std::vector<int> out;
	int po=0;
	for (int a=0;a < ti.size();a++){
		if (ti[a]==','){out.push_back(atoi(&ti.c_str()[po]));po=a+1;}
	}
	out.push_back(atoi(&ti.c_str()[po]));
	return out;
}

void PSG::boot(){
   	float sq2=sqrt(2.0);
	for(int a=0;a<16;a++){voltb[a]=1/pow(sq2,15-(a));}
	voltb[0]=0;int muu=0;noiseseed=2489081249;
    for (int a = 0; a < 3;a++){chct[a] = 0;chfl[a] = 0;nt_s[3] = 0;}
	for (int a = 0;a < 8;a++){cha_en[a]=1;}
}

void PSG::run(int size){
	int ss=1;float fss=ss;
	float chco[3],fia[3],noiseco=0;
	float envco=0,envol=0;
    int ot=0,nb=0,fivol[3];
    for (int a = 0; a < 3; a++){
        if (cha_freq[a]>0){chco[a]=((111860.781/float(cha_freq[a] & 4095))/22050.0)/fss;}else{chco[a]=0;}
    }
    if (env_freq>0){envco=(111860.781/float(env_freq)/22050.0);}
	int nf=noifreq;
	if (nf==0){nf=1;}
	noiseco=(111860.781/float((nf&31)));noiseco/=(22050.0*8);
	float ain;
    int noiseout=0;
	for (int a=0;a<size;a++){
		for (int b=0;b<3;b++){fia[b]=0;}
	    envct+=envco;
		if (envct>64){envct=fmod(envct,1);}
	    for (int c=0;c<ss;c++){ //Super sampled 4x audio, nice, also this explains all the division by 4's
		    noisect+=noiseco;
		    if (noisect>1){
			    noisect=fmod(noisect,1);noiseout^=(noiseseed&1);nb=(noiseseed&1)^((noiseseed&4)/4);noiseseed=(noiseseed>>1)+(nb<<16);
		    }
		    for (int d = 0; d < 3;d++){
                chct[d]+=chco[d]; if (chct[d]>1){chct[d]=fmod(chct[d],1);chfl[d]=(chfl[d]+1) & 1;}
		        ot=(chfl[d]|(cha_en[d])) & (noiseout|cha_en[d+3]);
                fia[d]+=ot*(1.0/(fss*2.0));
            }
	    }
        for (int d = 0; d < 3;d++){fivol[d]=cha_vol[d];}

	    if ((env_shape>-1 && env_shape<4) || env_shape==9){envol=15-envct;if (envol<0){envol=0;}}
	    if ((env_shape>3 && env_shape<8) || env_shape==0xf){envol=envct;if (envol>15){envol=0;}}
	    if (env_shape==8){envol=15-int(envct)%16;}
	    if (env_shape==0xa){envol=int(envct)%32;if (envol>15){envol=31-envol;}envol=15-envol;}
	    if (env_shape==0xb){envol=15-envct;if (envol<0){envol=15;}}
	    if (env_shape==0xc){envol=int(envct)%16;}
	    if (env_shape==0xd){envol=envct;if (envol>15){envol=15;}}
	    if (env_shape==0xe){envol=int(envct)%32;if (envol>15){envol=32-envol;}}
        ain=0;
        for (int d = 0; d < 3;d++){
            if (cha_vol[d]==16){
				fivol[d]=envol;};
            ain+=(fia[d]*(voltb[fivol[d] & 15]*24000.0)*0.33);
		}
	    buffer[a]=ain*2;
	}
}
float piwarp=0,roto=0;

void mus_engine::boot(){
	for (int a = 0; a < 32;a++){
		pat[0][a]=sin(a*(6.282/32))*32; //Sine
		pat[1][a]=(7-(a & 7))*4; //Ramp
		pat[2][a]=(a & 8)*4;//Arpeggio stuff?
	}
	for (int a = 0; a < 16; a++){
		adr[a]=64/((a*2)+1);
	}
}
void mus_engine::playnote(int inst,int note,int vol,int cha){
	ch[cha].adsr_state=1;
	ch[cha].adsrval=0;
	ch[cha].effvar[0]=0;
	ch[cha].effvar[1]=0;
	ch[cha].arpct=0;
	psaud.nt[cha].instrument=inst;
	psaud.nt[cha].pitch=note;
	psaud.nt[cha].volume=vol;
	psaud.cha_en[cha]=0;
	piwarp=0;
}
void mus_engine::stopnote(int cha){
	ch[cha].adsr_state=3;
}

void mus_engine::runengine(int cc){
	if (ch[cc].adsr_state >0){
		int i = psaud.nt[cc].instrument;
		int pid=0,aud=0;
		if (g_insts[i].chip==0){
			if (ch[cc].adsr_state==1){
				int ip=adr[g_insts[i].adsr_siz[0]];
				ch[cc].adsrval+=ip*2;if (ch[cc].adsrval>62){ch[cc].adsr_state=2;ch[cc].adsrval=63;goto ed;}
			}
			if (ch[cc].adsr_state==2){
				int ip = adr[g_insts[i].adsr_siz[1]]/2;
				int sus = (g_insts[i].adsr_sus*4);
				ch[cc].adsrval-=ip;if (ch[cc].adsrval<sus){ch[cc].adsrval=sus;goto ed;}
			}
			if (ch[cc].adsr_state==3){
				int ip=adr[g_insts[i].adsr_siz[2]];
				ch[cc].adsrval-=ip;if (ch[cc].adsrval<1){ch[cc].adsr_state=0;ch[cc].adsrval=0;psaud.cha_en[cc]=1;goto ed;}
			}
			ed:
			if (g_insts[i].effect==1){
				if (ch[cc].adsr_state==1){
					pid=(60-ch[cc].effvar[0]);
					ch[cc].effvar[0]+=1;
					if (pid<0){pid=0;}
				}
				aud=(((pat[0][(ch[cc].effvar[0]/4)  & 31]+32))*(ch[cc].effvar[1]/32))/64;
				aud *=-1;
				ch[cc].effvar[0]+=g_insts[i].effpar[0]+1;
				ch[cc].effvar[1]+=g_insts[i].effpar[1]+1;
				if (ch[cc].effvar[1]>255){ch[cc].effvar[1]=255;}
			}

			if (g_insts[i].effect==2){
				if (ch[cc].adsr_state>1){
					pid = 4 * (ch[cc].effvar[0]);
					ch[cc].effvar[0]+=g_insts[i].effpar[0];
				}			
			}

			int arp=0;
			arp = g_insts[i].arpt[ch[cc].arpct];
			ch[cc].arpct++;
			if (g_insts[i].arpt[ch[cc].arpct]==255){ch[cc].arpct=0;}
			if (ch[cc].arpct>g_insts[i].arpt.size()-1){ch[cc].arpct=g_insts[i].arpt.size()-1;}

			if (g_insts[i].instype==0){
				psaud.cha_vol[cc]=(ch[cc].adsrval/4)+aud;
				if (psaud.cha_vol[cc]<0){psaud.cha_vol[cc]=0;}
				if (psaud.cha_vol[cc]>15){psaud.cha_vol[cc]=15;}

				psaud.cha_freq[cc]=psaud.freqtopsg(noteToFreq(psaud.nt[cc].pitch+arp))+pid;
				psaud.cha_en[cc+3]=1;
				psaud.cha_en[cc]=0;
			}
			if (g_insts[i].instype==1){
				psaud.cha_vol[cc]=(ch[cc].adsrval/4)+aud;
				if (psaud.cha_vol[cc]<0){psaud.cha_vol[cc]=0;}
				if (psaud.cha_vol[cc]>15){psaud.cha_vol[cc]=15;}
				psaud.noifreq=((psaud.nt[cc].pitch+arp)-30)+pid/64;
				if (psaud.noifreq<0){psaud.noifreq=0;}
				if (psaud.noifreq>31){psaud.noifreq=31;}
				psaud.cha_en[cc+3]=0;
				psaud.cha_en[cc]=1;
			}
			if (g_insts[i].instype==2){
				psaud.cha_en[cc+3]=1;
				psaud.cha_en[cc]=0;
				psaud.cha_freq[cc]=1;
				int vot=0;
				if (((ch[cc].adsrval/4)+aud)>0){vot=16;}
				psaud.cha_vol[cc]=vot;
				psaud.env_shape=0xa;
				psaud.env_freq=(psaud.freqtopsg(noteToFreq((psaud.nt[cc].pitch-10)+arp))+pid)/16;
			}

		}	
	}
}

void SCC::run(int siz){

}




int MusicSet::Encode(unsigned char * data){
    return 0;
}

void MusicSet::Decode(unsigned char * data){
}


char pars[][16]={"Type","Attack","Decay","Sustain","Release","effect","eff1","eff2","eff3","eff4"};
char kb[]="zxcvbnmasdfghjklqwertyuiop";
int nbs[16];
int lanb=0;
void InstrumentSet::Boot(){
	psaud.boot();
	aswin=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	aswin->AddControl("prv",Control::BUTTON,ivec2(5,0),ivec2(25,25));aswin->SetText("prv","<");
	aswin->AddControl("nxt",Control::BUTTON,ivec2(55,0),ivec2(25,25));aswin->SetText("nxt",">");
	aswin->AddControl("insnb",Control::LABEL,ivec2(30,7),ivec2(50,25));aswin->SetText("insnb","0");
	aswin->AddControl("adsrb",Control::EPICTURE,ivec2(5,100),ivec2(300,200));
	aswin->AddControl("arpb",Control::EPICTURE,ivec2(335,100),ivec2(300,200));
	aswin->AddControl("adsrt",Control::TEXT,ivec2(5,305),ivec2(300,25));aswin->SetText("adsrt","4,4,4,4");
	aswin->AddControl("arpt",Control::TEXT,ivec2(335,305),ivec2(300,25));aswin->SetText("arpt","0");

	aswin->AddControl("name",Control::TEXT,ivec2(5,50),ivec2(125,25));aswin->SetText("name","piano");
	aswin->AddControl("chip",Control::COMBO,ivec2(140,50),ivec2(125,25));aswin->SetText("chip","PSG|SCC|OPLL|");
	aswin->AddControl("itype",Control::COMBO,ivec2(280,50),ivec2(125,25));aswin->SetText("itype","SQUARE|NOISE|TRIANGLE|");

	aswin->AddControl("lb1",Control::LABEL,ivec2(5,35),ivec2(125,25));aswin->SetText("lb1","Name:");
	aswin->AddControl("lb2",Control::LABEL,ivec2(140,35),ivec2(125,25));aswin->SetText("lb2","Chip:");
	aswin->AddControl("lb3",Control::LABEL,ivec2(280,35),ivec2(125,25));aswin->SetText("lb3","Type:");

	aswin->AddControl("lb4",Control::LABEL,ivec2(5,340),ivec2(125,25));aswin->SetText("lb4","Effect:");
	aswin->AddControl("effect",Control::TEXT,ivec2(5,360),ivec2(125,25));aswin->SetText("effect","0,0,0");
	curins=0;
	if (ms>-1){
		Update();
	}else{
		eng.push_back(mus_engine());
		ms = eng.size()-1;
		eng[ms].boot();
		for (int a = 0; a < 16;a++){
			eng[ms].g_insts.push_back(instrument());
			eng[ms].g_insts.back().chip=0;
		}
	}
}
int cha[3],chact=0;
void InstrumentSet::Run(int status){
	aswin->RunWindow();
	for (int a = 0; a < 3; a++){eng[ms].runengine(a);}
	RunAudio();

	for (int a = 0;a < strlen(kb);a++){
		bool nope=false;
		for (int b = 0; b < 3; b++){if (cha[b]==kb[a]){nope=true;}}
		if (keys[kb[a]]>0 && !nope){
			for (int b = 0; b < 3; b++){
				if (cha[b]==0){
					cha[b]=kb[a];
					eng[ms].playnote(curins,a+30,15,b);
					break;
				}
			}
		}
	}
	for (int a = 0; a < 3;a++){
		if (cha[a]>0 && keys[cha[a]]==0){
			eng[ms].stopnote(a);
			cha[a]=0;
		}
	}

	aswin->SetNumberAsText("trash",0);
	std::vector <int> vl;
	char t[100];
	aswin->GetText("adsrt",t);vl = relist(t);
	if (vl.size()!=4){vl.resize(4,0);}
	if (t[strlen(t)-1]>64){aswin->SetText("adsrt",delist(vl).c_str());}
	aswin->GetText("arpt",t);
	eng[ms].g_insts[curins].arpt = relist(t);	
	aswin->GetText("name",t);
	eng[ms].g_insts[curins].name = t;
	eng[ms].g_insts[curins].instype = aswin->GetValue("itype",0);
	eng[ms].g_insts[curins].chip = aswin->GetValue("chip",0);
	eng[ms].g_insts[curins].adsr_siz[0]=vl[0];
	eng[ms].g_insts[curins].adsr_siz[1]=vl[1];
	eng[ms].g_insts[curins].adsr_sus=vl[2];
	eng[ms].g_insts[curins].adsr_siz[2]=vl[3];
	vl.clear();
	aswin->GetText("effect",t);vl = relist(t);
	if (vl.size()>0){eng[ms].g_insts[curins].effect=vl[0];}
	if (vl.size()>1){eng[ms].g_insts[curins].effpar[0]=vl[1];}
	if (vl.size()>2){eng[ms].g_insts[curins].effpar[1]=vl[2];}

	int cha=0;
	if (aswin->GetValue("prv",0)>0){if (curins>0){curins--;cha=1;}}
	if (aswin->GetValue("nxt",0)>0){if (curins<15){curins++;cha=1;}}
	if (cha>0){
		aswin->SetNumberAsText("insnb",curins);
		aswin->SetText("name",eng[ms].g_insts[curins].name.c_str());
		std::vector<int>vll;vll.resize(4,0);
		vll[0]=eng[ms].g_insts[curins].adsr_siz[0];vll[1]=eng[ms].g_insts[curins].adsr_siz[1];
		vll[2]=eng[ms].g_insts[curins].adsr_sus;vll[3]=eng[ms].g_insts[curins].adsr_siz[2];
		aswin->SetText("adsrt",delist(vll).c_str());
		aswin->SetText("arpt",delist(eng[ms].g_insts[curins].arpt).c_str());
		vll[0]=eng[ms].g_insts[curins].effect;vll[1]=eng[ms].g_insts[curins].effpar[0];vll[2]=eng[ms].g_insts[curins].effpar[1];
		aswin->SetText("effect",delist(vll).c_str());
		aswin->SetValue("itype",0,eng[ms].g_insts[curins].instype);
		aswin->SetValue("chip",0,eng[ms].g_insts[curins].chip);
	}

}

int InstrumentSet::Encode(unsigned char * data){
   	DH dat;dat.data=data;dat.pos=0;

	dat.wshort(eng[ms].g_insts.size());
	for (int a=0;a<eng[ms].g_insts.size();a++){
		dat.wbyte(eng[ms].g_insts[a].name.size());
		for (int b = 0; b < eng[ms].g_insts[a].name.size(); b++){
			dat.wbyte(eng[ms].g_insts[a].name[b]);
		}
		dat.wbyte(eng[ms].g_insts[a].chip);
		dat.wbyte(eng[ms].g_insts[a].instype);
		dat.wbyte(eng[ms].g_insts[a].effect);
		for (int b = 0;b < 2; b++){dat.wbyte(eng[ms].g_insts[a].effpar[b]);}
		for (int b = 0;b < 3; b++){dat.wbyte(eng[ms].g_insts[a].adsr_siz[b]);}
		dat.wbyte(eng[ms].g_insts[a].adsr_sus);
		dat.wbyte(eng[ms].g_insts[a].arpt.size());
		for (int b = 0; b < eng[ms].g_insts[a].arpt.size(); b++){
			dat.wbyte(eng[ms].g_insts[a].arpt[b]);
		}
		for (int b = 0;b < 32; b++){
			dat.wbyte(eng[ms].g_insts[a].data[b]);
		}
	}
	return dat.pos;

}

void InstrumentSet::Decode(unsigned char * data){
	DH dat;dat.data=data;dat.pos=0;
	eng.push_back(mus_engine());
	ms = eng.size() - 1;
	eng[ms].boot();
	eng[ms].g_insts.clear();
	eng[ms].g_insts.resize(dat.rshort());
	for (int a=0;a<eng[ms].g_insts.size();a++){
		int si=dat.rbyte();
		for (int b = 0; b < si;b++){
			eng[ms].g_insts[a].name+=dat.rbyte();
		}
		eng[ms].g_insts[a].chip = dat.rbyte();
		eng[ms].g_insts[a].instype = dat.rbyte();
		eng[ms].g_insts[a].effect = dat.rbyte();
		for (int b = 0; b < 2; b++){eng[ms].g_insts[a].effpar[b]=(char)dat.rbyte();}
		for (int b = 0;b < 3; b++){eng[ms].g_insts[a].adsr_siz[b]=(char)dat.rbyte();}
		eng[ms].g_insts[a].adsr_sus=dat.rbyte();
		si=dat.rbyte();
		eng[ms].g_insts[a].arpt.clear();
		for (int b = 0; b < si; b++){eng[ms].g_insts[a].arpt.push_back((char)dat.rbyte());}
		for (int b = 0;b < 32; b++){eng[ms].g_insts[a].data[b] = dat.rbyte();}

	}	
}
void InstrumentSet::Update(){
	aswin->SetNumberAsText("insnb",curins);
	aswin->SetText("name",eng[ms].g_insts[curins].name.c_str());
	std::vector<int>vll;vll.resize(4,0);
	vll[0]=eng[ms].g_insts[curins].adsr_siz[0];vll[1]=eng[ms].g_insts[curins].adsr_siz[1];
	vll[2]=eng[ms].g_insts[curins].adsr_sus;vll[3]=eng[ms].g_insts[curins].adsr_siz[2];
	aswin->SetText("adsrt",delist(vll).c_str());
	aswin->SetText("arpt",delist(eng[ms].g_insts[curins].arpt).c_str());
	vll[0]=eng[ms].g_insts[curins].effect;vll[1]=eng[ms].g_insts[curins].effpar[0];vll[2]=eng[ms].g_insts[curins].effpar[1];
	aswin->SetText("effect",delist(vll).c_str());
	aswin->SetValue("itype",0,eng[ms].g_insts[curins].instype);
	aswin->SetValue("chip",0,eng[ms].g_insts[curins].chip);
}

void MusicSet::Boot(){
	psaud.boot();
	eng.push_back(mus_engine());
	ms=eng.size()-1;
	eng[ms].boot();
	LoadFile("./base.uis", "base.uis", true);
	InstrumentSet* mus = (InstrumentSet*) fileoutld;
	mus->Boot();
	eng[ms].g_insts=eng[mus->ms].g_insts;
	eng.pop_back();
	delete mus;
	aswin=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	aswin->AddControl("Piano",Control::EPICTURE,ivec2(5,80),ivec2(980,364));
	aswin->AddControl("Notebar",Control::SCROLL,ivec2(980,80),ivec2(24,364));
	aswin->AddControl("Musicbar",Control::SCROLL,ivec2(5,444),ivec2(975,24));
	aswin->AddControl("Instruset",Control::LIST,ivec2(5,490),ivec2(150,160));aswin->SetText("Instruset","Base|");
	aswin->AddControl("AddLib",Control::BUTTON,ivec2(5,650),ivec2(80,24));aswin->SetText("AddLib","Add");
	aswin->AddControl("RemLib",Control::BUTTON,ivec2(90,650),ivec2(80,24));aswin->SetText("RemLib","Remove");
	aswin->AddControl("BPM",Control::TEXT,ivec2(40,50),ivec2(100,24));aswin->SetText("BPM","150");
	aswin->AddControl("BPMlabel",Control::LABEL,ivec2(5,55),ivec2(100,24));aswin->SetText("BPMlabel","BPM:");
	aswin->AddControl("Play",Control::BUTTON,ivec2(200,50),ivec2(80,24));aswin->SetText("Play","Play");
	aswin->AddControl("Stop",Control::BUTTON,ivec2(285,50),ivec2(80,24));aswin->SetText("Stop","Stop");

}

int bongct=0;
int noup=40;
void MusicSet::Run(int status){
	aswin->RunWindow();
	Box(ivec2(8,140),ivec2(8+970,140+360),color(0));
	int offct=0;
	for (int a = 0; a < 40;a++){
		color cc = color(24,24,24);
		if ((a & 3)==3){cc=color(48,48,48);}
		if ((a & 15)==15){cc=color(128,128,128);}

		Line(ivec2(96+a*24,140),ivec2(96+a*24,140+360),cc);
	
	}
	for (int a = 0; a < 15;a++){
		Line(ivec2(8,140+a*24),ivec2(978,140+a*24),color(32,32,32));
		Box(ivec2(8,142+a*24),ivec2(8,142+a*24)+ivec2(80,20),color(255));
		Box(ivec2(12,146+a*24),ivec2(12,146+a*24)+ivec2(80,20),color(64));
		Box(ivec2(10,144+a*24),ivec2(10,144+a*24)+ivec2(80,20),color(220));
	}
}
