#pragma once
#include <memory>
#include <map>
#include <vector>
#include <string>

enum class RBMergeType {
	RBMERGE_DICT = 0,
	RBMERGE_LIST = 1,
};
enum class RBMergeRuleNew {
	RBMERGE_IGNORE = 0,
	RBMERGE_ADD = 1,
};
enum class RBMergeRuleRemoved {
	RBMERGE_IGNORE = 0,
	RBMERGE_REMOVE = 1,
};
enum class RBMergeRuleShared {
	RBMERGE_IGNORE = 0,
	RBMERGE_REPLACE = 1,
	RBMERGE_MERGE = 2,
};

class RBMergeRule {
public:
	RBMergeRule(std::string name, RBMergeType mergeType, std::string listKey, RBMergeRuleNew ruleNew, RBMergeRuleRemoved ruleRemoved, RBMergeRuleShared ruleShared)
		: name(name), mergeType(mergeType), listKey(listKey), ruleNew(ruleNew), ruleRemoved(ruleRemoved), ruleShared(ruleShared) {}

	std::string name;

	RBMergeType mergeType;
	std::string listKey;

	RBMergeRuleNew ruleNew;
	RBMergeRuleRemoved ruleRemoved;
	RBMergeRuleShared ruleShared;
};

class RBMergeRules {
public:
	RBMergeRules(const std::shared_ptr<RBMergeRule> defaultRule) : m_defaultRule(defaultRule) {}
	void Add(const std::string& name, const std::shared_ptr<RBMergeRule> rule) { m_rules.emplace(name, rule); }
	const std::shared_ptr<RBMergeRule> Get(const std::string& name) const;
private:
	std::map<std::string, std::shared_ptr<RBMergeRule>> m_rules;
	const std::shared_ptr<RBMergeRule> m_defaultRule;
};

std::vector<std::pair<std::vector<std::string>, std::shared_ptr<RBMergeRules>>> getKnownMergeFilesRules();