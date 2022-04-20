#include "RBMergeRules.h"
#include <memory>
#include <vector>
#include <string>


std::shared_ptr<RBMergeRules> getResearchMergeRules() {
    auto defaultRule = std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE);
    auto rules = std::make_shared<RBMergeRules>(defaultRule);

    // append trees (list)
    rules->Add("categories", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    rules->Add("ResearchTree", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "category", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    // append nodes (list)
    rules->Add("nodes", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    rules->Add("ResearchNode", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "research_name", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    // append awards (list)
    rules->Add("research_awards", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    rules->Add("ResearchAward", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "blueprint", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));

    // overwrite flags (unused?)
    rules->Add("research_flags", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    // overwrite tooltip
    rules->Add("requirement_tooltip", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    // overwrite requirements
    rules->Add("requirements", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    rules->Add("ResearchNodeRequirement", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "research_name", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    // overwrite costs
    rules->Add("research_costs", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    rules->Add("ResearchCost", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "resource", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    
    // overwrite scripts
    rules->Add("research_scripts", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    rules->Add("ResearchScript", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "script_name", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));

    rules->Add("Strings", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    rules->Add("StringData", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "key", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    rules->Add("Floats", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    //rules->Add("StringData", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "key", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    rules->Add("Vectors", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    //rules->Add("StringData", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "key", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    rules->Add("Integers", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    rules->Add("IntData", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "key", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));

    return rules;
}

std::shared_ptr<RBMergeRules> getWeaponStatsMergeRules() {
    auto defaultRule = std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE);
    auto rules = std::make_shared<RBMergeRules>(defaultRule);

    // stats list
    rules->Add("stat_def_vec", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    rules->Add("WeaponStatDef", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "stat_type", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    return rules;
}

std::vector<std::pair<std::vector<std::string>, std::shared_ptr<RBMergeRules>>> getKnownMergeFilesRules() {
    
    std::vector<std::pair<std::vector<std::string>, std::shared_ptr<RBMergeRules>>> mergeFilesRules;

    // research tree files
    {
        std::vector<std::string> files = { "scripts/research/research_tree.rt","scripts/research/research_tree_prologue.rt", "scripts/research/research_tree_survival.rt" };
        std::shared_ptr<RBMergeRules> rules = getResearchMergeRules();
        mergeFilesRules.push_back(std::pair(files, rules));
    }

    {
        std::vector<std::string> files = { "scripts/blueprint_tables/weapon_stats.dat" };
        std::shared_ptr<RBMergeRules> rules = getWeaponStatsMergeRules();
        mergeFilesRules.push_back(std::pair(files, rules));
    }

    return mergeFilesRules;
}