#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "RBMergeRules.h"

enum class RBNodeType {
	RBNODE_EMPTY = 0,
	RBNODE_VALUE = 1,
	RBNODE_LIST = 2,
};

class RBNodeValue;
class RBNodeList;
class RBNodeEmpty;

class RBNode
{
public:
	virtual RBNodeType GetType() const = 0;
	virtual std::string GetName() const = 0;
	virtual void Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) = 0;
	virtual void Serialize(std::ostream &out, int indent) = 0;
};

class RBNodeValue : public RBNode
{
public:
	RBNodeValue(std::string name, std::string value) : m_name(name), m_value(value) {}
	RBNodeType GetType() const override { return RBNodeType::RBNODE_VALUE; }
	std::string GetName() const override { return m_name; }
	std::string GetValue() { return m_value; }
	void Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override;
	void Serialize(std::ostream& out, int indent) override;
private:
	std::string m_name;
	std::string m_value;
};

class RBNodeList : public RBNode
{
public:
	RBNodeList(std::string name) : m_name(name) { }
	RBNodeList(std::string name, std::vector<std::shared_ptr<RBNode>> nodes) : m_name(name), m_nodes(nodes) { }
	RBNodeType GetType() const override { return RBNodeType::RBNODE_LIST; }
	std::string GetName() const override { return m_name; }
	std::vector<std::shared_ptr<RBNode>> GetNodes() { return m_nodes; }
	void AddNode(std::shared_ptr<RBNode> node) { m_nodes.push_back(node); }
	void SetNode(std::shared_ptr<RBNode> node, size_t index) { m_nodes[index] = node; }
	void RemoveNode(std::shared_ptr<RBNode> node);
	size_t Size() { return m_nodes.size(); }
	bool Empty() { return m_nodes.size()==0; }
	bool Contains(std::shared_ptr<RBNode> node);
	bool Contains(std::string name);
	std::shared_ptr<RBNode> GetNode(std::string name);
	void Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override;
	bool IsDict();
	std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> AsDictMap();
	bool IsList();
	std::string ListName();
	std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> AsListMap(std::string &keyName);
	void Serialize(std::ostream& out, int indent) override;
private:

	std::string m_name;
	std::vector<std::shared_ptr<RBNode>> m_nodes;
};

class RBNodeEmpty : public RBNode
{
public:
	RBNodeEmpty(std::string name) : m_name(name) { }
	RBNodeType GetType() const override { return RBNodeType::RBNODE_EMPTY; };
	std::string GetName() const override { return m_name; }
	void Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override;
	void Serialize(std::ostream& out, int indent) override;
private:
	std::string m_name;

};

