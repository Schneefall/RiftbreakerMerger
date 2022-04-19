#pragma once
#include <map>
#include <string>
class Argparse
{
public:
	Argparse(const int argc, const char** argv);
	bool HasArg(std::string name);
	std::string GetString(std::string name);
	int GetInt(std::string name);
	float GetFloat(std::string name);
	bool GetBool(std::string name);
private:
	void SetDefaults();
	std::map<std::string, std::string> m_args;
};

