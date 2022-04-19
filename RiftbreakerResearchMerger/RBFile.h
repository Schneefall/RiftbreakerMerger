#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "RBNode.h"



class RBFile
{
public:
	RBFile(std::istream& in) { m_root = std::make_shared<RBNodeList>(std::string("ROOT"));  Parse(in); }
	template<typename T>
	std::shared_ptr<T> AsType(std::shared_ptr<RBNode> node);
	template<> std::shared_ptr<RBNodeEmpty> AsType(std::shared_ptr<RBNode> node);
	template<> std::shared_ptr<RBNodeList> AsType(std::shared_ptr<RBNode> node);
	template<> std::shared_ptr<RBNodeValue> AsType(std::shared_ptr<RBNode> node);
	void Merge(std::shared_ptr<RBFile> other, std::shared_ptr<RBMergeRules> rules);
	void Serialize(std::ostream& out);
private:
	void Parse(std::istream& in);
	std::shared_ptr<RBNodeList> m_root;

};
