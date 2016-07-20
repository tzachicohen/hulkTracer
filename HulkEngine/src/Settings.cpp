//////////////////////////////////////////////////////////////////
// Settings.cpp													//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////


#include "Settings.hpp"

bool Settings::parseStrings(int argc, wchar_t * argv[])
{
	for (int i =2 ; i < argc ; i++) {
		std::wstring option = argv[i];
		size_t index = option.find(L"=");
		if (index == -1 ) {
            wprintf(L"Illegal syntax in attribute.- %s\n", option.c_str());
			return false;
		}
		if (m_defaults.end() == m_defaults.find(option.substr(0,index))) {
            wprintf(L"Unknown attribute- %s.\n", option.substr(0, index).c_str());
			return false;
		}

		m_values[option.substr(0,index)] = option.substr(index+1,option.length());
	}
	return true;
}

const wchar_t * Settings::getValue(const std::wstring & value)
{
	if (m_values.end() != m_values.find(value)) {
		return m_values[value].c_str();
	}

	if (m_defaults.end() != m_defaults.find(value)) {
		return m_defaults[value].c_str();
	}
	return NULL;
}

void Settings::addDefaults()
{
	m_defaults[L"--renderType"] = L"GLT";
	m_defaults[L"--fixedTic"] = L"0";
	m_defaults[L"--singleFrame"] = L"0";
	m_defaults[L"--saveImages"] = L"0";
	m_defaults[L"--dumpStats"] = L"0";
    m_defaults[L"--treeType"] = L"SAHNlogN";
	m_defaults[L"--kernelType"] = L"GPUTrace_stackless";
    m_defaults[L"--saveBin"] = L"0";
    m_defaults[L"--p2p"] = L"0";
}

void Settings::printHelp()
{
	printf("  Usage: hulkEngine.exe <scene file name> [optional attributes]\n");
	printf("  Hulk engine currently support AFF and OBJ file formats.\n");
	printf("  Supported attributes:\n");
	for (auto it = m_defaults.begin(); it != m_defaults.end(); ++it)
	{
		wprintf (L"  %s , defalut value is %s\n", it->first.c_str(), it->second.c_str());
	}
	printf ("  Create by Tzachi Cohen, all rights reserved.(tzachi_cohen@hotmail.com)\n");
}