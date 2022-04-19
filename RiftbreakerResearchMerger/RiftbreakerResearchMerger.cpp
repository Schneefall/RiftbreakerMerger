
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <set>
#include <memory>
#include <regex>
#include "miniz/miniz.h"
#include "RBFile.h"
#include "RBMergeRules.h"
#include "Argparse.h"

const char* patchExt = ".merge";

bool archiveHasFile(const std::filesystem::path &archivePath, const std::string &fileName) {
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));
    std::string path = archivePath.string();
    mz_bool status = mz_zip_reader_init_file(&zip_archive, path.c_str(), 0);
    if (!status)
    {
        std::cerr << "Failed to read pack " << archivePath << ": " << zip_archive.m_last_error << std::endl;
        return false;
    }

    int i = mz_zip_reader_locate_file(&zip_archive, fileName.c_str(), nullptr, 0);

    mz_zip_reader_end(&zip_archive);

    return i >= 0;
}

std::pair<std::filesystem::path, std::vector<std::pair<std::filesystem::path, bool>>> getArchivesForMerge(const std::filesystem::path& packPath, const std::string& fileName) {

    std::set<std::filesystem::path> sortedPacks;
    for (const auto& file : std::filesystem::directory_iterator(packPath)) {
        sortedPacks.insert(file.path());
    }

    std::regex basePackMask("\\d\\d_.+_data\\.zip$", std::regex_constants::icase);
    std::regex ignorePackMask("\\d\\d_.+_(audio|video)\\.zip$", std::regex_constants::icase);
    std::regex archiveMask(".+.zip$", std::regex_constants::icase);

    std::filesystem::path basePack("");
    std::string basePackFileName("");
    std::vector<std::pair<std::filesystem::path, bool>> modPacks;
    
    for (const auto& file : sortedPacks) {
        if (std::filesystem::is_directory(file)) {
            continue;
        }
        std::string archiveName = file.filename().string();
        //std::cout << "Checking file " << archiveName << std::endl;
        if (std::regex_search(archiveName, ignorePackMask)) {
            //std::cout << "Ignored file." << std::endl;
            continue;
        }

        if (std::regex_search(archiveName, basePackMask)) {
            //std::cout << "Is base pack." << std::endl;
            if (fileName.compare(basePackFileName) > 0 && archiveHasFile(file, fileName)) { //
                //std::cout << "Has file." << std::endl;
                basePack = file;
                basePackFileName = archiveName;
            }
        }
        else if (std::regex_search(archiveName, archiveMask)) {
            //std::cout << "Is mod pack." << std::endl;
            std::string patchName = fileName + patchExt;
            if (archiveHasFile(file, patchName)) {
                //std::cout << "Has patch file." << std::endl;
                modPacks.push_back(std::pair(file.string(), true));
            }
            else if (archiveHasFile(file, fileName)) {
                //std::cout << "Has base file." << std::endl;
                modPacks.push_back(std::pair(file.string(), false));
            }
        }
    }

    return std::pair<std::filesystem::path, std::vector<std::pair<std::filesystem::path, bool>>>(basePack, modPacks);
}

std::shared_ptr<RBFile> readRBFile(const std::filesystem::path &archiveName, const std::string &fileName) {
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));
    size_t fileSize = 0;
    void* p_file;

    mz_bool status = mz_zip_reader_init_file(&zip_archive, archiveName.string().c_str(), 0);
    if (!status)
    {
        //printf("mz_zip_reader_init_file() failed!\n");
        throw std::runtime_error("Failed to initialize archive.");
    }

    p_file = mz_zip_reader_extract_file_to_heap(&zip_archive, fileName.c_str(), &fileSize, 0);
    if (!p_file) {
        throw std::runtime_error("Failed to read from archive.");
    }

    //std::string file(static_cast<const char*>(p_file), );
    std::string file;
    file.resize(fileSize);
    memcpy(file.data(), p_file, fileSize);

    mz_free(p_file);
    mz_zip_reader_end(&zip_archive);


    std::istringstream instream(file);

    std::shared_ptr<RBFile> researchFile = std::make_shared<RBFile>(instream);
    return researchFile;
}

bool saveRBFile(const std::filesystem::path& archiveName, const std::string& fileName, std::shared_ptr<RBFile> file) {

    std::ostringstream oss(std::ios::binary);
    file->Serialize(oss);
    std::string dataString = oss.str();
    const char* data = dataString.c_str();

    mz_bool status = mz_zip_add_mem_to_archive_file_in_place(archiveName.string().c_str(), fileName.c_str(), data, strlen(data) + 1, nullptr, 0, MZ_BEST_COMPRESSION);
    if (!status)
    {
        std::cout << "ERROR: Failed to write '" << fileName << "' to archive '" << archiveName << "'.";
        return false;
    }
    return true;
}

enum class MergeStatus {
    OK = 0,
    FAILED = 1,
};

MergeStatus createMergeFile(const std::filesystem::path& packPath, const std::string& fileName, const std::string &mergeFileName, std::shared_ptr<RBMergeRules> rules) {
    std::cout << std::endl << "Merging '" << fileName << "'." << std::endl;
    
    auto paths = getArchivesForMerge(packPath, fileName);
    auto basePath = paths.first;
    if (basePath.empty()) {
        std::cout << "ERROR: Could not find base pack for file " << fileName << "." << std::endl;
        return MergeStatus::FAILED;
    }
    auto modPaths = paths.second;
    if (modPaths.size() == 0) {
        std::cout << "File " << fileName << " is not modified by any mods." << std::endl;
        return MergeStatus::OK;
    }
    std::cout << "Found latest base file in " << basePath << ", modified by " << modPaths.size() << " mod packs." << std::endl;

    std::cout << "Reading base pack." << std::endl;
    std::shared_ptr<RBFile> baseReseachFile;
    try {
        baseReseachFile = readRBFile(basePath, fileName);
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to parse base pack: " << e.what() << std::endl;
        return MergeStatus::FAILED;
    }

    for (const auto& modPack : modPaths) {
        std::cout << "Reading " << (modPack.second ? "patch file" : "base file") << " from mod pack '" << modPack.first << "'." << std::endl;
        std::shared_ptr<RBFile> modFile;
        std::string modFileName = modPack.second ? fileName + patchExt : fileName;
        try {
            //modFiles.push_back(readResearchFile(modPack, researchFile));
            modFile = readRBFile(modPack.first, modFileName);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to parse mod pack: " << e.what() << std::endl;
            return MergeStatus::FAILED;
        }

        std::cout << "Updating base pack with mod pack." << std::endl;
        try {
            baseReseachFile->Merge(modFile, rules);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to merge: " << e.what() << std::endl;
            return MergeStatus::FAILED;
        }
    }

    std::filesystem::path mergedPath = std::filesystem::path(packPath).append(mergeFileName);
    if (!saveRBFile(mergedPath.string(), fileName, baseReseachFile)) {
        return MergeStatus::FAILED;
    }

    return MergeStatus::OK;
}



std::shared_ptr<RBMergeRules> getResearchMergeRules() {
    auto defaultRule = std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE);
    auto rules = std::make_shared<RBMergeRules>(defaultRule);
    
    // append trees
    rules->Add("categories", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "category", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    // append nodes
    rules->Add("nodes", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "research_name", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    // overwrite requirements
    rules->Add("requirements", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    // overwrite scripts
    rules->Add("research_scripts", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_REPLACE));
    // append awards
    rules->Add("research_awards", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "blueprint", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));

    return rules;
}

std::shared_ptr<RBMergeRules> getWeaponStatsMergeRules() {
    auto defaultRule = std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_DICT, "", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE);
    auto rules = std::make_shared<RBMergeRules>(defaultRule);

    // stats list
    rules->Add("stat_def_vec", std::make_shared<RBMergeRule>("", RBMergeType::RBMERGE_LIST, "stat_type", RBMergeRuleNew::RBMERGE_ADD, RBMergeRuleRemoved::RBMERGE_IGNORE, RBMergeRuleShared::RBMERGE_MERGE));
    return rules;
}

void waitForExit() {
    // Keep console open
    std::cin.ignore(std::cin.rdbuf()->in_avail());
    std::cout << std::endl;
    std::cout << "Press [Enter] to exit...";
    std::cin.ignore();
}

int main(const int argc, const char** argv)
{
    std::cout << "Riftbreaker Research Tree Merger." << std::endl << std::endl;
    
    std::filesystem::path packPath;
    std::string mergedPackName;

    try {
        Argparse args(argc, argv);
        packPath = args.GetString("packpath");
        mergedPackName = args.GetString("outname");
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to read arguments:\n\t" << e.what() << std::endl;
        std::cerr << "Available arguments:\n-packpath <path to pack files> -rtpath <unused> -outpath <name of merge file>";
        waitForExit();
        return -1;
    }

    std::filesystem::path mergedPath = std::filesystem::path(packPath).append(mergedPackName);

    try
    {
        remove(mergedPath);
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: Failed to remove old merged pack:\n\t" << e.what() << std::endl;
        waitForExit();
        return -1;
    }
    
    size_t files = 0;
    size_t failed = 0;
    {
        std::vector<std::string> treeFiles = { "scripts/research/research_tree.rt","scripts/research/research_tree_prologue.rt", "scripts/research/research_tree_survival.rt" };
        auto researchRules = getResearchMergeRules();
        for (const std::string researchFile : treeFiles) {
            ++files;
            MergeStatus status = createMergeFile(packPath, researchFile, mergedPackName, researchRules);
            if (status == MergeStatus::FAILED) {
                ++failed;
            }
        }
    }

    {
        std::vector<std::string> treeFiles = { "scripts/blueprint_tables/weapon_stats.dat" };
        auto researchRules = getWeaponStatsMergeRules();
        for (const std::string researchFile : treeFiles) {
            ++files;
            MergeStatus status = createMergeFile(packPath, researchFile, mergedPackName, researchRules);
            if (status == MergeStatus::FAILED) {
                ++failed;
            }
        }
    }

    std::cout << std::endl;
    if (failed > 0) {
        std::cout << failed << " of " << files << " FAILED to merge." << std::endl;
    }
    else {
        std::cout << "All " << files << " files merged SUCCESSFULLY." << std::endl;
    }

    waitForExit();
}
