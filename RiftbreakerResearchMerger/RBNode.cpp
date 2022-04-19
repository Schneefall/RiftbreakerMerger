#include "RBNode.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include "parser_utils.h"

bool RBNodeList::Contains(std::shared_ptr<RBNode> node)
{
	return std::find(m_nodes.begin(), m_nodes.end(), node) != m_nodes.end();
}

bool RBNodeList::Contains(std::string name)
{
	for (const auto& node : m_nodes) {
		if (name.compare(node->GetName()) == 0) {
			return true;
		}
	}
	return false;
}

std::shared_ptr<RBNode> RBNodeList::GetNode(std::string name)
{
	for (const auto& node : m_nodes) {
		if (name.compare(node->GetName()) == 0) {
			return node;
		}
	}
	std::stringstream ss;
	ss << "Node '" << m_name << "' does not contain '" << name << "'.";
	throw std::runtime_error(ss.str());
}

void RBNodeList::RemoveNode(std::shared_ptr<RBNode> node)
{
	auto it = std::find(m_nodes.begin(), m_nodes.end(), node);
	m_nodes.erase(it);
}

bool RBNodeList::IsDict()
{
	if (Empty()) {
		return true;
	}
	std::string name = m_nodes[0]->GetName();
	for (int i = 1; i < m_nodes.size(); ++i) {
		if (name.compare(m_nodes[i]->GetName()) == 0) {
			return false;
		}
	}
	return true;
}

std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> RBNodeList::AsDictMap()
{
	std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> map;

	std::shared_ptr<RBNode> node;
	for (int i = 0; i < m_nodes.size(); ++i) {
		node = m_nodes[i];
		map.emplace(node->GetName(), std::pair<size_t, std::shared_ptr<RBNode>>(i, node));
	}
	return map;
}

bool RBNodeList::IsList()
{
	if (Empty()) {
		return true;
	}
	std::string name = m_nodes[0]->GetName();
	for (const auto& node : m_nodes) {
		if (node->GetType() == RBNodeType::RBNODE_VALUE) {
			//std::cout << node->GetName() << " is valueNode" << std::endl;
			return false;
		}
		if (name.compare(node->GetName()) != 0) {
			//std::cout << node->GetName() << " does not match name" << name << std::endl;
			return false;
		}
	}
	return true;
}

std::string RBNodeList::ListName()
{
	if (Empty()) {
		return "";
	}
	std::string name = m_nodes[0]->GetName();
	for (const auto& node : m_nodes) {
		if (name.compare(node->GetName()) != 0) {
			return "";
		}
	}
	return name;
}

std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> RBNodeList::AsListMap(std::string& keyName)
{
	std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> map;

	std::shared_ptr<RBNode> node;
	for (int i = 0; i < m_nodes.size(); ++i) {
		node = m_nodes[i];
		if (node->GetType() != RBNodeType::RBNODE_LIST) {
			std::stringstream ss;
			ss << "Node " << i << " '" << node->GetName() << "' of '" << m_name << "' has no keys.'";
			throw std::runtime_error(ss.str());
		}
		std::shared_ptr<RBNodeList> listNode = std::static_pointer_cast<RBNodeList>(node);
		if(!listNode->Contains(keyName)) {
			std::stringstream ss;
			ss << "Node " << i << " '" << node->GetName() << "' of '" << m_name << "' is missing list key '" << keyName << "'.";
			throw std::runtime_error(ss.str());
		}
		std::shared_ptr<RBNode> keyNote = listNode->GetNode(keyName);
		if (keyNote->GetType() != RBNodeType::RBNODE_VALUE) {
			std::stringstream ss;
			ss << "List key '" << keyName << " of node node " << i << " '" << node->GetName() << "' of '" << m_name << "' is not a value node.";
			throw std::runtime_error(ss.str());
		}
		std::shared_ptr<RBNodeValue> valueNode = std::static_pointer_cast<RBNodeValue>(keyNote);
		map.emplace(valueNode->GetValue(), std::pair<size_t, std::shared_ptr<RBNodeList>>(i, listNode));
	}
	return map;
}

void RBNodeList::Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules)
{
	if (other->GetName().compare(m_name) != 0) {
		std::stringstream ss;
		ss << "Attempt to merge '" << other->GetName() << "' with '" << m_name << "'.";
		throw std::runtime_error(ss.str());
	}

	if (other->GetType() != GetType()) {
		throw std::runtime_error("Can't merge nodes with different types.");
	}

	auto otherList = std::static_pointer_cast<RBNodeList>(other);
	std::shared_ptr<RBMergeRule> rule = rules->Get(m_name);

	std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> listMap;
	std::map<std::string, std::pair<size_t, std::shared_ptr<RBNode>>> otherListMap;

	if (rule->mergeType == RBMergeType::RBMERGE_DICT) {
		if (!IsDict()) {
			std::stringstream ss;
			ss << "Base '" << m_name << "' is not a valid dict.";
			//for (const auto& node : m_nodes) ss << node->GetName() << ", ";
			throw std::runtime_error(ss.str());
		}
		if (!otherList->IsDict()) {
			std::stringstream ss;
			ss << "Update '" << m_name << "' is not a valid dict.";
			throw std::runtime_error(ss.str());
		}

		listMap = AsDictMap();
		otherListMap = otherList->AsDictMap();
	}
	else if (rule->mergeType == RBMergeType::RBMERGE_LIST) {
		if (!IsList()) {
			std::stringstream ss;
			ss << "Base '" << m_name << "' is not a valid list.";
			for (const auto& node : m_nodes) ss << node->GetName() << ", ";
			throw std::runtime_error(ss.str());
		}
		if (!otherList->IsList()) {
			std::stringstream ss;
			ss << "Update '" << m_name << "' is not a valid list.";
			throw std::runtime_error(ss.str());
		}

		std::string listName;
		if (!Empty()) {
			listName = ListName();
			if (!otherList->Empty() && listName.compare(otherList->ListName()) != 0) {
				std::stringstream ss;
				ss << "List names of '" << m_name << "' do not match: " << listName << ", " << otherList->ListName() << ".";
				throw std::runtime_error(ss.str());
			}
		}
		else {
			if (otherList->Empty()) {
				return; // both empty
			}
			listName = otherList->ListName();
		}
		if (listName.empty()) {
			std::stringstream ss;
			ss << "'" << m_name << "' is not a list.";
			throw std::runtime_error(ss.str());
		}

		listMap = AsListMap(rule->mergeKey);
		otherListMap = otherList->AsListMap(rule->mergeKey);
	}

	for (const auto& otherEntry : otherListMap) {
		auto otherNode = otherEntry.second.second;
		//const RBNodeType otherType = otherNode->GetType();

		auto baseEntry = listMap.find(otherEntry.first);

		rule = rules->Get(otherEntry.first);

		if (baseEntry != listMap.end()) {
			auto baseNode = baseEntry->second.second;
			const RBNodeType baseType = baseNode->GetType();
			// exists in base list, update/merge
			switch (rule->ruleShared)
			{
			case RBMergeRuleShared::RBMERGE_IGNORE:
				break;
			case RBMergeRuleShared::RBMERGE_REPLACE:
				SetNode(otherNode, baseEntry->second.first);
				break;
			case RBMergeRuleShared::RBMERGE_MERGE:
				if (baseType == RBNodeType::RBNODE_EMPTY) {
					SetNode(otherNode, baseEntry->second.first);
				}
				else {
					baseNode->Merge(otherNode, rules);
				}
				break;
			default:
				break;
			}

		}
		else {
			// does not exist in base , add
			switch (rule->ruleNew)
			{
			case RBMergeRuleNew::RBMERGE_IGNORE:
				break;
			case RBMergeRuleNew::RBMERGE_ADD:
				AddNode(otherNode);
				break;
			default:
				break;
			}
		}
	}
	return;
	// handle removed nodes
	for (const auto& baseEntry : listMap) {
		auto baseNode = baseEntry.second.second;
		//const RBNodeType baseType = otherNode->GetType();

		auto otherEntry = otherListMap.find(baseEntry.first);

		rule = rules->Get(baseEntry.first);
		if (otherEntry == otherListMap.end()) {
			switch (rule->ruleRemoved)
			{
			case RBMergeRuleRemoved::RBMERGE_IGNORE:
				break;
			case RBMergeRuleRemoved::RBMERGE_REMOVE:
				RemoveNode(baseNode);
				break;
			default:
				break;
			}

		}
	}
}

void RBNodeList::Serialize(std::ostream& out, int indent)
{
	writeLnBracketOpenNamed(out, indent, m_name);
	for (const auto& node : m_nodes) {
		node->Serialize(out, indent + 1);
	}
	writeLnBracketClose(out, indent);
}

void RBNodeValue::Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules)
{
	if (other->GetName().compare(m_name) != 0) {
		std::stringstream ss;
		ss << "Attempt to merge '" << other->GetName() << "' with '" << m_name << "'.";
		throw std::runtime_error(ss.str());
	}

	if (other->GetType() != GetType()) {
		throw std::runtime_error("Can't merge nodes with different types.");
	}

	auto otherValue = std::static_pointer_cast<RBNodeValue>(other);

	m_value = otherValue->m_value;
}

void RBNodeValue::Serialize(std::ostream& out, int indent)
{
	writeLnPairIndented(out, indent, m_name, m_value);
}

void RBNodeEmpty::Merge(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules)
{
	if (other->GetName().compare(m_name) != 0) {
		std::stringstream ss;
		ss << "Attempt to merge '" << other->GetName() << "' with '" << m_name << "'.";
		throw std::runtime_error(ss.str());
	}

	if (other->GetType() != GetType()) {
		throw std::runtime_error("Can't merge nodes with different types.");
	}
}

void RBNodeEmpty::Serialize(std::ostream& out, int indent)
{
	writeLnIndented(out, indent, m_name);
}

const std::shared_ptr<RBMergeRule> RBMergeRules::Get(const std::string& name) const
{
	auto rule = m_rules.find(name);
	if (rule != m_rules.end()) {
		return rule->second;
	}
	else {
		return m_defaultRule;
	}
}
