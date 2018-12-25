# osuSkills
http://osuskills.com

**osu!Skills** is an unofficial system of rating player results based on actual skills used when playing. It's intended to show strengths and weaknesses of players by using separate Skill Points.

**osu!Skills** is made by Kert, abraker, FullTablet, RuSt

# Tools and dependencies

Project is being developed under Visual Studio 2015 but .dll for [osuSkillsGUI](https://github.com/Kert/osuSkillsGUI) app must be complied using MinGW 7.3.0 x64 compiler for C/C++

```g++ -std=c++11 -shared *.cpp -o osuSkills.dll```

It uses external code: vector2d.h (irrlicht), INIReader.h, slider code from Tom94

# Branches
Main branch builds a .dll. Basically it's used to see if the code compiles correctly before compiling it for website itself or [osuSkillsGUI](https://github.com/Kert/osuSkillsGUI)
For easier debugging you can use console_app branch (which builds an .exe file) where you can calculate skills for a single map

# Contributing
Anyone is free to improve current calculations for a better system and ranking
