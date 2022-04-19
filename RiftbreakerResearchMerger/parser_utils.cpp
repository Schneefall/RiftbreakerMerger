#include "parser_utils.h"
#include <stdexcept>

bool checkToken(std::istringstream& instream, std::string expected) {
	std::string token;
	instream >> token;
	bool OK = token.compare(expected) == 0;
	if (!OK) {
		std::stringstream ss;
		ss << "Unexpected token '" << token << "' at position " << instream.tellg() << ", expected '" << expected << "'.";
		throw std::runtime_error(ss.str());
	}
	return OK;
}

void skipBlock(std::istringstream& instream) {
	int braceCount = 0;
	size_t pos = instream.tellg();

	std::string token;
	instream >> token;
	if (token.compare("{") == 0) {
		braceCount++;
		while (!instream.eof()) {
			instream >> token;
			if (token.compare("{") == 0) {
				braceCount++;
			}
			else if (token.compare("}") == 0) {
				braceCount--;
			}
			
			if (braceCount <= 0) {
				break;
			}
		}
	}
	else {
		instream.seekg(pos);
	}
}



std::vector<std::string> tokenize(std::string& line)
{
	std::vector<std::string> v;
	std::string::iterator pos = line.begin();
	while (pos != line.end()) {

		std::string::iterator start = std::find_if(pos, line.end(), [](unsigned char c) {return !std::isspace(c); });
		std::string::iterator end;
		if (*start == '"') {
			// there can be spaces in literals
			end = std::find_if(start + 1, line.end(), [](unsigned char c) {return c=='"'; }) + 1;
		}
		else {
			end = std::find_if(start + 1, line.end(), [](unsigned char c) {return std::isspace(c); });
		}

		v.push_back(line.substr(start - line.begin(), end - line.begin()));

		pos = end;
	}
	return v;

	std::istringstream iss(line);
	while (!iss.eof()) {
		std::string token;
		iss >> token;
		v.push_back(token);
	}
	return v;
}

bool isValue(const std::string& token)
{
	return token[0] == '"' && token[token.length() - 1] == '"';
}

bool isBlockOpen(const std::string& token)
{
	return token.length() == 1 && token[0] == '{';
}

bool isBlockClose(const std::string& token)
{
	return token.length() == 1 && token[0] == '}';
}


void addIndent(std::ostream& outstream, int indent)
{
	for (int i = 0; i < indent; ++i) outstream << "\t";
}

double parseDouble(std::istringstream& instream)
{
	std::string token;
	instream >> token;
	token.erase(0, 1);
	token.erase(token.end() - 1);
	return std::stod(token);
}

std::string formatDouble(const double value)
{
	std::stringstream ss;
	ss.setf(std::ios::fixed);
	ss.precision(3);
	ss << '"' << value << '"';
	return ss.str();
}
