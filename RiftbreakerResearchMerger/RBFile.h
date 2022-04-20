#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "RBNode.h"



class RBFile
{
public:
	RBFile(std::istream& in) { m_root = std::make_shared<RBNodeList>(std::string("ROOT"));  Parse(in); }
	RBFile(std::shared_ptr<RBNodeList> root);
	std::shared_ptr<RBFile> Copy();
	void Merge(std::shared_ptr<RBFile> other, std::shared_ptr<RBMergeRules> rules);
	void Serialize(std::ostream& out);
	void RemoveEqual(std::shared_ptr<RBFile> other, std::shared_ptr<RBMergeRules> rules);
private:
	void Parse(std::istream& in);
	std::shared_ptr<RBNodeList> m_root;

};
