#pragma once
#include <sstream>
#include <string>
#include <vector>

bool checkToken(std::istringstream& instream, std::string expected);
void skipBlock(std::istringstream& instream);

static inline void removeComment(std::string& line, std::string& delim)
{
	size_t start = line.find(delim);
	if (start != std::string::npos) {
		line.erase(line.begin() + start, line.end());
	}

}
static inline void ltrim(std::string& line)
{
	line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char c) {return !std::isspace(c); }));
}
static inline void rtrim(std::string& line)
{
	line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char c) {return !std::isspace(c); }).base(), line.end());
}
static inline void trim(std::string& line)
{
	ltrim(line);
	rtrim(line);
}


std::vector<std::string> tokenize(std::string& line);

bool isValue(const std::string& token);
bool isBlockOpen(const std::string& token);
bool isBlockClose(const std::string& token);

void addIndent(std::ostream& outstream, int indent);
//parse double value where the number is surrounded by ""
double parseDouble(std::istringstream& instream);

std::string formatDouble(const double value);

template<typename T>
inline void writeIndented(std::ostream& outstream, int indent, T data)
{
	addIndent(outstream, indent);
	outstream << data;
}
template<typename T>
inline void writeLnIndented(std::ostream& outstream, int indent, T data) {
	writeIndented(outstream, indent, data);
	outstream << '\n';
}
template<typename T1, typename T2>
inline void writeLnPairIndented(std::ostream& outstream, int indent, T1 data1, T2 data2) {
	writeIndented(outstream, indent, data1);
	outstream << " " << data2 << '\n';
}

inline void writeLnBracketOpen(std::ostream& outstream, int indent) {
	writeLnIndented(outstream, indent, "{");
}
template<typename T>
inline void writeLnBracketOpenNamed(std::ostream& outstream, int indent, T data) {
	writeLnIndented(outstream, indent, data);
	writeLnIndented(outstream, indent, "{");
}
inline void writeLnBracketClose(std::ostream& outstream, int indent) {
	writeLnIndented(outstream, indent, "}");
	outstream << '\n'; // bracket closed has 2 newline afterwards
}
