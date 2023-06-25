#ifndef COMPILER_H_INCLUDED
#define COMPILER_H_INCLUDED
#include "data.h"
std::string Compile();
std::string CompilePack();
std::vector <int> AssembleProgramNew(std::vector<std::string> prg,int orga, std::vector<o_macro> mcs);
extern int copyit;
extern std::string copyto;
extern std::string asmerr;
void TestAssembler();
#endif