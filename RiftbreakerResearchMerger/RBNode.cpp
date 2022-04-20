#include "RBNode.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include "parser_utils.h"

bool RBNodeList::Contains(std::shared_ptr<RBNode> node) const
{
	return std::find(m_nodes.begin(), m_nodes.end(), node) != m_nodes.end();
}

bool RBNodeList::Contains(std::string name) const
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

std::shared_ptr<RBNode> RBNodeList::Copy() const
{
	std::shared_ptr<RBNodeList> copy = std::make_shared<RBNodeList>(m_name);
	for (const auto& node : m_nodes) {
		copy->AddNode(node->Copy());
	}
	return copy;
}

void RBNodeList::RemoveNode(std::shared_ptr<RBNode> node)
{
	auto it = std::find(m_nodes.begin(), m_nodes.end(), node);
	m_nodes.erase(it);
}

bool RBNodeList::IsDict() const
{
	if (Empty()) {
		return true;
	}
	
	for (int i = 0; i < m_nodes.size(); ++i) {
		for (int k = 0; k < m_nodes.size(); ++k) {
			if (k != i && m_nodes[k]->GetName().compare(m_nodes[i]->GetName()) == 0) {
				return false;
			}
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

bool RBNodeList::IsList() const
{
	if (Empty()) {
		return true;
	}
	std::string name = m_nodes[0]->GetName();
	for (const auto& node : m_nodes) {
		if (node->GetType() != RBNodeType::RBNODE_LIST) {
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

std::string RBNodeList::ListName() const
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
		std::shared_ptr<RBMergeRule> listElementRule = rules->Get(listName);
		if (listElementRule->listKey.empty()) {
			std::stringstream ss;
			ss << "List element type '" << listName << "' of list '" << m_name << "' has no list key set.";
			throw std::runtime_error(ss.str());
		}

		listMap = AsListMap(listElementRule->listKey);
		otherListMap = otherList->AsListMap(listElementRule->listKey);
	}

	for (const auto& otherEntry : otherListMap) {
		auto otherNode = otherEntry.second.second;
		//const RBNodeType otherType = otherNode->GetType();

		auto baseEntry = listMap.find(otherEntry.first);

		std::shared_ptr<RBMergeRule> nodeRule = rules->Get(otherNode->GetName());

		if (baseEntry != listMap.end()) {
			auto baseNode = baseEntry->second.second;
			const RBNodeType baseType = baseNode->GetType();
			// exists in base list, update/merge
			switch (nodeRule->ruleShared)
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
			switch (nodeRule->ruleNew)
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

		if (otherEntry == otherListMap.end()) {
			std::shared_ptr<RBMergeRule> nodeRule = rules->Get(baseNode->GetName());
			switch (nodeRule->ruleRemoved)
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

void RBNodeList::Serialize(std::ostream& out, int indent) const
{
	writeLnBracketOpenNamed(out, indent, m_name);
	for (const auto& node : m_nodes) {
		node->Serialize(out, indent + 1);
	}
	writeLnBracketClose(out, indent);
}

bool RBNodeList::Compare(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) const
{
	if (other->GetType() != RBNodeType::RBNODE_LIST || m_name.compare(other->GetName()) != 0) {
		return false;
	}

	std::shared_ptr<RBMergeRule> rule = rules->Get(m_name);
	std::shared_ptr<RBNodeList> otherList = std::static_pointer_cast<RBNodeList>(other);

	if (Size() != otherList->Size()) {
		return false;
	}

	if (rule->mergeType == RBMergeType::RBMERGE_DICT) {
		// both dict, same node names and same names equal
		if (!IsDict() || !otherList->IsDict()) {
			std::stringstream ss;
			ss << "Node '" << m_name << "' is not a valid dict.";
			throw std::runtime_error(ss.str());
		}
		// here: same length and all nodes have different names
		for (const auto& node : m_nodes) {
			if (otherList->Contains(node->GetName())) {
				const auto otherNode = otherList->GetNode(node->GetName());
				if (!node->Compare(otherNode, rules)) {
					return false;
				}
			}
			else {
				return false;
			}
		}
	}
	else if (rule->mergeType == RBMergeType::RBMERGE_LIST) {
		// both list, same keys (and listName) and all same keys equal
		if (!IsList() || !otherList->IsList()) {
			std::stringstream ss;
			ss << "Node '" << m_name << "' is not a valid list.";
			throw std::runtime_error(ss.str());
		}
		
		std::string listName = ListName();
		std::string otherListName = otherList->ListName();
		if (listName.compare(otherListName) != 0) {
			return false;
		}
		if (listName.empty()) {
			return true;
		}

		std::shared_ptr<RBMergeRule> listElementRule = rules->Get(listName);
		std::string listKeyName = listElementRule->listKey;
		if (listKeyName.empty()) {
			std::stringstream ss;
			ss << "List '" << m_name << "' with list nodes '" << listName << "' has no list key.";
			throw std::runtime_error(ss.str());
		}
		//here: same number of nodes, same node names, all nodes RBNodeList
		
		//check if nodes with the same key are identical
		//handle possibility that keys are duplicates?
		std::map<std::string, std::shared_ptr<RBNodeList>> keysNodes;
		for (int i = 0; i < m_nodes.size(); ++i) {
			//if (node->GetType() != RBNodeType::RBNODE_LIST)  -> checked in IsList()
			std::shared_ptr<RBNodeList> nodeList = std::static_pointer_cast<RBNodeList>(m_nodes[i]);
			if (!nodeList->Contains(listKeyName)) {
				std::stringstream ss;
				ss << "List element " << i << " of list '" << m_name << "' does not contain key node '" << listKeyName << "'.";
				throw std::runtime_error(ss.str());
			}
			std::shared_ptr<RBNode> keyNode = nodeList->GetNode(listKeyName);
			if (keyNode->GetType() != RBNodeType::RBNODE_VALUE) {
				std::stringstream ss;
				ss << "Key node '" << listKeyName << "' of list element " << i << " of list '" << m_name << "' is not a valid key node.";
				throw std::runtime_error(ss.str());
			}
			std::shared_ptr<RBNodeValue> keyNodeValue = std::static_pointer_cast<RBNodeValue>(keyNode);
			std::string nodeKey = keyNodeValue->GetValue();
			keysNodes.emplace(nodeKey, nodeList);
		}

		std::map<std::string, std::shared_ptr<RBNodeList>> otherKeysNodes;
		for (int i = 0; i < m_nodes.size(); ++i) {
			//if (node->GetType() != RBNodeType::RBNODE_LIST)  -> checked in IsList()
			std::shared_ptr<RBNodeList> nodeList = std::static_pointer_cast<RBNodeList>(otherList->m_nodes[i]);
			if (!nodeList->Contains(listKeyName)) {
				std::stringstream ss;
				ss << "List element " << i << " of list '" << m_name << "' does not contain key node '" << listKeyName << "'.";
				throw std::runtime_error(ss.str());
			}
			std::shared_ptr<RBNode> keyNode = nodeList->GetNode(listKeyName);
			if (keyNode->GetType() != RBNodeType::RBNODE_VALUE) {
				std::stringstream ss;
				ss << "Key node '" << listKeyName << "' of list element " << i << " of list '" << m_name << "' is not a valid key node.";
				throw std::runtime_error(ss.str());
			}
			std::shared_ptr<RBNodeValue> keyNodeValue = std::static_pointer_cast<RBNodeValue>(keyNode);
			std::string nodeKey = keyNodeValue->GetValue();
			otherKeysNodes.emplace(nodeKey, nodeList);
		}

		//can happen with duplicate keys. unless the duplicates are identical.
		if (keysNodes.size() != otherKeysNodes.size()) {
			return false;
		}
		
		for (const auto& [nodeKey, node] : keysNodes) {
			auto otherNode = otherKeysNodes.find(nodeKey);
			if (otherNode != otherKeysNodes.end()) { //contains
				if (!otherNode->second->Compare(node, rules)) {
					return false;
				}
			}
			else {
				return false;
			}
		}
	}
	return true;
}
/*
void RBNodeList::SetModified(const bool modified)
{
	m_modified = modified;
	for (auto& node : m_nodes) {
		node->SetModified(modified);
	}
}*/

void RBNodeList::RemoveEqual(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules)
{
	if (other->GetType() != RBNodeType::RBNODE_LIST || m_name.compare(other->GetName()) != 0) {
		throw std::runtime_error("Can't remove equal if base node is different.");
	}

	std::shared_ptr<RBMergeRule> rule = rules->Get(m_name);
	auto otherList = std::static_pointer_cast<RBNodeList>(other);

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
		std::shared_ptr<RBMergeRule> listElementRule = rules->Get(listName);
		if (listElementRule->listKey.empty()) {
			std::stringstream ss;
			ss << "List element type '" << listName << "' of list '" << m_name << "' has no list key set.";
			throw std::runtime_error(ss.str());
		}

		listMap = AsListMap(listElementRule->listKey);
		otherListMap = otherList->AsListMap(listElementRule->listKey);
	}

	for (const auto& baseEntry : listMap) {
		auto baseNode = baseEntry.second.second;
		//const RBNodeType baseType = otherNode->GetType();

		auto otherEntry = otherListMap.find(baseEntry.first);

		if (otherEntry != otherListMap.end()) {
			std::shared_ptr<RBMergeRule> nodeRule = rules->Get(baseNode->GetName());
			auto otherNode = otherEntry->second.second;
			if (baseNode->GetName().compare(rule->listKey)!=0 && baseNode->Compare(otherNode, rules)) {
				// do not remove entires that are used as list key.
				RemoveNode(baseNode);
			}
			else if(baseNode->GetName().compare(otherNode->GetName())==0 && baseNode->GetType()==RBNodeType::RBNODE_LIST && otherNode->GetType()==RBNodeType::RBNODE_LIST){
				baseNode->RemoveEqual(otherNode, rules);
			}
		}
	}
}

std::shared_ptr<RBNode> RBNodeValue::Copy() const
{
	return std::make_shared<RBNodeValue>(m_name, m_value);
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

void RBNodeValue::Serialize(std::ostream& out, int indent) const
{
	writeLnPairIndented(out, indent, m_name, m_value);
}

bool RBNodeValue::Compare(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) const
{
	if (other->GetType() != RBNodeType::RBNODE_VALUE || m_name.compare(other->GetName())!=0) {
		return false;
	}
	
	std::shared_ptr<RBNodeValue> otherValue = std::static_pointer_cast<RBNodeValue>(other);
	return m_value.compare(otherValue->m_value)==0;
}

std::shared_ptr<RBNode> RBNodeEmpty::Copy() const
{
	return std::make_shared<RBNodeEmpty>(m_name);
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

void RBNodeEmpty::Serialize(std::ostream& out, int indent) const
{
	writeLnIndented(out, indent, m_name);
}

bool RBNodeEmpty::Compare(std::shared_ptr<RBNode> other, std::shared_ptr<RBMergeRules> rules) const
{
	if (other->GetType() != RBNodeType::RBNODE_EMPTY || m_name.compare(other->GetName()) != 0) {
		return false;
	}

	return true;
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
