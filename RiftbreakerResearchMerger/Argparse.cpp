#include "Argparse.h"
#include <sstream>

Argparse::Argparse(const int argc, const char** argv)
{
	SetDefaults();
	std::string arg;
	for (int i = 1; i < argc; ++i) {
		arg = std::string(argv[i]);
		if (arg.compare("-packpath")==0) {
			if (i == argc - 1) {
				throw std::runtime_error("-packpath requires a value.");
			}
			m_args["packpath"] = std::string(argv[++i]);
		}
		else if (arg.compare("-rtpath") == 0) {
			if (i == argc - 1) {
				throw std::runtime_error("-rtpath requires a value.");
			}
			m_args["rtpath"] = std::string(argv[++i]);
		}
		else if (arg.compare("-outpath") == 0) {
			if (i == argc - 1) {
				throw std::runtime_error("-outpath requires a value.");
			}
			m_args["outname"] = std::string(argv[++i]);
		}
		else if (arg.compare("-position") == 0) {
			m_args["position"] = std::string("true");
		}
		else {
			std::stringstream ss;
			ss << "Unknown argument " << arg;
			throw std::runtime_error(ss.str());
		}
	}
}

bool Argparse::HasArg(std::string name)
{
	return m_args.count(name) > 0;
}

std::string Argparse::GetString(std::string name)
{
	return m_args[name];
}

bool Argparse::GetBool(std::string name)
{
	std::string arg = m_args[name];
	if (arg.compare("true") == 0) {
		return true;
	}
	else if (arg.compare("false")==0) {
		return false;
	}
	else {
		std::stringstream ss;
		ss << name << ": " << arg << " is not a valid bool (true|false) argument.";
		throw std::runtime_error(ss.str());
	}
}

void Argparse::SetDefaults()
{
	m_args[std::string("packpath")] = std::string("packs");
	m_args[std::string("rtpath")] = std::string("scripts/research");
	m_args[std::string("outname")] = std::string("zzz_ResearchMerge.zip");
	m_args[std::string("position")] = std::string("false");
}
