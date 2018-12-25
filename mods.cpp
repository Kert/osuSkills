#include "mods.h"
#include "globals.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <algorithm>

using std::cout;
using std::endl;

bool HasMod(Beatmap &beatmap, MODS mod)
{
	return (mod & beatmap.mods) > 0;
}

bool ApplyMods(Beatmap &beatmap, int mods)
{
	beatmap.mods = mods;
	// Don't allow exclusive mods
	if ((HasMod(beatmap, EZ) && HasMod(beatmap, HR)) || (HasMod(beatmap, HT) && HasMod(beatmap, DT)))
	{
		cout << beatmap.name << " You can't select both DT and HT or both EZ and HR at once!" << endl;
		beatmap.mods = 0;
		return 0;
	}
		
	if (HasMod(beatmap, EZ)) // Note: EZ needs to be applied BEFORE DT or HT
	{
		beatmap.ar *= 0.5;
		beatmap.od *= 0.5;
		beatmap.cs *= 0.5;
	}

	if (HasMod(beatmap, HR)) // Note: HR needs to be applied BEFORE DT or HT
	{
		if (beatmap.ar*1.4 < 10) beatmap.ar *= 1.4;
		else					 beatmap.ar = 10;

		if (beatmap.od*1.4 < 10) beatmap.od *= 1.4;
		else					 beatmap.od = 10;

		beatmap.cs = std::min(10.0, beatmap.cs * 1.3);
	}

	if (HasMod(beatmap, HT)) // Note: HT needs to be applied AFTER HR or EZ
	{
		for (auto &obj : beatmap.hitObjects)
		{
			obj.time = static_cast<int>(ceil(static_cast<double>(obj.time) / 0.75));
		}
		for (auto &timings : beatmap.timingPoints)
		{
			if(timings.beatInterval > 0)
				timings.beatInterval /= 0.75;
			timings.offset = static_cast<int>(ceil(static_cast<double>(timings.offset) / 0.75));
		}

		beatmap.ar = ms2AR(static_cast<int>(static_cast<double>(AR2ms(beatmap.ar)) / 0.75));
		//beatmap.od = beatmap.od * 0.75;
	}

	if (HasMod(beatmap, DT))   // Note: DT needs to be applied AFTER HR or EZ
	{
		for (auto &obj : beatmap.hitObjects)
		{
			obj.time = static_cast<int>(ceil(static_cast<double>(obj.time) / 1.5));
		}
		for (auto &timings : beatmap.timingPoints)
		{
			if (timings.beatInterval > 0)
				timings.beatInterval /= 1.5;
			timings.offset = static_cast<int>(ceil((double)timings.offset / 1.5));
		}
		beatmap.ar = std::min(ms2AR((int)((double)AR2ms(beatmap.ar) / 1.5)), 11.0);
		//beatmap.od = beatmap.od * 1.5;
	}
	
	if (HasMod(beatmap, HD))
	{

	}

	if (HasMod(beatmap, FL))
	{

	}

	return 1;
}

