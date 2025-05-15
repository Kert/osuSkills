#ifndef _filereader_h_
#define _filereader_h_

#include "globals.h"
#include "generic.h"
#include "utils.h"

#include <cstring>

using std::cout;
using std::endl;

namespace FileReader
{
	template < class ContainerT >
	void tokenize(const std::string& str, ContainerT& tokens,
		          const std::string& delimiters = " ", bool trimEmpty = false)
	{
		std::string::size_type pos, lastPos = 0;

		using value_type = typename ContainerT::value_type;
		using size_type = typename ContainerT::size_type;

		while (true)
		{
			pos = str.find_first_of(delimiters, lastPos);
			if (pos == std::string::npos)
			{
				pos = str.length();

				if (pos != lastPos || !trimEmpty)
					tokens.push_back(value_type(str.data() + lastPos,
									 (size_type)pos - lastPos));

				break;
			}
			else
			{
				if (pos != lastPos || !trimEmpty)
					tokens.push_back(value_type(str.data() + lastPos,
									 (size_type)pos - lastPos));
			}

			lastPos = pos + 1;
		}
	}

	bool ParseBeatmap(std::ifstream &filepath, Beatmap &beatmap)
	{
		enum FOUND
		{
			FOUND_NONE,
			FOUND_METADATA,
			FOUND_DIFFICULTY,
			FOUND_TIMINGPOINTS,
			FOUND_HITOBJECTS
		};

		std::string line = "";
		Beatmap bData;
		FOUND found = FOUND_NONE;

		while (getline(filepath, line))
		{
			if (!line.length())
				continue;
			if(line.back() == '\r')
				line.pop_back();
			if (!line.length())
				continue;

			if (!line.compare(0, 10, "[Metadata]"))				found = FOUND_METADATA;
			else if (!line.compare(0, 12, "[Difficulty]"))		found = FOUND_DIFFICULTY;
			else if (!line.compare(0, 14, "[TimingPoints]"))	found = FOUND_TIMINGPOINTS;
			else if (!line.compare(0, 12, "[HitObjects]")) 		found = FOUND_HITOBJECTS;
			else if (found == FOUND_METADATA)
			{
				std::vector<std::string> tokens;
				tokenize(line, tokens, ":");
				if (!tokens[0].compare(0, tokens[0].length(), "Artist"))
				{
					if (tokens.size() > 1)
						bData.artist = tokens[1];
				}

				if (!tokens[0].compare(0, tokens[0].length(), "Title"))
				{
					if (tokens.size() > 1)
						bData.title = tokens[1];
				}

				if (!tokens[0].compare(0, tokens[0].length(), "Version"))
				{
					if (tokens.size() > 1)
						bData.version = tokens[1];
				}

				if (!tokens[0].compare(0, tokens[0].length(), "Creator"))
				{
					if (tokens.size() > 1)
						bData.creator = tokens[1];
				}
			}
			else if (found == FOUND_DIFFICULTY)
			{
				std::vector<std::string> tokens;
				tokenize(line, tokens, ":");
				if (!tokens[0].compare(0, tokens[0].length(), "HPDrainRate"))
				{
					bData.hp = atoi(tokens[1].c_str());
				}

				if (!tokens[0].compare(0, tokens[0].length(), "CircleSize"))
				{
					bData.cs = atoi(tokens[1].c_str());
				}

				if (!tokens[0].compare(0, tokens[0].length(), "OverallDifficulty"))
				{
					bData.od = atoi(tokens[1].c_str());
				}

				if (!tokens[0].compare(0, tokens[0].length(), "ApproachRate"))
				{
					bData.ar = atof(tokens[1].c_str());
				}

				if (!tokens[0].compare(0, tokens[0].length(), "SliderMultiplier"))
				{
					bData.sm = atof(tokens[1].c_str());
				}

				if (!tokens[0].compare(0, tokens[0].length(), "SliderTickRate"))
				{
					bData.st = atof(tokens[1].c_str());
				}
			}
			else if (found == FOUND_TIMINGPOINTS)
			{
				TimingPoint tPoint;
				std::vector<std::string> tokens;
				tokenize(line, tokens, ",");
				
				if (tokens.size() < 2)
				{
					found = FOUND_NONE;
					continue;
				}
				
				tPoint.offset = atoi(tokens[0].c_str());
				tPoint.beatInterval = atof(tokens[1].c_str());
				
				if (tokens.size() > 6)		tPoint.inherited = !atoi(tokens[6].c_str());
				else						tPoint.inherited = 0;

				// Old maps don't have metres
				if (tokens.size() >= 3)		tPoint.meter = atoi(tokens[2].c_str());
				else						tPoint.meter = 4;
				
				if (tPoint.offset == -1 || tPoint.meter == -1)
				{
					cout << beatmap.name << " Wrong timing point data!";
					return 0;
				}
				
				bData.timingPoints.push_back(tPoint);
			}
			else if (found == FOUND_HITOBJECTS)
			{
				HitObject hitObject;
				hitObject.pixelLength = 0;
				hitObject.repeat = 1;
				hitObject.ncurve = 0;
				hitObject.toRepeatTime = 0;

				std::vector<std::string> tokens;
				tokenize(line, tokens, ",");
				if(tokens.size() < 4)
				{
					cout << beatmap.name << " HitObjects line: not enough tokens!";
					continue;
				}
				hitObject.pos.X = atof(tokens[0].c_str());
				hitObject.pos.Y = atof(tokens[1].c_str());
				hitObject.time  = atoi(tokens[2].c_str());
				hitObject.type  = atoi(tokens[3].c_str());
				hitObject.endTime = hitObject.time;

				// TODO: color hacks types
				if (IsHitObjectType(hitObject.type, HitObjectType::Normal)) // circle
				{
					hitObject.endPoint = hitObject.pos;
					bData.hitObjects.push_back(hitObject);
					bData.timeMapper[hitObject.time] = bData.hitObjects.size() - 1;
				}
				else if (IsHitObjectType(hitObject.type, HitObjectType::SLIDER))
				{
					std::vector<std::string> sliderTokens;
					tokenize(tokens[5], sliderTokens, "|");
					hitObject.curveType = sliderTokens[0].c_str()[0];
					
					for (unsigned i = 1; i < sliderTokens.size(); i++)
					{
						std::vector<std::string> curveTokens;
						tokenize(sliderTokens[i], curveTokens, ":");
						
						Vector2d curve;
							curve.X = atoi(curveTokens[0].c_str());
							curve.Y = atoi(curveTokens[1].c_str());
						
						hitObject.curves.push_back(curve);
					}

					hitObject.repeat = atoi(tokens[6].c_str());
					hitObject.pixelLength = atof(tokens[7].c_str());
					bData.hitObjects.push_back(hitObject);

					bData.timeMapper[hitObject.time] = bData.hitObjects.size() - 1;
				}

				else if(IsHitObjectType(hitObject.type, HitObjectType::Spinner))
					bData.spinners++;
				else if (IsHitObjectType(hitObject.type, HitObjectType::Hold))
					continue;
				else // what is this object?
				{
					continue;
				}
			}
			else if (!line.compare(0, 5, "Mode:"))
			{
				int mode = atoi(line.substr(5, line.length()).c_str());
				if (mode > 0)
					return 0;
			}
			else if (!line.compare(0, 17, "osu file format v"))
			{
				int format = atoi(line.substr(17, line.length()).c_str());
				bData.format = format;
			}
		}
		if (bData.format == -1)
			cout << beatmap.name << "Warning: Wrong osu file format version!" << endl;
		if (bData.hp == -1 || bData.cs == -1 || bData.od == -1 || bData.sm == -1 || bData.st == -1)
		{
			cout << beatmap.name << " Wrong file format!" << endl;
			return 0;
		}
		if (bData.ar == -1)
			bData.ar = bData.od;

		beatmap = bData;
		beatmap.name = beatmap.artist + " - " + beatmap.title + " (" + beatmap.creator + ") [" + beatmap.version + "]";
		return 1;
	}

	bool GetMapListFromFile(std::string filepath, std::vector<std::string> &mapList)
	{
		std::ifstream listFile(filepath);
		if (listFile.is_open())
		{
			cout << "Opened map list file " << filepath << endl;

			std::string line = "";
			while (getline(listFile, line))
			{
				if (!line.length())   continue;
				if (!line.find("//")) continue; // ignore commented maps
					
				mapList.push_back(line);
			}

			listFile.close();
			return 1;
		}
		else
		{
			cout << "Can't open " << filepath << endl;
			return 0;
		}
	}
}
#endif
