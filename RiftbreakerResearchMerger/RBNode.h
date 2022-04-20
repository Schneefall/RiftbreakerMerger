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
	virtual std::shared_ptr<RBNode> Copy() const = 0;
	virtual RBNodeType GetType() const = 0;
	virtual std::string GetName() const = 0;
	virtual void Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) = 0;
	virtual void Serialize(std::ostream &out, int indent) const = 0;
	virtual bool Compare(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) const = 0;
	//virtual void SetModified(const bool modified) = 0;
	//virtual bool IsModified() const = 0;
	virtual void RemoveEqual(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) = 0;
};

class RBNodeValue : public RBNode
{
public:
	RBNodeValue(std::string name, std::string value) : m_name(name), m_modified(false), m_value(value) {}
	std::shared_ptr<RBNode> Copy() const override;
	RBNodeType GetType() const override { return RBNodeType::RBNODE_VALUE; }
	std::string GetName() const override { return m_name; }
	std::string GetValue() const { return m_value; }
	void Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override;
	void Serialize(std::ostream& out, int indent) const override;
	bool Compare(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) const override;
	//void SetModified(const bool modified) override { m_modified = modified; };
	//bool IsModified() const override { return m_modified; }
	void RemoveEqual(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override { throw std::runtime_error("Can't remove from value node"); }
private:
	std::string m_name;
	std::string m_value;
	bool m_modified;
};

class RBNodeList : public RBNode
{
public:
	RBNodeList(std::string name) : m_name(name), m_modified(false) { }
	RBNodeList(std::string name, std::vector<std::shared_ptr<RBNode>> nodes) : m_name(name), m_modified(false), m_nodes(nodes) { }
	std::shared_ptr<RBNode> Copy() const override;
	RBNodeType GetType() const override { return RBNodeType::RBNODE_LIST; }
	std::string GetName() const override { return m_name; }
	std::vector<std::shared_ptr<RBNode>> GetNodes() { return m_nodes; }
	void AddNode(std::shared_ptr<RBNode> node) { m_nodes.push_back(node); }
	void SetNode(std::shared_ptr<RBNode> node, size_t index) { m_nodes[index] = node; }
	void RemoveNode(std::shared_ptr<RBNode> node);
	size_t Size() const { return m_nodes.size(); }
	bool Empty() const { return m_nodes.size()==0; }
	bool Contains(std::shared_ptr<RBNode> node) const;
	bool Contains(std::string name) const;
	std::shared_ptr<RBNode> GetNode(std::string name);
	void Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override;
	bool IsDict() const;
	std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> AsDictMap();
	bool IsList() const;
	std::string ListName() const;
	std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> AsListMap(std::string &keyName);
	void Serialize(std::ostream& out, int indent) const override;
	bool Compare(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) const override;
	//void SetModified(const bool modified) override;
	//bool IsModified() const override { return m_modified; }
	void RemoveEqual(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override;
private:

	std::string m_name;
	std::vector<std::shared_ptr<RBNode>> m_nodes;
	bool m_modified;
};

class RBNodeEmpty : public RBNode
{
public:
	RBNodeEmpty(std::string name) : m_name(name), m_modified(false) { }
	RBNodeType GetType() const override { return RBNodeType::RBNODE_EMPTY; };
	std::string GetName() const override { return m_name; }
	std::shared_ptr<RBNode> Copy() const override;
	void Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override;
	void Serialize(std::ostream& out, int indent) const override;
	bool Compare(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) const override;
	//void SetModified(const bool modified) override { m_modified = modified; };
	//bool IsModified() const override { return m_modified; }
	void RemoveEqual(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) override { throw std::runtime_error("Can't remove from value node"); }
private:
	std::string m_name;
	bool m_modified;

};

