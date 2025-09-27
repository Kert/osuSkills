#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>
#include "INIReader.h"

std::string config = "config.cfg";
static bool varsLoaded = false;
// skill name / variable name / value
static std::unordered_map<std::string, std::unordered_map<std::string, double>> VARS;

using std::cout;
using std::endl;

void SaveFormulaVars()
{
	std::ofstream file(config);
	if (file.is_open())
	{
		for(auto skill : VARS)
		{
			file << "[" << skill.first << "]" << endl;
			for(auto var : skill.second)
			{
				file << var.first << "=" << var.second << endl;
			}
			file << endl;
		}
		file.close();
	}
}

void ResetFormulaVars()
{
	VARS["Stamina"]["Decay"] = 0.94;
	VARS["Stamina"]["Mult"] = 0.8;
	VARS["Stamina"]["Pow"] = 0.1;
	VARS["Stamina"]["DecayMax"] = 0;
	VARS["Stamina"]["Scale"] = 7000;
	VARS["Stamina"]["LargestInterval"] = 50550.0;
	VARS["Stamina"]["TotalMult"] = 4.6;
	VARS["Stamina"]["TotalPow"] = 0.75;
	
	VARS["Tenacity"]["IntervalMult"] = 0.37;
	VARS["Tenacity"]["IntervalMult2"] = 13000;
	VARS["Tenacity"]["IntervalPow"] = 0.143;
	VARS["Tenacity"]["LengthDivisor"] = 0.08;
	VARS["Tenacity"]["LengthMult"] = 15.0;
	VARS["Tenacity"]["TotalMult"] = 5;
	VARS["Tenacity"]["TotalPow"] = 0.75;
	
	VARS["Agility"]["DistMult"] = 1;
	VARS["Agility"]["DistPow"] = 1;
	VARS["Agility"]["DistDivisor"] = 2;
	VARS["Agility"]["TimeMult"] = 0.001;
	VARS["Agility"]["TimePow"] = 1.04;
	VARS["Agility"]["StrainDecay"] = 16.9201;
	VARS["Agility"]["AngleMult"] = 4;
	VARS["Agility"]["SliderStrainDecay"] = 2;
	VARS["Agility"]["Weighting"] = 0.78;
	VARS["Agility"]["TotalMult"] = 30;
	VARS["Agility"]["TotalPow"] = 0.28;
	
	VARS["Accuracy"]["AccScale"] = 0.01;
	VARS["Accuracy"]["VerScale"] = 0.30;
	VARS["Accuracy"]["TotalMult"] = 23100;
	VARS["Accuracy"]["TotalPow"] = 1.3;
	
	VARS["Precision"]["AgilityLimit"] = 700;
	VARS["Precision"]["AgilityPow"] = 0.1;
	VARS["Precision"]["AgilitySubtract"] = 0.995462;
	VARS["Precision"]["TotalMult"] = 20;
	VARS["Precision"]["TotalPow"] = 2;
		
	VARS["Memory"]["FollowpointsNerf"] = 0.8;
	VARS["Memory"]["SliderBuff"] = 1.1;
	VARS["Memory"]["TotalMult"] = 115;
	VARS["Memory"]["TotalPow"] = 0.3;	
	VARS["Memory"]["DistanceHiddenBuff"] = 1.15;
	VARS["Memory"]["MaxHiddenApproachRateBuff"] = 0.2;
	VARS["Memory"]["MediumCircleSizeRebalance"] = 36.49;
	VARS["Memory"]["CircleSizeRebalanceFactor"] = 0.005;

	VARS["Reaction"]["AvgWeighting"] = 0.7;
	VARS["Reaction"]["PatternDamping"] = 0.15;
	VARS["Reaction"]["FadeinPercent"] = 0.1;
	VARS["Reaction"]["VerScale"] = 12.2;
	VARS["Reaction"]["CurveExp"] = 0.64;
}

// load tweakable variables used in formulas for strain calculation
void LoadFormulaVars()
{
	varsLoaded = true;
	// loading default variables
	ResetFormulaVars();	
	// If file exists
	if (std::ifstream(config).good())
	{
		INIReader reader(config);
		if (reader.ParseError() < 0) {
			//std::cout << "Can't load 'test.ini'\n";
			return;
		}
		// loop thru keys
		for(auto skill : VARS)
		{
			std::string skillName = skill.first;
			for(auto var : skill.second)
			{
				std::string varName = var.first;
				if (VARS[skillName].find(varName) == VARS[skillName].end())
					cout << "Algorithm variable loader: Wrong variable name - " << varName << endl;
				else
					VARS[skillName][varName] = reader.GetReal(skillName, varName, VARS[skillName][varName]);
			}
		}
	}
	SaveFormulaVars();
}

double GetVar(std::string skill, std::string name)
{
	/*if (VARS.find(skill) == VARS.end())
	{
		cout << "Algorithm variable loader: Wrong skill - " << skill << endl;
		return 0;
	}
	else
	{
		if (VARS[skill].find(name) == VARS[skill].end())
		{
			cout << "Algorithm variable loader: Wrong variable name - " << name << endl;
			return 0;
		}
		else
		{*/
			return VARS[skill][name];
	//	}
	//}	
}

bool FormulaVarsLoaded()
{
	return varsLoaded;
}