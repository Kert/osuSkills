#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Slider.h"
#include "filereader.h"
#include "generic.h"
#include "tenacity.h"
#include "stamina.h"
#include "agility.h"
#include "reading.h"
#include "mods.h"
#include "patterns.h"
#include "precision.h"
#include "accuracy.h"
#include "memory.h"
#include "reaction.h"
#include "strains.h"
#include "tweakvars.h"

namespace osuSkills
{
	bool PreprocessMap(Beatmap &beatmap)
	{
		if (beatmap.hitObjects.size() < 2)
		{
			cout << endl << beatmap.name << "The map has less than 2 hit objects!" << endl << endl;
			return 0;
		}
		PrepareTimingPoints(beatmap);
		ApproximateSliderPoints(beatmap);
		BakeSliderData(beatmap);

		PrepareAimData(beatmap);
		PrepareTapData(beatmap);
		if (beatmap.distances.size() == 0)
			return 0;
		return 1;
	}

	void CalculateSkills(Beatmap &beatmap)
	{
		CalculateReaction(beatmap, HasMod(beatmap, HD));
		CalculateStamina(beatmap);
		CalculateTenacity(beatmap);
		bool agilityV2 = false;
		if (agilityV2)
			CalculateAgilityStrains(beatmap);  // calculates precision as well. Might seperate that later
		else
			CalculateAgility(beatmap);
		CalculatePrecision(beatmap, HasMod(beatmap, HD));
		CalculateAccuracy(beatmap);
		if (HasMod(beatmap, FL))
		{
			CalculateMemory(beatmap);
		}
		CalculateReading(beatmap, HasMod(beatmap, HD));
	}

	int ProcessFile(std::string filepath, int mods, Beatmap& beatmap)
	{
		std::ifstream beatmapFile(filepath);
		if (beatmapFile.is_open())
		{
			if (!FileReader::ParseBeatmap(beatmapFile, beatmap))
			{
				cout << "\r*** Parsing... failed! " << filepath << endl << endl;
				return 0;
			}
			beatmapFile.close();

			if (mods != 0)
				ApplyMods(beatmap, mods);

			return PreprocessMap(beatmap);
		}
		else
		{
			cout << "Can't open " << filepath << endl;
			return 0;
		}
	}

	extern "C" __declspec(dllexport) int ReloadFormulaVars()
	{
		LoadFormulaVars();
		return 1;
	}
	
	extern "C" __declspec(dllexport) int CalculateBeatmapSkills(std::string filepath, int &circles, int &sliderspinners, int mods, Skills& skills, std::string &name, double &ar, double &cs)
	{
		// redirect cout to file
		std::fstream output("log.txt", std::fstream::out | std::fstream::app);
		std::streambuf *coutbuf = std::cout.rdbuf();
		cout.rdbuf(output.rdbuf());

		if(!FormulaVarsLoaded())
			LoadFormulaVars();
		Beatmap beatmap;

		if (!ProcessFile(filepath, mods, beatmap))
		{
			std::cout.rdbuf(coutbuf); // return to default
			return 0;
		}
		CalculateAimStrains(beatmap);
		CalculateTapStrains(beatmap);
		CalculateSkills(beatmap);
		int circ = 0;
		int slispi = beatmap.spinners;
		for (auto &obj : beatmap.hitObjects)
		{
			if (IsHitObjectType(obj.type, HitObjectType::Normal))
				circ++;
			else if (IsHitObjectType(obj.type, HitObjectType::SLIDER))
				slispi++;
		}
		circles = circ;
		sliderspinners = slispi;
		skills = beatmap.skills;
		name = beatmap.name;
		ar = beatmap.ar;
		cs = beatmap.cs;
		std::cout.rdbuf(coutbuf); // return to default
		return 1;
	}
}