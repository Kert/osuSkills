#ifndef _tweakvars_h_
#define _tweakvars_h_

#include <string>

void LoadFormulaVars();
bool FormulaVarsLoaded();
double GetVar(std::string skill, std::string name);

#endif