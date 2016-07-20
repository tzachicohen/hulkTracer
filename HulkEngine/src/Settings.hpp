//////////////////////////////////////////////////////////////////
// Settings.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#ifndef HULK_SETTING
#define HULK_SETTING

#include <string>
#include <map>
#include <stdio.h>

class Settings
{
public:
	static Settings& getInstance() {
		static Settings settings;
		return settings;
	}
	const wchar_t * getValue(const std::wstring & );
	bool parseStrings(int argc, wchar_t * argv[]);
	void addDefaults();
	void printHelp();
private:
	//this enum holds the default values
	std::map<std::wstring,std::wstring> m_defaults;
	std::map<std::wstring,std::wstring> m_values;
	Settings() { addDefaults();} 
};

#endif