#include "MainWindow.h"
#include "data.h"
#include <sstream>

std::vector<funccal> fcalls;
std::vector <int> AssembleProgramNew(std::vector<std::string> prg,int orga, std::vector<o_macro> mcs);


struct asmin{
	std::string input;
	std::vector <int> opnumbers; //0-255 opnumber, -1 input number
	int inbytes=0; //Number of byte inputs
};

std::vector <asmin> asmins;
std::vector <std::string> notnumb;

std::vector <std::string> splitins(std::string pr){
	std::vector <std::string> rt;
		rt.push_back(std::string());
		int first = 0;
		for (int a = 0;a < pr.size(); a++){
			if (pr[a] == ';'){break;} //It's comment so all processing is not needed
			if (pr[a]>32 && first == 0){first = 1;}
			if ((pr[a] < 33 && first == 1) || pr[a] == ','){rt.push_back(std::string()); first = 2;}
			if (pr[a]>32 && pr[a]!=','){
				char nn = pr[a];if (nn > 96 && nn < 123){nn = nn & 223;}rt.back()+=nn;
			}
		}
		if (rt.back()==""){rt.pop_back();}
	return rt;
}

std::string generic(std::string in,std::string let){
	int sta = 0;
	std::string ret = in;
	for (int a = 0;a < in.size();a++){
		if (in[a]>47 && in[a]<58){sta = a;break;}
	}
	if (sta>0){
		int nu = std::stoi(in.c_str()+sta);
		if (nu>255){
			ret=in.substr(0,sta)+let+let+let+let+")";
		}else{
			ret=in.substr(0,sta)+let+let+")";
		}

	}

	return ret;
}

void ProgramFile::Boot(){
	aswin=new Window(rd,50,ivec2(0,60),ivec2(1024,600));
	aswin->AddControl("Textin",Control::EPICTURE,ivec2(5,5),ivec2(990,400));
	aswin->AddControl("Scroll",Control::SCROLL,ivec2(990,5),ivec2(26,400));
	aswin->AddControl("Build",Control::BUTTON,ivec2(5,410),ivec2(100,30));aswin->SetText("Build","Build");
	aswin->AddControl("SaveTex",Control::BUTTON,ivec2(105,410),ivec2(120,30));aswin->SetText("SaveTex","Save as text");
	aswin->AddControl("SaveBin",Control::BUTTON,ivec2(225,410),ivec2(120,30));aswin->SetText("SaveBin","Save as binary");
	aswin->AddControl("Output",Control::TEXT,ivec2(5,450),ivec2(990,30));
	aswin->AddControl("APIbox",Control::LIST,ivec2(5,485),ivec2(990,150));
	aswin->AddControl("APIdesc",Control::TEXT,ivec2(5,635),ivec2(990,30));
	std::string ac;
	for (int a = 0;a < fcalls.size();a++){
		ac+=fcalls[a].name;ac+="|";
	}
	aswin->SetText("APIbox",ac.c_str());
	if (prg.size()<1){
		prg.push_back(";ix is: 0 type, 1 level pos id, 2-3 x,4 y, 5-8 input values");
		prg.push_back("ld a,$80 ;This gets run with b as $80 everytime a new instance is called");
		prg.push_back("cp b");
		prg.push_back("jp nz,runprogram");
		prg.push_back("ld a,$ff ;And you must respond 0 to 'yep, give me some memory', or $ff for do not allocate");
		prg.push_back("ret ;you can use it to make a one shot thing");
		prg.push_back(";you also get the program parameters in ix if you need to check things such as if its the same");
		prg.push_back(";or get parameters for one shot functions");
		prg.push_back("runprogram:");
		prg.push_back("ret");
	}
}
std::string atest;
int keyedge=0;
int curline=0;
bool focus=true;
std::vector<int> out;
std::string outt;
void ProgramFile::Run(int status){
	strcpy(flexmenu,"Level|Import|Export");
	aswin->RunWindow();
	Box(ivec2(10,70),ivec2(990,460),color(255));
	if (!MouseTest(ivec2(10,70),ivec2(990,460)) && mouse_bt > 0){
		focus = false;
	}
	if (MouseTest(ivec2(10,70),ivec2(990,460)) && mouse_bt > 0){
		int cn = (mouse_y - 70)/15;
		if (cn < prg.size()){
			curpos = (mouse_x-40)/8;
			if (curpos<0){curpos=0;}
			if (curpos>prg[curline].size()){curpos=prg[curline].size();}
			curline = cn;
		}
	 	focus=true;
	 }
	if (focus){
	    std::string note;
		if (prg.size()==0){prg.push_back(note);}
		if (enterpressed>0 && keyedge == 0){prg.insert(prg.begin()+curline+1,note);keyedge=1;curline++;}
		if (keys[tsdl(SDLK_UP)]>0 && keyedge==0){curline--;keyedge=1;}
		if (keys[tsdl(SDLK_DOWN)]>0 && keyedge==0){curline++;keyedge=1;}
		if ((enterpressed+keys[tsdl(SDLK_UP)]+keys[tsdl(SDLK_DOWN)])==0){keyedge=0;}
		if (keys[tsdl(SDLK_DELETE)]>0 && keyedge==0 && prg[curline].size()<1){prg.erase(prg.begin()+curline);keyedge=1;}
		if (curline<0){curline=0;}
		if (curline>prg.size()-1){curline=prg.size()-1;}
		if (curpos>prg[curline].size()){curpos=prg[curline].size();}
		ConnectKBD(&prg[curline]);
	}

	for (int a = 0;a < prg.size();a++){
		Print(ivec2(10,70+a*15),color(0,128,255),"%i",a + 1);
		Print(ivec2(40,70+a*15),color(0,0,0),"%s",prg[a].c_str());
		if (a == curline){
			if (focus){Print(ivec2(40+(curpos*8),70+a*15),color(0,0,0),"_");}
		}
	}
	if (aswin->GetValue("Build",0)>0){
		std::vector <o_macro> empty;
		out= AssembleProgramNew(prg,0,empty);
		char st[8];
		outt="";
		for (int a =0; a < out.size();a++){
			sprintf(st,"%#02x,",out[a]);
			outt+=st;
		}
		aswin->SetText("Output",outt.c_str());
	}
	if (aswin->GetValue("SaveTex",0)>0){
		FILE * f = fopen("out.txt","w");
		fprintf(f,outt.c_str());
		fclose(f);
	}
	if (aswin->GetValue("SaveBin",0)>0){
		unsigned char opr[65535];
		for (int a = 0;a < out.size();a++){opr[a]=out[a];}
		FILE * f = fopen ("out.bin","wb");
		fwrite(opr,1,out.size(),f);
		fclose(f);
	}
	int vv=aswin->GetValue("APIbox",0);
	if (vv>fcalls.size()-1){vv=fcalls.size()-1;}
	aswin->SetText("APIdesc",fcalls[vv].desc.c_str());
}


bool isnum(char * ch){
	bool isit = true;
	for (int a = 0; a < strlen(ch);a++){if (ch[a] < 48 || ch[0]>57){isit = false;break;}}
	return isit;
}
bool isnum(std::string ch){
	bool isit = true;
	for (int a = 0; a < ch.size();a++){if (ch[a] < 48 || ch[0]>57){isit = false;break;}}
	return isit;
}

void TestAssembler(){
}

int ProgramFile::Encode(unsigned char* data){
	int size = 0;
	char enter[]="\n";
	for (int a = 0; a < prg.size();a++){
		for (int b = 0;b < prg[a].size();b++){
			data[size++]=prg[a][b];
		}
		for (int b = 0;b < strlen(enter);b++){data[size++]=enter[b];}		
	}
	return size;
}

void ProgramFile::Decode(unsigned char* data){
	prg.clear();
	prg.push_back(std::string());
	int lin=0;
	char enter[]="\n";
	for (int a = 0; a < strlen((char*)data);a++){
		if (data[a]==enter[0]){
			a+=strlen(enter)-1;lin++;
			prg.push_back(std::string());
		}else{
			prg[lin]+=data[a];
		}
	}
}

void loaddb(){
	char lin[200];
	FILE * fa = fopen("z80asm.txt","r");
	while (fgets(lin,200,fa)){
		int pos=0;
		for (int a = 1;a < strlen(lin);a++){
			if (lin[a]==';'){pos = a - 1;break;}
		}
		asmins.push_back(asmin());
		asmins.back().input = lin; asmins.back().input=asmins.back().input.substr(0,pos);
		char mini[3];
		mini[2]=0;
		for (int a = pos+2; a < strlen(lin)-2;a+=2){
			mini[0]=lin[a];mini[1]=lin[a+1];
			if (mini[0]=='X'){
				asmins.back().opnumbers.push_back(-1);asmins.back().inbytes++;
			}else{
				asmins.back().opnumbers.push_back(std::stoi(mini,nullptr,16));
			}
			
		}
	}
	fclose(fa);
	notnumb.push_back("NC");notnumb.push_back("NZ");notnumb.push_back("");
	for (int a = 0;a < asmins.size();a++){
		std::vector <std::string> sp = splitins(asmins[a].input);
		for (int c = 1; c < sp.size();c++){
			bool nope = false;
			for (int b = 0; b < sp[c].size();b++){
				if (sp[c][b]=='N'){nope = true;break;}
			}			
			if (isnum(sp[c])){nope = true;}
			if (!nope){
				nope =false;
				for (int b = 0; b < notnumb.size();b++){
					if (notnumb[b] == sp[c]){nope = true;break;}
				}
				if (!nope){notnumb.push_back(sp[c]);}
			}
		}		
	}
}

struct math_line{
	std::string pro;
	std::vector<int> offs;
};

std::string asmerr;

std::vector <int> AssembleProgramNew(std::vector<std::string> prg, int orga, std::vector<o_macro> mcs){
	asmerr="";
	std::string curline;
	std::string curlineb;
	std::vector<int> rt;
	std::vector<std::string> labels;
	std::vector<int> labelpos;
	std::vector<math_line> mts;
	for (int a = 0; a < prg.size();a++){
		std::vector <std::string> op = splitins(prg[a]);
		if (op.size()>0){
			if (op[0][op[0].size()-1]==':'){
				labels.push_back(op[0].substr(0,op[0].size()-1));
				labelpos.push_back(orga + rt.size());
				op.erase(op.begin());
			}
			if (op[0]==".DB"){
				for (int b = 1; b < op.size();b++){
					bool mathed = false;
					if (isnum(op[b])){rt.push_back(std::stoi(op[b]) & 255);mathed = true;}
					if (!mathed && op[b].size()>0){
						rt.push_back(0);
						mts.push_back(math_line()); mts.back().pro = op[b];mts.back().offs.push_back(rt.size()-1);
					}
				}
			}
			if (op[0]==".DW"){
				for (int b = 1; b < op.size();b++){
					bool mathed = false;
					if (isnum(op[b])){rt.push_back(std::stoi(op[b]) & 255);rt.push_back((std::stoi(op[b])/256) & 255);mathed = true;}
					if (!mathed && op[b].size()>0){
						rt.push_back(0);rt.push_back(0);
						mts.push_back(math_line()); mts.back().pro = op[b];mts.back().offs.push_back(rt.size()-2);mts.back().offs.push_back(rt.size()-1);
					}

				}
			}


		}
		int number=0; //Get which one of the operands is a number if any
		for (int b = 1; b < op.size();b++){
			bool numb = true;
			for (int a = 0;a < notnumb.size();a++){if (op[b] == notnumb[a]){numb = false;break;}}
			if (numb){number=b;break;}
		}
		std::string hd="",ed="";
		std::string pro;
		if (number > 0){
			std::string pro = op[number];
			if (pro.substr(0,4)=="(IX+"){hd="(IX+";ed=")";pro = pro.substr(4,pro.size()-5);}
			if (pro.substr(0,4)=="(IY+"){hd="(IY+";ed=")";pro = pro.substr(4,pro.size()-5);}
			if (pro.substr(0,1)=="("){hd="(";ed=")";pro = pro.substr(1,pro.size()-2);}
			mts.push_back(math_line()); mts.back().pro = pro;
		}
		
		if (op.size()>0){
			curline = op[0];
			curlineb = op[0];
			if (op.size()>1){if (number == 1){curline+=" "+hd+"N"+ed;curlineb+=" "+hd+"NN"+ed;}else{curline+=" "+op[1];curlineb+=" "+op[1];}}			
			if (op.size()>2){if (number == 2){curline+=","+hd+"N"+ed;curlineb+=","+hd+"NN"+ed;}else{curline+=","+op[2];curlineb+=","+op[2];}}			
			int sel=-1;
			for (int b = 0;b < asmins.size();b++){
				//It had a test of <256 of the result on the first line
				if (asmins[b].input == curline){sel=b;break;} //Only use a 16bit solution if no 8bit one that matches is found
				if (asmins[b].input == curlineb){sel=b;}
			}
			if (sel>-1){
				for (int b=0;b < asmins[sel].opnumbers.size();b++){
					if (asmins[sel].opnumbers[b]<0){
						rt.push_back(0);mts.back().offs.push_back(rt.size()-1);
					}else{
						rt.push_back(asmins[sel].opnumbers[b]);
					}
				}
			}else{
				asmerr+="Line:";asmerr+=std::to_string(a+1) ; asmerr+=" is invalid|";
			}
		}
	}
	for (int a = 0;a < mts.size();a++){
		std::vector <std::string> ops;
		std::vector <int> ope;
		std::vector <int> opsnu;
			
		ops.push_back("");
		for (int c = 0; c < mts[a].pro.size();c++){
			bool sk=false;
			if (mts[a].pro[c]=='*'){ope.push_back(0);ops.push_back("");sk=true;}
			if (mts[a].pro[c]=='/'){ope.push_back(1);ops.push_back("");sk=true;}
			if (mts[a].pro[c]=='+'){ope.push_back(2);ops.push_back("");sk=true;}
			if (mts[a].pro[c]=='-'){ope.push_back(3);ops.push_back("");sk=true;} //Pemdas heh
			if (!sk){ops.back()+=mts[a].pro[c];}
		}
			
		opsnu.resize(ops.size(),0);
		for (int c = 0;c < ops.size();c++){
			if (isnum(ops[c])){opsnu[c] = std::stoi(ops[c]);}
			if (ops[c][0]=='$'){opsnu[c] = std::stoi(&ops[c][1],nullptr,16);}
			for (int d = 0; d < labels.size();d++){
				if (ops[c] == labels[d]){opsnu[c] = labelpos[d];break;}
			}
			for (int d = 0; d < fcalls.size(); d++){
				if (ops[c] == fcalls[d].uname){opsnu[c] = 0x50 + d * 3;break;}
			}
			for (int d = 0; d < mcs.size();d++){
				if (ops[c] == mcs[d].name){opsnu[c] = mcs[d].id;break;}
			}
			//Add pointers and stuff here
		}


		std::vector <int> opst1,opst2;
		for (int p = 0; p < 3;p++){
			opst1.clear();opst2.clear();
			for (int b = 0;b < ope.size();b++){
				if (ope[b]==p){
					if (p==0){opsnu[b+1]=opsnu[b]*opsnu[b+1];}
					if (p==1){opsnu[b+1]=opsnu[b]/opsnu[b+1];}
					if (p==2){opsnu[b+1]=opsnu[b]+opsnu[b+1];}
					if (p==3){opsnu[b+1]=opsnu[b]-opsnu[b+1];}
					ope.erase(ope.begin()+b);
					opsnu.erase(opsnu.begin()+b);
					b--;
				}
			}
		}
		int lval=opsnu[0]; //After all the math, we have the answer
		int divi=1;
		for (int b = 0; b < mts[a].offs.size();b++){
			rt[mts[a].offs[b]]=((lval/divi) & 255);
			divi*=256;
		}
	}
	
	return rt;
}


void fllldescs(){
	fcalls.clear();
	fcalls.push_back(funccal("CreateSprite","Creates a new sprite. L Type,DE X,Y,B Flip X,Y,C Frame. A returns The Object ID"));
	fcalls.push_back(funccal("MoveSprite","Sets the sprite position. C Object ID, DE X,Y"));
	fcalls.push_back(funccal("SetFrameFlip","Sets sprite frame and flip. C Object ID,DE frame,flip"));
	fcalls.push_back(funccal("GetSpritePos","Get sprite position. C Object ID. DE returns position"));
	fcalls.push_back(funccal("RemoveSprite","Destroy the sprite. C Object ID"));
	fcalls.push_back(funccal("GetScreenCoord","HL global position, returns A as screen coord (or 255 as out of screen)"));
	fcalls.push_back(funccal("FindObj","IY start position (set to 7400 to begin), C type, Returns A to 1 if EOL, IY with last address"));
	fcalls.push_back(funccal("BoxTest","E points to the offset in IX pointer with position, size and point, returns 1 to A if in box"));
	fcalls.push_back(funccal("CreateDot","HL position, C dot type,returns in A"));
	fcalls.push_back(funccal("UpdateDot","HL position, E ID, set to FFFF to kill it"));
	fcalls.push_back(funccal("ChangeDot","B new dot type, E ID"));
	fcalls.push_back(funccal("QueryDots","HL position, DE size,C dot type (only high nibble),A to dot ID if found"));
	fcalls.push_back(funccal("SpawnProgram","HL pointer to spawn table, returns memory pointer in HL"));
	fcalls.push_back(funccal("AddEffect", "A effect ID , bc X pos (16bit), D Y pos, E flip"));
	fcalls.push_back(funccal("GetTileType", "HL X (yep,16bit), D y, Return on A"));
	fcalls.push_back(funccal("AddItem", "bc X pos (16bit), D Y pos, E max type (add random item)"));
 
	//Only high nibble because i may want to make several different kinds but still compatible dots
	loaddb();
}