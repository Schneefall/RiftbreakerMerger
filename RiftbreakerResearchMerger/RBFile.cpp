#include "RBFile.h"
#include <algorithm>
#include "parser_utils.h"

template<typename T>
int index(std::vector<T> & v , T & e ) {
	auto it = std::find(v.begin(), v.end(), e);
	if (it != v.end()) {
		return std::distance(v.begin(), it);
	}
	return -1;
}

template<>
inline std::shared_ptr<RBNodeEmpty> RBFile::AsType(std::shared_ptr<RBNode> node)
{
	if (node->GetType() == RBNodeType::RBNODE_EMPTY) {
		return std::static_pointer_cast<RBNodeEmpty>(node);
	}
	else {
		throw std::runtime_error("Can't convert to empty node.");
	}
}
template<>
inline std::shared_ptr<RBNodeList> RBFile::AsType(std::shared_ptr<RBNode> node)
{
	if (node->GetType() == RBNodeType::RBNODE_LIST) {
		return std::static_pointer_cast<RBNodeList>(node);
	}
	else {
		throw std::runtime_error("Can't convert to list node.");
	}
}
template<>
inline std::shared_ptr<RBNodeValue> RBFile::AsType(std::shared_ptr<RBNode> node)
{
	if (node->GetType() == RBNodeType::RBNODE_VALUE) {
		return std::static_pointer_cast<RBNodeValue>(node);
	}
	else {
		throw std::runtime_error("Can't convert to value node.");
	}
}

void RBFile::Merge(std::shared_ptr<RBFile> other, std::shared_ptr<RBMergeRules> rules)
{
	m_root->Merge(other->m_root, rules);
}

void RBFile::Serialize(std::ostream& out)
{
	for (const auto& node : m_root->GetNodes()) {
		node->Serialize(out, 0);
	}
}

void RBFile::Parse(std::istream& in)
{
	std::vector<std::shared_ptr<RBNodeList>> stack;
	std::shared_ptr<RBNodeList> activeNode = m_root;
	std::string line;
	std::string delim("//");
	size_t lineNumber = 0;
	std::string lastName("");

	while (!in.eof()) {
		 std::getline(in, line);
		 ++lineNumber;
		 removeComment(line, delim);
		 trim(line);

		 if (line.empty()) continue;

		 auto tokens = tokenize(line);
		 for (const auto& token : tokens) {
			 if (isValue(token)) {
				 if (lastName.empty()) {
					 std::stringstream ss;
					 ss << "Value " << token << " in line " << lineNumber << " has no name.";
					 throw std::runtime_error(ss.str());
				 }
				 activeNode->AddNode(std::make_shared<RBNodeValue>(lastName, token));
				 lastName.clear();
			 }
			 else if (isBlockOpen(token)) {
				 if (lastName.empty()) {
					 std::stringstream ss;
					 ss << "Block in line " << lineNumber << " has no name.";
					 throw std::runtime_error(ss.str());
				 }
				 auto node = std::make_shared<RBNodeList>(lastName);
				 activeNode->AddNode(node);
				 stack.push_back(activeNode);
				 activeNode = node;
				 lastName.clear();
			 }
			 else if (isBlockClose(token)) {
				 if (!lastName.empty()) {
					 activeNode->AddNode(std::make_shared<RBNodeEmpty>(lastName));
					 lastName.clear();
				 }
				 if (stack.size() == 0) {
					 std::stringstream ss;
					 ss << "Unexpected '" << token << "' in line " << lineNumber << ".";
					 throw std::runtime_error(ss.str());
				 }

				 activeNode = stack.back();
				 stack.pop_back();
			 }
			 else {
				 if (!lastName.empty()) {
					 activeNode->AddNode(std::make_shared<RBNodeEmpty>(lastName));
					 lastName.clear();
				 }
				 lastName = token;
			 }
		 }
	}
	if (!lastName.empty()) {
		activeNode->AddNode(std::make_shared<RBNodeEmpty>(lastName));
		lastName.clear();
	}
	if (stack.size() > 0) {
		std::stringstream ss;
		ss << "Unexpected EOF, not all blocks are closed.";
		throw std::runtime_error(ss.str());
	}
}
