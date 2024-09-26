#include "MainWindow.h"
#include "data.h"
#include "popup.h"

unsigned short abuff[20000];
int abuffpos=0;
int bucor=0;

struct PSG{
	int cha_freq[3];
	int cha_vol[3];
	int cha_en[8];
	int noifreq;
	int env_freq,env_shape;
	unsigned short buffer[8192*4];
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

	int rampct,cur_rampct,rampaddr;
	int curvol,curpitch_bend,curpitch,rawpitch;
	int chatype,curvvol,curvpitch;
	int pressed=0;
	int cur_ramptype=0;
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

int racg=0,raco=0;
int offit=0;

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
	psaud.boot();
}



void AudioCallback(void*  userdata, Uint8* stream, int len){
	memcpy(stream,abuff,len);
	if (abuffpos>2047){
		abuffpos-=2048;
		for (int a=0;a<abuffpos+1;a++){
			abuff[a]=abuff[a+2048];
		}
	}
	raco+=2048;
	if (abs(raco-racg)>8192){raco=0;racg=0;}
	if (raco>2000000){racg=racg-raco;raco=0;}

}
int freqco=0;
int ctm=0,ctn=0;
int nt[]={(4*12)+9,(4*12)+9,(4*12)+9,(4*12)+5,(4*12)+9,(5*12)+0,(4*12)+0};
void RunAudio(){
	if ((racg-raco)<500){bucor=735;}
	if ((racg-raco)>2000){return;}
	racg+=bucor+735;
	psaud.run(735+bucor);
    for (int a = 0; a < 735+bucor;a++){abuff[abuffpos++]=psaud.buffer[a];}
	bucor = 0;
	if (abuffpos>10000){abuffpos=10000;}
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
	noiseco=(111860.781/float((nf&31)));noiseco/=(22050.0*2);
	float ain;
    int noiseout=0;
	for (int a=0;a<size;a++){
		for (int b=0;b<3;b++){fia[b]=0;}
	    envct+=envco;
		if (envct>16383.0){envct=fmod(envct,16384.0)+15.0;}
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
	    if (env_shape==0xa){envol=int(envct)%32;if (envol>15){envol=31-envol;}}
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
	int tbl[128];
	for (int a = 1; a < 128;a++){
		tbl[a]=psaud.freqtopsg(noteToFreq(a));
	}
	FILE * f =fopen("tbl.txt","w");
	int ct=0;
	for (int a = 0; a < 8;a++){
		fprintf(f,".dw ");
		for (int b =0; b < 16;b++){
			fprintf(f,"$%04x,",tbl[ct++]);
		}
		fprintf(f,"\n");
	}
	fclose(f);	
}
void mus_engine::playnote(int inst,int note,int vol,int cha){
	if (g_insts[inst].ramps.size()>0){
		ch[cha].rampaddr=0;
		ch[cha].cur_rampct=0;
		ch[cha].rampct=g_insts[inst].ramps.size()+1;
		ch[cha].pressed=1;
		psaud.nt[cha].instrument=inst;
		psaud.nt[cha].pitch=note;
		psaud.nt[cha].volume=vol;
		psaud.cha_en[cha]=0;
		piwarp=0;
	}
}
void mus_engine::stopnote(int cha){
	ch[cha].pressed = 0;
	//ch[cha].rampct=0;
	//psaud.cha_en[cha]=1;

}

void mus_engine::runengine(int cc){
	int i = psaud.nt[cc].instrument;
	if (ch[cc].rampct>0){
		if (ch[cc].cur_rampct<1){
			ch[cc].cur_rampct = g_insts[i].ramps[ch[cc].rampaddr].rsize;
			ch[cc].curpitch = psaud.freqtopsg(noteToFreq(psaud.nt[cc].pitch));
			ch[cc].rawpitch=psaud.nt[cc].pitch+1;
			ch[cc].curpitch_bend = g_insts[i].ramps[ch[cc].rampaddr].ipitch;
			ch[cc].curvol=g_insts[i].ramps[ch[cc].rampaddr].ivol;
			ch[cc].cur_ramptype = g_insts[i].ramps[ch[cc].rampaddr].type;
			float iflip=1.0;
			if (ch[cc].cur_rampct>0) iflip = 1.0/float(ch[cc].cur_rampct);
			ch[cc].curvvol=float(g_insts[i].ramps[ch[cc].rampaddr].vvol - g_insts[i].ramps[ch[cc].rampaddr].ivol)*iflip;
			ch[cc].curvpitch=float(g_insts[i].ramps[ch[cc].rampaddr].vpitch - g_insts[i].ramps[ch[cc].rampaddr].ipitch)*iflip;
			if (g_insts[i].loopini==ch[cc].rampaddr && ch[cc].pressed > 0){
				ch[cc].rampaddr=g_insts[i].looppt;
				ch[cc].rampct=(g_insts[i].ramps.size()+1)-(g_insts[i].looppt);
			}else{
				ch[cc].rampaddr++;ch[cc].rampct--;
			}
			if (ch[cc].rampct==0){
				psaud.cha_en[cc]=1;
				psaud.cha_vol[cc]=0;
				return;
			}
		}
		ch[cc].curpitch_bend&=255;
		ch[cc].curvol&=255;
		if (ch[cc].cur_ramptype==0 || ch[cc].cur_ramptype==3){
			psaud.cha_vol[cc]=(ch[cc].curvol/16) & 15;
			psaud.cha_freq[cc]=ch[cc].curpitch+ch[cc].curpitch_bend;
			if (ch[cc].cur_ramptype==0){psaud.cha_en[cc+3]=1;}
			psaud.cha_en[cc]=0;
		}
		if (ch[cc].cur_ramptype==1 || ch[cc].cur_ramptype==3){
			psaud.noifreq=((ch[cc].rawpitch+ch[cc].curpitch_bend)) & 31;
			psaud.cha_vol[cc]=(ch[cc].curvol/16) & 15;
			psaud.cha_en[cc+3]=0;
			if (ch[cc].cur_ramptype==1){psaud.cha_en[cc]=1;}
		}
		if (ch[cc].cur_ramptype==2){
			psaud.cha_vol[cc]=16;
			psaud.cha_en[cc+3]=1;psaud.cha_en[cc]=0;
			psaud.cha_freq[cc]=10;psaud.env_shape=0xa;
			psaud.env_freq=(ch[cc].curpitch+ch[cc].curpitch_bend)/16;
		}
		ch[cc].cur_rampct-=1;
		ch[cc].curpitch_bend+=cc[ch].curvpitch;
		ch[cc].curvol+=cc[ch].curvvol;

	}
}

void SCC::run(int siz){

}




int MusicSet::Encode(unsigned char * data){
   	DH dat;dat.data=data;dat.pos=0;
	dat.wbyte(songs.size());
	for (int a = 0; a < songs.size();a++){
		dat.wshort(songs[a].bpm);
		dat.wshort(songs[a].tracks.size());
		for (int b = 0; b < songs[a].tracks.size();b++){
			dat.wbyte(songs[a].tracks[b].name.size());
			for (int c = 0; c < songs[a].tracks[b].name.size();c++){
				dat.wbyte(songs[a].tracks[b].name[c]);
			}
			dat.wshort(songs[a].tracks[b].size);
			dat.wshort(songs[a].tracks[b].notes.size());
			for (int c = 0; c < songs[a].tracks[b].notes.size();c++){
				dat.wbyte(songs[a].tracks[b].notes[c].oct);
				dat.wbyte(songs[a].tracks[b].notes[c].note);
				dat.wbyte(songs[a].tracks[b].notes[c].inst);
				dat.wbyte(songs[a].tracks[b].notes[c].duration);
			}
		}		
		dat.wshort(songs[a].time_ent.size());
		for (int b = 0;b< songs[a].time_ent.size();b++){
			dat.wshort(songs[a].time_ent[b].start);
			dat.wshort(songs[a].time_ent[b].size);
			dat.wbyte(songs[a].time_ent[b].lane);
			dat.wbyte(songs[a].time_ent[b].track);
		}
		/*
		dat.wshort(songs[a].notes.size());
		for (int b = 0; b < songs[a].notes.size();b++){
			dat.wbyte(songs[a].notes[b].cmd);
			dat.wbyte(songs[a].notes[b].instrument);
			dat.wbyte(songs[a].notes[b].pitch);
			dat.wshort(songs[a].notes[b].pos);
			dat.wbyte(songs[a].notes[b].size);
			dat.wbyte(songs[a].notes[b].volume);
		}
		*/
	}
    return dat.pos;
}

void MusicSet::Decode(unsigned char * data){
   	DH dat;dat.data=data;dat.pos=0;
	songs.clear();
	songs.resize(dat.rbyte());
	for (int a = 0; a < songs.size();a++){
		songs[a].bpm = dat.rshort();
		songs[a].tracks.clear();songs[a].tracks.resize(dat.rshort());
		for (int b = 0; b < songs[a].tracks.size();b++){
			char tx[160];
			int size = dat.rbyte();
			for (int c = 0; c < size;c++){tx[c] = dat.rbyte();}tx[size]=0;
			songs[a].tracks[b].name = tx;
			songs[a].tracks[b].size = dat.rshort();
			songs[a].tracks[b].notes.clear();songs[a].tracks[b].notes.resize(dat.rshort());
			
			for (int c = 0; c < songs[a].tracks[b].notes.size();c++){
				songs[a].tracks[b].notes[c].oct = dat.rbyte();
				songs[a].tracks[b].notes[c].note = dat.rbyte();
				songs[a].tracks[b].notes[c].inst = dat.rbyte();
				songs[a].tracks[b].notes[c].duration = dat.rbyte();
			}
		}		
		songs[a].time_ent.clear();songs[a].time_ent.resize(dat.rshort());
	
		for (int b = 0;b< songs[a].time_ent.size();b++){
			songs[a].time_ent[b].start = dat.rshort();
			songs[a].time_ent[b].size = dat.rshort();
			songs[a].time_ent[b].lane = dat.rbyte();
			songs[a].time_ent[b].track = dat.rbyte();
		}
	
	}
}


char pars[][16]={"Type","Attack","Decay","Sustain","Release","effect","eff1","eff2","eff3","eff4"};
char kb[]="zxcvbnmasdfghjklqwertyuiop";
int nbs[16];
int lanb=0;
void InstrumentSet::Boot(bool newf){
	aswin=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	aswin->AddControl("prv",Control::BUTTON,ivec2(5,0),ivec2(25,25));aswin->SetText("prv","<");
	aswin->AddControl("nxt",Control::BUTTON,ivec2(55,0),ivec2(25,25));aswin->SetText("nxt",">");
	aswin->AddControl("insnb",Control::LABEL,ivec2(30,7),ivec2(50,25));aswin->SetText("insnb","0");

	aswin->AddControl("l1",Control::LABEL,ivec2(5,30),ivec2(125,25));aswin->SetText("l1","Name:             SCC waveID:     loop begin:  loop point:");
	aswin->AddControl("name",Control::TEXT,ivec2(5,50),ivec2(125,25));aswin->SetText("name","piano");
	aswin->AddControl("chipe",Control::TEXT,ivec2(140,50),ivec2(125,25));aswin->SetText("chipe","0");
	aswin->AddControl("testbox",Control::TEXT,ivec2(480,50),ivec2(125,25));aswin->SetText("testbox","tb");
	aswin->AddControl("loi",Control::TEXT,ivec2(280,50),ivec2(75,25));aswin->SetText("loi","0");
	aswin->AddControl("lor",Control::TEXT,ivec2(380,50),ivec2(75,25));aswin->SetText("lor","0");

	aswin->AddControl("l2",Control::LABEL,ivec2(5,85),ivec2(125,25));aswin->SetText("l2","Ramp Size:  IniVol:    EndVol:     IniPitch:   EndPitch:   Rtype:");
	aswin->AddControl("rsize",Control::TEXT,ivec2(5,100),ivec2(85,25));aswin->SetText("rsize","15");
	aswin->AddControl("inivol",Control::TEXT,ivec2(5+(1*95),100),ivec2(85,25));aswin->SetText("inivol","255");
	aswin->AddControl("endvol",Control::TEXT,ivec2(5+(2*95),100),ivec2(85,25));aswin->SetText("endvol","255");
	aswin->AddControl("inipit",Control::TEXT,ivec2(5+(3*95),100),ivec2(85,25));aswin->SetText("inipit","0");
	aswin->AddControl("endpit",Control::TEXT,ivec2(5+(4*95),100),ivec2(85,25));aswin->SetText("endpit","0");
	aswin->AddControl("itype",Control::COMBO,ivec2(5+(5*95),100),ivec2(105,25));aswin->SetText("itype","SQUARE|NOISE|TRIANGLE|SQNOISE|");
	aswin->AddControl("addr",Control::BUTTON,ivec2(5,140),ivec2(100,25));aswin->SetText("addr","Add ramp");
	aswin->AddControl("remr",Control::BUTTON,ivec2(5+(1*102),140),ivec2(100,25));aswin->SetText("remr","Remove ramp");
	aswin->AddControl("ralist",Control::LIST,ivec2(5,170),ivec2(600,300));aswin->SetText("ralist","");

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
int oselec=-1;

void InstrumentSet::Run(int status){
	aswin->RunWindow();
	for (int a = 0; a < 3; a++){eng[ms].runengine(a);}
	//RunAudio();

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
	int cha=0;
	bool refresh_list=false;
	if (eng[ms].g_insts[curins].ramps.size()>0){
		int gets = aswin->GetValue("ralist",0);
		if (gets>eng[ms].g_insts[curins].ramps.size()-1){gets=eng[ms].g_insts[curins].ramps.size()-1;}
		if (oselec!=gets){
			aswin->SetNumberAsText("rsize",eng[ms].g_insts[curins].ramps[gets].rsize);
			aswin->SetNumberAsText("inivol",eng[ms].g_insts[curins].ramps[gets].ivol);
			aswin->SetNumberAsText("endvol",eng[ms].g_insts[curins].ramps[gets].vvol);
			aswin->SetNumberAsText("inipit",eng[ms].g_insts[curins].ramps[gets].ipitch);
			aswin->SetNumberAsText("endpit",eng[ms].g_insts[curins].ramps[gets].vpitch);
			aswin->SetValue("itype",0,eng[ms].g_insts[curins].ramps[gets].type);
		}
		if (eng[ms].g_insts[curins].ramps[gets].rsize != aswin->GetTextAsNumber("rsize")){refresh_list=true;}
		if (eng[ms].g_insts[curins].ramps[gets].ivol != aswin->GetTextAsNumber("inivol")){refresh_list=true;}
		if (eng[ms].g_insts[curins].ramps[gets].vvol != aswin->GetTextAsNumber("endvol")){refresh_list=true;}
		if (eng[ms].g_insts[curins].ramps[gets].ipitch != aswin->GetTextAsNumber("inipit")){refresh_list=true;}
		if (eng[ms].g_insts[curins].ramps[gets].vpitch != aswin->GetTextAsNumber("endpit")){refresh_list=true;}
		if (eng[ms].g_insts[curins].ramps[gets].type != aswin->GetValue("itype",0)){refresh_list=true;}

		if (eng[ms].g_insts[curins].loopini != aswin->GetTextAsNumber("loi")){refresh_list=true;}
		if (eng[ms].g_insts[curins].looppt != aswin->GetTextAsNumber("lor")){refresh_list=true;}

		eng[ms].g_insts[curins].ramps[gets].rsize = aswin->GetTextAsNumber("rsize");
		eng[ms].g_insts[curins].ramps[gets].ivol = aswin->GetTextAsNumber("inivol");
		eng[ms].g_insts[curins].ramps[gets].vvol = aswin->GetTextAsNumber("endvol");
		eng[ms].g_insts[curins].ramps[gets].ipitch = aswin->GetTextAsNumber("inipit");
		eng[ms].g_insts[curins].ramps[gets].vpitch = aswin->GetTextAsNumber("endpit");
		eng[ms].g_insts[curins].ramps[gets].type = aswin->GetValue("itype",0);
		eng[ms].g_insts[curins].loopini = aswin->GetTextAsNumber("loi");
		eng[ms].g_insts[curins].looppt = aswin->GetTextAsNumber("lor");

		oselec = gets;
	}
	int ocurin=0;
	if (aswin->GetValue("prv",0)>0){if (curins>0){ocurin=curins--;cha=1;}}
	if (aswin->GetValue("nxt",0)>0){if (curins<15){ocurin=curins++;cha=1;}}
	if (cha>0){
		char tx[1000];aswin->GetText("name",tx);
		eng[ms].g_insts[ocurin].name=tx;
		eng[ms].g_insts[ocurin].loopini = aswin->GetTextAsNumber("loi");
		eng[ms].g_insts[ocurin].looppt = aswin->GetTextAsNumber("lor");

		Update();
		if (eng[ms].g_insts[curins].ramps.size()>0){
			aswin->SetNumberAsText("rsize",eng[ms].g_insts[curins].ramps[0].rsize);
			aswin->SetNumberAsText("inivol",eng[ms].g_insts[curins].ramps[0].ivol);
			aswin->SetNumberAsText("endvol",eng[ms].g_insts[curins].ramps[0].vvol);
			aswin->SetNumberAsText("inipit",eng[ms].g_insts[curins].ramps[0].ipitch);
			aswin->SetNumberAsText("endpit",eng[ms].g_insts[curins].ramps[0].vpitch);
			aswin->SetValue("itype",0,eng[ms].g_insts[curins].ramps[0].type);
		}
		refresh_list = true;
	}
	if (aswin->GetValue("addr",0)>0){
		eng[ms].g_insts[curins].ramps.push_back(Ramp());
		eng[ms].g_insts[curins].ramps.back().rsize = aswin->GetTextAsNumber("rsize");
		eng[ms].g_insts[curins].ramps.back().ivol = aswin->GetTextAsNumber("inivol");
		eng[ms].g_insts[curins].ramps.back().vvol = aswin->GetTextAsNumber("endvol");
		eng[ms].g_insts[curins].ramps.back().ipitch = aswin->GetTextAsNumber("inipit");
		eng[ms].g_insts[curins].ramps.back().vpitch = aswin->GetTextAsNumber("endpit");
		eng[ms].g_insts[curins].ramps.back().type = aswin->GetValue("itype",0);

		refresh_list = true;
	}
	if (aswin->GetValue("remr",0)>0){
		if (eng[ms].g_insts[curins].ramps.size()>0){
			refresh_list = true;
			int gets = aswin->GetValue("ralist",0);
			if (gets>eng[ms].g_insts[curins].ramps.size()-1){gets=eng[ms].g_insts[curins].ramps.size()-1;}
			eng[ms].g_insts[curins].ramps.erase(eng[ms].g_insts[curins].ramps.begin()+gets);
			if (eng[ms].g_insts[curins].ramps.size()>0){
				int gets = aswin->GetValue("ralist",0);
				aswin->SetNumberAsText("rsize",eng[ms].g_insts[curins].ramps[gets].rsize);
				aswin->SetNumberAsText("inivol",eng[ms].g_insts[curins].ramps[gets].ivol);
				aswin->SetNumberAsText("endvol",eng[ms].g_insts[curins].ramps[gets].vvol);
				aswin->SetNumberAsText("inipit",eng[ms].g_insts[curins].ramps[gets].ipitch);
				aswin->SetNumberAsText("endpit",eng[ms].g_insts[curins].ramps[gets].vpitch);
				aswin->SetValue("itype",0,eng[ms].g_insts[curins].ramps[gets].type);
			}
		}
	}
	if (refresh_list){
		std::string tbl;
		char tx[1000];
		for (int a = 0; a < eng[ms].g_insts[curins].ramps.size();a++){
			sprintf(tx,"size: %i, ivol: %i, evol: %i ", eng[ms].g_insts[curins].ramps[a].rsize,eng[ms].g_insts[curins].ramps[a].ivol,eng[ms].g_insts[curins].ramps[a].vvol);
			tbl+=tx;
			sprintf(tx,"ipit: %i, epit: %i, type: %i|", eng[ms].g_insts[curins].ramps[a].ipitch,eng[ms].g_insts[curins].ramps[a].vpitch,eng[ms].g_insts[curins].ramps[a].type);
			tbl+=tx;
		}
		aswin->SetText("ralist",tbl.c_str());
	}
	aswin->SetText("testbox","tb");
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
		dat.wbyte(eng[ms].g_insts[a].loopini);
		dat.wbyte(eng[ms].g_insts[a].looppt);
		dat.wshort(eng[ms].g_insts[a].ramps.size());
		for (int b = 0; b < eng[ms].g_insts[a].ramps.size();b++){
			dat.wshort(eng[ms].g_insts[a].ramps[b].ivol);
			dat.wshort(eng[ms].g_insts[a].ramps[b].ipitch);
			dat.wshort(eng[ms].g_insts[a].ramps[b].vvol);
			dat.wshort(eng[ms].g_insts[a].ramps[b].vpitch);
			dat.wbyte(eng[ms].g_insts[a].ramps[b].rsize);
			dat.wbyte(eng[ms].g_insts[a].ramps[b].type);
		}	
	}
	return dat.pos;

}

void InstrumentSet::Decode(unsigned char * data){
	DH dat;dat.data=data;dat.pos=0;
	eng.push_back(mus_engine());
	ms = eng.size() - 1;
	eng[ms].boot();
	insts.clear();
	insts.resize(dat.rshort());
	for (int a=0;a<insts.size();a++){
		int si=dat.rbyte();
		for (int b = 0; b < si;b++){
			insts[a].name+=dat.rbyte();
		}
		insts[a].chip = dat.rbyte();
		insts[a].instype = dat.rbyte();
		insts[a].effect = dat.rbyte();
		for (int b = 0; b < 2; b++){insts[a].effpar[b]=(char)dat.rbyte();}
		for (int b = 0;b < 3; b++){insts[a].adsr_siz[b]=(char)dat.rbyte();}
		insts[a].adsr_sus=dat.rbyte();
		si=dat.rbyte();
		insts[a].arpt.clear();
		for (int b = 0; b < si; b++){insts[a].arpt.push_back((char)dat.rbyte());}
		for (int b = 0;b < 32; b++){insts[a].data[b] = dat.rbyte();}
		insts[a].loopini = dat.rbyte();
		insts[a].looppt = dat.rbyte();
		insts[a].ramps.clear();
		int rcount = dat.rshort();		
		for (int b = 0; b < rcount;b++){
			insts[a].ramps.push_back(Ramp());
			insts[a].ramps.back().ivol=dat.rshort();
			insts[a].ramps.back().ipitch=dat.rshort();
			insts[a].ramps.back().vvol=dat.rshort();
			insts[a].ramps.back().vpitch=dat.rshort();
			insts[a].ramps.back().rsize=dat.rbyte();
			insts[a].ramps.back().type=dat.rbyte();
		}	
	}	
	eng[ms].g_insts = insts;
}
void InstrumentSet::Update(){
	aswin->SetNumberAsText("insnb",curins);
	aswin->SetText("name",eng[ms].g_insts[curins].name.c_str());
	aswin->SetNumberAsText("loi",eng[ms].g_insts[curins].loopini);
	aswin->SetNumberAsText("lor",eng[ms].g_insts[curins].looppt);
}


void MusicSet::Boot(bool newf){
	eng.push_back(mus_engine());
	ms=eng.size()-1;
	eng[ms].boot();
	
	//LoadFile("./base.uis", "base.uis", true);
	filt[0].label = "Music files"; filt[0].all = false;
	filt[0].extens.clear(); filt[0].extens.push_back(".uis");

	int pos = fpath.rfind('/');
	int opos = fpath.rfind('\\');
	if (opos > pos) { pos = opos; }
	std::string dir = fpath;dir.resize(pos+1);
	char odir[1024];
	strcpy(odir, dir.c_str());
	if (LoadDir(odir, false)) {
		for (int a = 0; a < df.size();a++){
			if (df[a].fname[0]!='.'){	
				printf("%s\n",df[a].fname);
				LoadFile((dir+df[a].fname).c_str(),df[a].fname,false);
				InstrumentSet* mus = (InstrumentSet*) fileoutld;
				for (int b = 0;b < eng[mus->ms].g_insts.size();b++){
					eng[ms].g_insts.push_back(eng[mus->ms].g_insts[b]);
				}
				eng.pop_back();
				delete mus;
			}
		}
	}
	
	aswin=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	aswin->AddControl("Tracker",Control::EPICTURE,ivec2(5,60),ivec2(294,404));
	aswin->AddControl("TrackerBar",Control::SCROLL,ivec2(299,60),ivec2(24,404));
	aswin->AddControl("BPM",Control::TEXT,ivec2(580,10),ivec2(100,24));aswin->SetText("BPM","150");
	aswin->AddControl("BPMlabel",Control::LABEL,ivec2(545,15),ivec2(100,24));aswin->SetText("BPMlabel","BPM:");
	aswin->AddControl("Timeline",Control::EPICTURE,ivec2(350,40),ivec2(644,290));
	aswin->AddControl("TimelineBar",Control::SCROLL,ivec2(350,330),ivec2(644,24));
	aswin->AddControl("Instrument",Control::LIST,ivec2(5,544),ivec2(294,120));
	aswin->AddControl("Tname",Control::TEXT,ivec2(50,40-4),ivec2(270,24));aswin->SetText("Tname","Track0");
	aswin->AddControl("Tnlabel",Control::LABEL,ivec2(5,40),ivec2(100,24));aswin->SetText("Tnlabel","Name:");

	aswin->AddControl("Toctl",Control::LABEL,ivec2(5,480),ivec2(100,24));aswin->SetText("Toctl","Octave:2");
	aswin->AddControl("Tdurl",Control::LABEL,ivec2(125,480),ivec2(100,24));aswin->SetText("Tdurl","Duration :1");

	aswin->AddControl("Tracks",Control::LIST,ivec2(350,384),ivec2(294,260));
	aswin->AddControl("Atrack",Control::BUTTON,ivec2(350,645),ivec2(120,24));aswin->SetText("Atrack","Create track");
	aswin->AddControl("Rtrack",Control::BUTTON,ivec2(470,645),ivec2(120,24));aswin->SetText("Rtrack","Remove track");
	aswin->AddControl("Play",Control::BUTTON,ivec2(350,10),ivec2(80,24));aswin->SetText("Play","Play");
	aswin->AddControl("Stop",Control::BUTTON,ivec2(435,10),ivec2(80,24));aswin->SetText("Stop","Stop");
	aswin->AddControl("PlayTWI",Control::BUTTON,ivec2(750,10),ivec2(120,24));aswin->SetText("PlayTWI","Play Twice");

	aswin->AddControl("PlayT",Control::BUTTON,ivec2(5,10),ivec2(100,24));aswin->SetText("PlayT","Play track");
	aswin->AddControl("StopT",Control::BUTTON,ivec2(110,10),ivec2(100,24));aswin->SetText("StopT","Stop track");

	aswin->AddControl("Addt",Control::BUTTON,ivec2(350,360),ivec2(120,24));aswin->SetText("Addt","Add track");

	songs.push_back(song());
	
	std::string musi;
	for (int a = 0;a < eng[ms].g_insts.size();a++){
		musi+=eng[ms].g_insts[a].name;
		musi+="|";
	}
	aswin->SetText("Instrument",musi.c_str());	
}

int bongct=0;
int noup=40;
int playing=0,songpos=0,nnote=0;

float noct=0,loopcount=0;
int notemode=0,onmouse = 0;
int curbnote[6];
int nostrx = 0,nostry = 0, grabnote = 0;
int seblock=0;
int curoct=2,cursiz=1;
int edge=0,edgenote=0;
int mustop=0,ntract=0;
int otrack=-1;
int tm_grab=-1,tm_ox,tm_oy,tm_gmode;
int otras=0;
void MusicSet::Run(int status){
	aswin->RunWindow();
	//Tracker
	std::string pia="..C C#D D#E F F#G G#A A#B ";
	int senote=0,seinst=0,scr=0;
	Box(ivec2(10,124),ivec2(295,520),color(0));
	Box(ivec2(354,104),ivec2(354+636,100+286),color(0));
	int curtrack=aswin->GetValue("Tracks",0);
	if (curtrack>songs[0].tracks.size()-1){curtrack=songs[0].tracks.size()-1;}

	if (songs[0].tracks.size()>0){
		if (otrack!=curtrack){
			aswin->SetText("Tname",songs[0].tracks[curtrack].name.c_str());
		}
		char namae[100];
		aswin->GetText("Tname",namae);
		songs[0].tracks[curtrack].name=namae;
		std::vector<track_nt> track = songs[0].tracks[curtrack].notes;
		int trasiz=songs[0].tracks[curtrack].size;
		//if (track.size()<1){track.resize(100);}
		scr=aswin->GetValue("TrackerBar",0)/10;
		for (int a = 0; a < 8;a++){
			int co = 128,ps=a+scr;
			if (seblock==ps){co=255;}
			Line(ivec2(10,124+(a*49)),ivec2(295,124+(a*49)),color(0,co,0));
			Line(ivec2(10,123+((a+1)*49)),ivec2(295,123+((a+1)*49)),color(0,co,0));
			if (ps<track.size()){
				Print(ivec2(10,125+(a*49)),color(0,co,0),"Note: %s(%i)",pia.substr(track[ps % 12].note*2,2).c_str(),track[ps % 12].note);
				Print(ivec2(10,135+(a*49)),color(0,co,0),"Duration: %i",track[ps].duration);
				if (track[ps].note==0){co*=0.5;}
				Print(ivec2(10,145+(a*49)),color(0,co,0),"Octave: %i",track[ps].oct);
				Print(ivec2(10,155+(a*49)),color(0,co,0),"Instrument: %s",eng[ms].g_insts[track[ps].inst].name.c_str());
			}else{
				Print(ivec2(10,125+(a*49)),color(0,co,0),"Empty");
			}

			if (mouse_bt==1 && MouseTest(ivec2(10,124),ivec2(285,400))){
				UnconnectKB();
				seblock = ((mouse_y-124)/49)+scr;
				if (seblock<track.size()){
					cursiz=track[seblock].duration;
				}
			}

			if (mouse_bt>1 && MouseTest(ivec2(10,124),ivec2(285,400))){
				UnconnectKB();
				seblock = ((mouse_y-124)/49)+scr;
			}
		}
		bool nope=false;
		for (int a = 0;a < strlen(kb);a++){
			if (keys[kb[a]]>0){
				nope=true;
				if (seblock>=track.size()){track.resize(seblock+1);}
				track[seblock].oct=curoct+(a/12);
				track[seblock].duration=cursiz;
				track[seblock].note=(a % 12)+1;
				track[seblock].inst = aswin->GetValue("Instrument",0);
				if (edgenote==0){
					eng[ms].playnote(track[seblock].inst,(track[seblock].note-1)+(track[seblock].oct*12),15,0);
					edgenote=1;
				}
			}		
		}
		if (!nope && edgenote==1){edgenote=0;eng[ms].stopnote(0);}
		if (keys[32]>0){
			if (seblock>=track.size()){track.resize(seblock+1);}
			track[seblock].note=0;track[seblock].duration = cursiz;
			int maxsiz=-1;
			for (int a = track.size()-1; a > -1;a--){
				if (track[a].note>0){maxsiz=a;break;}
			}
			std::vector<track_nt> bkp = track;
			track.clear();track.resize(maxsiz+2);
			for (int a = 0; a < maxsiz+2;a++){track[a] = bkp[a];}

		}
		if (edge==0){
			if (keys[0+128]>0 && curoct<8){curoct++;edge=1;}
			if (keys[2+128]>0 && curoct>1){curoct--;edge=1;}
			if (keys[1+128]>0 && cursiz<8){cursiz++;edge=1;}
			if (keys[3+128]>0 && cursiz>1){cursiz--;edge=1;}
			if (edge>0){
				if (seblock>=track.size()){track.resize(seblock+1);}
				track[seblock].oct=curoct;
				track[seblock].duration=cursiz;
				track[seblock].inst = aswin->GetValue("Instrument",0);
			}
		}
		int sm=0;for (int a = 0;a < 4;a++){sm+=keys[a+128];}if (sm==0){edge=0;}
		std::string pr;
		pr="Octave:"+std::to_string(curoct);aswin->SetText("Toctl",pr.c_str());
		pr="Duration:"+std::to_string(cursiz);aswin->SetText("Tdurl",pr.c_str());

		if (aswin->GetValue("PlayT",0)>0){
			BuildTrack(0,curtrack);
			playing=1;
			songpos = 0;
			noct=1.0;
			nnote=0;
			for (int a = 0; a < 6;a++){
				bake_note_mpos[a]=0;
				bake_note_pos[a]=0;
			}

		}
		if (aswin->GetValue("StopT",0)>0){
			playing=0;
			for (int a = 0; a < 6;a++){eng[ms].stopnote(a);}
		}
		trasiz=0;for (int b = 0;b<track.size();b++){trasiz+=track[b].duration;};
		songs[0].tracks[curtrack].size = trasiz;
		songs[0].tracks[curtrack].notes = track; 
	//Timeline
		for (int a = 1; a < 7;a++){
			Line(ivec2(354,(a*40)+104),ivec2(354+636,(a*40)+104),color(0,128,0));
		}
		int sc2=aswin->GetValue("TimelineBar",0)/4;

		for (int a = 0; a < songs[0].time_ent.size();a++){
			int ini=(songs[0].time_ent[a].start) ,siz=(songs[0].time_ent[a].start+songs[0].time_ent[a].size),ps=songs[0].time_ent[a].lane;
			int tra=songs[0].time_ent[a].track;
			ini-=sc2;siz-=sc2;
			int inisqui=ini;
			if (ini<0){ini=0;}
			if (siz>80){siz=80;}

			if (ini<80 && siz>ini){
				Box(ivec2((ini*8)+356,(ps*40)+106),ivec2((siz*8)+356,((ps+1)*40)+106),color(0,64,0));
				Box(ivec2((ini*8)+356,(ps*40)+106),ivec2((siz*8)+353,((ps+1)*40)+103),color(0,255,0));
				Box(ivec2((ini*8)+359,(ps*40)+109),ivec2((siz*8)+353,((ps+1)*40)+103),color(0,128,0));
				int si=(songs[0].tracks[tra].size*8);
				for (int b = 0; b < (siz-inisqui)*8;b+=si){
					int i=inisqui;
					if (((i*8)+b)>0){
						Line(ivec2((i*8)+356+b,(ps*40)+106),ivec2((i*8)+356+b,(ps*40)+146),color(0,64,0));
						Line(ivec2((i*8)+357+b,(ps*40)+106),ivec2((i*8)+357+b,(ps*40)+146),color(0,255,0));
					}
				}
				Print(ivec2((ini*8)+359,(ps*40)+109),color(0),songs[0].tracks[tra].name.c_str());
			}
		}
		if (aswin->GetValue("Addt",0)>0){
			songs[0].time_ent.push_back(tl_entry());
			songs[0].time_ent.back().start=0+sc2;
			songs[0].time_ent.back().track=aswin->GetValue("Tracks",0);
			songs[0].time_ent.back().size=songs[0].tracks[aswin->GetValue("Tracks",0)].size;
			if (songs[0].time_ent.back().size<4){songs[0].time_ent.back().size=4;}
		}
		if (MouseTest(ivec2(350,40),ivec2(644,350))){
			if (mouse_bt>0 && olmouse_bt==0){
				int sel = -1, pg=0;
				for (int a = 0; a < songs[0].time_ent.size();a++){
					int ini=((songs[0].time_ent[a].start)*8)+356,siz=((songs[0].time_ent[a].start+songs[0].time_ent[a].size)*8)+356,ps=(songs[0].time_ent[a].lane*40)+103;
					ini-=sc2*8;siz-=sc2*8;
					if (mouse_x>ini && mouse_x<siz && mouse_y>ps && mouse_y<ps+40){sel = a;pg=siz-mouse_x;tm_ox=mouse_x-ini;tm_oy=mouse_y-ps;break;}
				}
				if (mouse_bt==1){
					tm_grab = sel;
					tm_gmode = 0;if (pg<10){tm_gmode=1;}
				}
				if (mouse_bt>1){songs[0].time_ent.erase(songs[0].time_ent.begin()+sel);}
			}
			if (tm_grab>-1 && mouse_bt==1 && tm_gmode==0){
				
				songs[0].time_ent[tm_grab].start=((((mouse_x-tm_ox)-356))/8)+sc2;
				songs[0].time_ent[tm_grab].lane=(((mouse_y-tm_oy)-103))/40;
				if (songs[0].time_ent[tm_grab].start<0){songs[0].time_ent[tm_grab].start=0;}
				if (songs[0].time_ent[tm_grab].lane<0){songs[0].time_ent[tm_grab].lane=0;}
				if (songs[0].time_ent[tm_grab].lane>6){songs[0].time_ent[tm_grab].lane=6;}
			}
			if (tm_grab>-1 && mouse_bt==1 && tm_gmode==1){
				songs[0].time_ent[tm_grab].size=(((mouse_x-356)/8)+sc2)-songs[0].time_ent[tm_grab].start;
				if (songs[0].time_ent[tm_grab].size<2){songs[0].time_ent[tm_grab].size=2;}
			}
		}
	}
	if (aswin->GetValue("Atrack",0)>0){
		songs[0].tracks.push_back(track());
		songs[0].tracks.back().name="Ntrack"+std::to_string(ntract++);
		std::string tra;
		for (int a = 0;a < songs[0].tracks.size();a++){
			tra+= songs[0].tracks[a].name + "|";
		}
		aswin->SetText("Tracks",tra.c_str());
	}
	if (otras!=songs[0].tracks.size()){
		std::string tra;
		for (int a = 0;a < songs[0].tracks.size();a++){
			tra+= songs[0].tracks[a].name + "|";
		}
		aswin->SetText("Tracks",tra.c_str());
	}
	otras=songs[0].tracks.size();

	otrack=curtrack;
	//Music player
	if (aswin->GetValue("Play",0)>0 || aswin->GetValue("PlayTWI",0)>0){
		playing = 1;
		songpos = 0;
		noct=1.0;
		nnote=0;
		loopcount=0;
		if (aswin->GetValue("PlayTWI",0)>0){loopcount=1;}
		for (int a = 0; a < 6;a++){
			bake_note_mpos[a]=0;
			bake_note_pos[a]=0;
		}
		BuildMusic(0);
	}
	float BPM = aswin->GetTextAsNumber("BPM");
	songs[0].bpm = BPM;
	if (aswin->GetValue("Stop",0)>0){playing = 0;
		for (int a = 0; a < 6;a++){eng[ms].stopnote(a);}
	}
	if (playing){
		noct+=(4.0/3600.0)*BPM;
		if (noct>0.9999){
			noct = 0;
			//if (songpos > songs[0].bakesize){for (int a = 0;a < 3;a++){eng[ms].stopnote(a);}playing = 0;}
			for (int a = 0 ;a < 3; a++){
				while (bake_note_mpos[a]==songpos && bake_note_pos[a]<songs[0].bakemusic[a].bt.size()){
					int e1 = songs[0].bakemusic[a].bt[bake_note_pos[a]].e1;
					int e2 = songs[0].bakemusic[a].bt[bake_note_pos[a]].e2;
					if (e1 < 128){
						eng[ms].playnote(bake_note_instr[a],e1,15,a);
					}else{
						if (e1==128){eng[ms].stopnote(a);}
						if (e1==129){bake_note_instr[a] = e2;}
						if (e1==132){
							
							if (loopcount>0){
								songpos = 0;
								//noct=1.0;
								nnote=0;
								loopcount--;
								for (int a = 0; a < 6;a++){
									bake_note_mpos[a]=0;bake_note_pos[a]=0;
								}
								printf("Re loop!\n");
							}else{
								playing=false;
								eng[ms].stopnote(0);
								eng[ms].stopnote(1);
								eng[ms].stopnote(2);
								printf("End!\n");
							}
						}
					}
					if (e1<129){bake_note_mpos[a] += (e2 & 15);}

					bake_note_pos[a]++;
				}
			}
			songpos++;
		}

	}

	onmouse = mouse_bt;
	for (int a = 0; a < 3; a++){eng[ms].runengine(a);}
	//RunAudio();

}
void MusicSet::BuildTrack(int mus,int track){
	// 0-127, notes (byte 2 controls duration and note spacing)
	// bit 128 up = commands
	// 128 = mute note
	// 129 = instrument
	// 130 = volume
	// 131 = stop channel
	// 132 = loop song (só colocado no canal mais longo)
	if (songs[mus].tracks[track].notes.size()==0){return;}
	int buscha[6],sicha[6],inscha[6];
	for (int b = 0; b < 6;b++){
		songs[mus].bakemusic[b].bt.clear();
		buscha[b] = 0;sicha[b] = 0;	inscha[b] = -1;
	}
	std::vector<track_nt> nt = songs[mus].tracks[track].notes;  
	int oins=nt[0].inst;
	int totsize=0;
	songs[mus].bakemusic[0].bt.push_back(bakecmd(129,nt[0].inst));
	int acsize=0;
	for (int b=nt.size()-1;b >-1 ;b--){
		if (nt[b].duration>0){acsize=b+1;break;}
	}
	
	for (int b=0;b < acsize;b++){
		int nto = (nt[b].note-1) + nt[b].oct*12; 
		int dur = nt[b].duration;
		if (dur>0){
			totsize+=dur;
			if (oins!=nt[b].inst){songs[mus].bakemusic[0].bt.push_back(bakecmd(129,nt[b].inst));}
			oins = nt[b].inst;
			if (songs[mus].tracks[track].notes[b].note>0){
				songs[mus].bakemusic[0].bt.push_back(bakecmd(nto,dur));
			}else{
				songs[mus].bakemusic[0].bt.push_back(bakecmd(128,dur));
			}
		}
	}
	songs[mus].bakesize = totsize;
}

void MusicSet::BuildMusic(int mus){
	//Well, i decided to make it simpler (and bigger)
	// 0-127, notes (byte 2 controls duration and note spacing)
	// bit 128 up = commands
	// 128 = mute note
	// 129 = instrument
	// 130 = volume
	// 131 = stop channel
	// 132 = loop song (só colocado no canal mais longo)
	int totsize=0,selcha=0;
	for (int a = 0; a < 6;a++){
		int cpos=0;
		int ipos=0;
		int finote=1;
		songs[mus].bakemusic[a].bt.clear();
		for (int b = 0; b < songs[mus].time_ent.size();b++){
			if (songs[mus].time_ent[b].lane==a && songs[mus].time_ent[b].start >= cpos){
				std::vector<track_nt> nt = songs[mus].tracks[songs[mus].time_ent[b].track].notes;  
				int acsize=0;
				for (int c=nt.size()-1;c >-1 ;c--){if (nt[c].duration>0){acsize=c+1;break;}}

				ipos=songs[mus].time_ent[b].start;
				songs[mus].bakemusic[a].bt.push_back(bakecmd(129,nt[0].inst));
				int siza=ipos-cpos;
				if (siza>0){
					while (siza>15){songs[mus].bakemusic[a].bt.push_back(bakecmd(128,15));siza-=15;}
					songs[mus].bakemusic[a].bt.push_back(bakecmd(128,siza));
				}

				int oins=nt[0].inst;
				int tsize=ipos;
				int c=0,maxsi=songs[mus].time_ent[b].size+ipos;
				while (tsize<maxsi){
					int nto = (nt[c].note-1) + nt[c].oct*12; 
					int dur = nt[c].duration;
					int clip=0;
					if (dur>0){
						tsize+=dur;
						if (tsize>maxsi){clip=tsize-maxsi;}
						if (nt[c].note>0){
							if (oins!=nt[c].inst){songs[mus].bakemusic[a].bt.push_back(bakecmd(129,nt[c].inst));}
							oins = nt[c].inst;
						}
						if (nt[c].note>0){
							songs[mus].bakemusic[a].bt.push_back(bakecmd(nto,dur-clip));
						}else{
							songs[mus].bakemusic[a].bt.push_back(bakecmd(128,dur-clip));
						}
					}
					c++;c%=acsize;
				}
				cpos = songs[mus].time_ent[b].start+songs[mus].time_ent[b].size;
				if (tsize>totsize){totsize=tsize;selcha=a;}
			}
		}
	}
	songs[mus].bakesize = totsize;
	for (int a =0; a < 6;a++){
		if (a==selcha){
			songs[mus].bakemusic[a].bt.push_back(bakecmd(132,0));
		}else{
			songs[mus].bakemusic[a].bt.push_back(bakecmd(131,0));
		}
	}
	for (int a = 0; a < 3; a++){
		printf("Canal %i:\n",a);
		int ct=0;
		int size=0;
		for (int b = 0;b < songs[mus].bakemusic[a].bt.size();b++){
			printf("(%i, %i) ",songs[mus].bakemusic[a].bt[b].e1,songs[mus].bakemusic[a].bt[b].e2);
			if (ct>8){ct=0;printf("\n");}
			if (songs[mus].bakemusic[a].bt[b].e1<129){
				size+=songs[mus].bakemusic[a].bt[b].e2;
			}
		}
		printf("\n");
		printf("Tamanho:%i\n",size);
	}
}