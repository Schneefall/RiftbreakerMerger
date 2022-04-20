
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
//#include "miniz/miniz.c"
#include "RBFile.h"
#include "RBMergeRules.h"
#include "Argparse.h"

const char* patchExt = ".merge";

enum class MergeStatus {
    OK = 0,
    FAILED = 1,
    NOOP = 2,
};

bool archiveHasFile(const std::filesystem::path &archivePath, const std::string &fileName) {
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));
    std::string path = archivePath.string();
    mz_bool status = mz_zip_reader_init_file(&zip_archive, path.c_str(), 0);
    if (!status)
    {
        std::cerr << "Failed to read pack " << archivePath << ": " << zip_archive.m_last_error << std::endl;
        //debugging large archive error
        /*
        std::cout << "[TEST] " << "archive size: " << zip_archive.m_archive_size;
        std::cout << ", total files: " << zip_archive.m_total_files;
        std::cout << ", zip mode: " << zip_archive.m_zip_mode;
        std::cout << ", zip type: " << zip_archive.m_zip_type;
        std::cout << std::endl;
        throw std::runtime_error("DEBUG");
        */
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
    //throw std::runtime_error("DEBUG");

    return std::pair<std::filesystem::path, std::vector<std::pair<std::filesystem::path, bool>>>(basePack, modPacks);
}
std::filesystem::path getBaseArchiveForFile(const std::filesystem::path& packPath, const std::string& fileName) {

    std::set<std::filesystem::path> sortedPacks;
    for (const auto& file : std::filesystem::directory_iterator(packPath)) {
        sortedPacks.insert(file.path());
    }

    std::regex basePackMask("\\d\\d_.+_data\\.zip$", std::regex_constants::icase);
    std::regex ignorePackMask("\\d\\d_.+_(audio|video)\\.zip$", std::regex_constants::icase);
    //std::regex archiveMask(".+.zip$", std::regex_constants::icase);

    std::filesystem::path basePack("");
    std::string basePackFileName("");

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
    }

    return basePack;
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

bool addRBFileToPack(const std::filesystem::path& archiveName, const std::string& fileName, std::shared_ptr<RBFile> file) {

    std::ostringstream oss(std::ios::binary);
    file->Serialize(oss);
    std::string dataString = oss.str();
    const char* data = dataString.c_str();

    mz_bool status = mz_zip_add_mem_to_archive_file_in_place(archiveName.string().c_str(), fileName.c_str(), data, strlen(data), nullptr, 0, MZ_BEST_COMPRESSION);
    if (!status)
    {
        std::cerr << "ERROR: Failed to write '" << fileName << "' to archive '" << archiveName << "'.";
        return false;
    }
    return true;
}

MergeStatus  createMergeFile(const std::filesystem::path& packPath, const std::string& fileName, const std::string &mergeFileName, std::shared_ptr<RBMergeRules> rules, const bool verbose) {
    std::cout << std::endl << "Merging '" << fileName << "'." << std::endl;
    
    auto paths = getArchivesForMerge(packPath, fileName);
    auto basePath = paths.first;
    if (basePath.empty()) {
        std::cerr << "ERROR: Could not find base pack for file " << fileName << "." << std::endl;
        return MergeStatus::FAILED;
    }
    auto modPaths = paths.second;
    if (modPaths.size() == 0) {
        if(verbose) std::cout << "File " << fileName << " is not modified by any mods." << std::endl;
        return MergeStatus::OK;
    }
    if (verbose) std::cout << "Found latest base file in " << basePath << ", modified by " << modPaths.size() << " mod packs." << std::endl;

    if (verbose) std::cout << "Reading base pack." << std::endl;
    std::shared_ptr<RBFile> baseReseachFile;
    try {
        baseReseachFile = readRBFile(basePath, fileName);
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to parse base pack: " << e.what() << std::endl;
        return MergeStatus::FAILED;
    }

    std::shared_ptr<RBFile> mergeFile = baseReseachFile->Copy();

    for (const auto& modPack : modPaths) {
        std::filesystem::path modPackPath = modPack.first;
        bool isPatchFile = modPack.second;
        if (verbose) std::cout << "Reading " << (isPatchFile ? "patch file" : "base file") << " from mod pack '" << modPackPath.filename() << "'." << std::endl;
        std::shared_ptr<RBFile> modFile;
        std::string modFileName = isPatchFile ? fileName + patchExt : fileName;
        try {
            //modFiles.push_back(readResearchFile(modPack, researchFile));
            modFile = readRBFile(modPackPath, modFileName);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to parse mod pack: " << e.what() << std::endl;
            return MergeStatus::FAILED;
        }

        if (!isPatchFile) {
            if (verbose) std::cout << "Creating patch file." << std::endl;
            modFile->RemoveEqual(baseReseachFile, rules);
        }

        if (verbose) std::cout << "Updating with patch file." << std::endl;
        try {
            mergeFile->Merge(modFile, rules);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to merge: " << e.what() << std::endl;
            return MergeStatus::FAILED;
        }
    }

    std::filesystem::path mergedPath = std::filesystem::path(packPath).append(mergeFileName);
    if (!addRBFileToPack(mergedPath.string(), fileName, mergeFile)) {
        return MergeStatus::FAILED;
    }

    return MergeStatus::OK;
}

int mergeKnownFiles(std::filesystem::path& packPath, std::string& mergedPackName, const bool verbose) {

    std::filesystem::path mergedPath = std::filesystem::path(packPath).append(mergedPackName);

    try
    {
        remove(mergedPath);
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: Failed to remove old merged pack:\n\t" << e.what() << std::endl;
        return -1;
    }

    size_t numFiles = 0;
    size_t failed = 0;

    auto mergeFilesRules = getKnownMergeFilesRules();

    for (const auto& mergeFilesRule : mergeFilesRules)
    {
        std::vector<std::string> files = mergeFilesRule.first;
        std::shared_ptr<RBMergeRules> rules = mergeFilesRule.second;
        for (const std::string file : files) {
            ++numFiles;
            MergeStatus status = createMergeFile(packPath, file, mergedPackName, rules, verbose);
            if (status == MergeStatus::FAILED) {
                ++failed;
            }
        }
    }

    std::cout << std::endl;
    if (failed > 0) {
        std::cout << failed << " of " << numFiles << " FAILED to merge." << std::endl;
    }
    else {
        std::cout << "All " << numFiles << " files merged SUCCESSFULLY." << std::endl;
    }

    return 0;
}

std::pair<MergeStatus, std::shared_ptr<RBFile>> createPatchFile(const std::filesystem::path& packPath, const std::string& fileName, const std::string& modPackName, std::shared_ptr<RBMergeRules> rules, const bool verbose) {
    
    std::filesystem::path modPackPath = std::filesystem::path(packPath).append(modPackName);
    if (!archiveHasFile(modPackPath, fileName)) {
        if(verbose) std::cout << "File " << fileName << " is not modified." << std::endl;
        return std::pair(MergeStatus::NOOP, nullptr);
    }
    
    if (verbose) std::cout << "Creating patch for file '" << fileName << "':" << std::endl;

    auto basePath = getBaseArchiveForFile(packPath, fileName);
    if (basePath.empty()) {
        std::cerr << "ERROR: Could not find base pack for file " << fileName << "." << std::endl;
        return std::pair(MergeStatus::FAILED, nullptr);
    }
    if (verbose) std::cout << "Found latest base file in " << basePath << std::endl;

    if (verbose) std::cout << "Reading base file." << std::endl;
    std::shared_ptr<RBFile> baseFile;
    try {
        baseFile = readRBFile(basePath, fileName);
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to parse base file: " << e.what() << std::endl;
        return std::pair(MergeStatus::FAILED, nullptr);
    }

    if (verbose) std::cout << "Reading file from mod pack '" << modPackPath.filename() << "'." << std::endl;
    std::shared_ptr<RBFile> modFile;
    try {
        //modFiles.push_back(readResearchFile(modPack, researchFile));
        modFile = readRBFile(modPackPath, fileName);
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to parse mod pack: " << e.what() << std::endl;
        return std::pair(MergeStatus::FAILED, nullptr);
    }

    if (verbose) std::cout << "Creating patch file." << std::endl;
    try {
        modFile->RemoveEqual(baseFile, rules);
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to create patch: " << e.what() << std::endl;
        return std::pair(MergeStatus::FAILED, nullptr);
    }

    /*
    std::string patchFileName = fileName + patchExt;
    //std::filesystem::path patchPath = std::filesystem::path(packPath).append(patchFileName);
    if (!addRBFileToPack(modPackPath.string(), patchFileName, modFile)) {
        return std::pair(MergeStatus::FAILED, nullptr);
    }
    */
    return std::pair(MergeStatus::OK, modFile);
}

bool updatePatchesInPack(const std::filesystem::path& archivePath, std::map<std::string, std::shared_ptr<RBFile>>& patchFiles, const bool verbose) {
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));
    std::string path = archivePath.string();
    mz_bool status = mz_zip_reader_init_file(&zip_archive, path.c_str(), 0);
    if (!status)
    {
        std::cerr << "Failed to read pack " << archivePath << ": " << zip_archive.m_last_error << std::endl;
        return false;
    }

    mz_zip_archive_file_stat zip_file_stat;
    bool copy = false;
    mz_uint numFiles = mz_zip_reader_get_num_files(&zip_archive);
    if (verbose) std::cout << "Checking for old patch files." << std::endl;
    for (mz_uint i = 0; i < numFiles; ++i) {
        memset(&zip_file_stat, 0, sizeof(zip_file_stat));
        status = mz_zip_reader_file_stat(&zip_archive, i, &zip_file_stat);
        if (!status)
        {
            std::cerr << "Failed to file stat from " << archivePath << ": " << zip_archive.m_last_error << std::endl;
            return false;
        }

        std::string filename(zip_file_stat.m_filename);
        if (filename.find(patchExt)!=std::string::npos) { // == (filename.length() - std::strlen(patchExt))
            if (verbose) std::cout << "Found patch file." << std::endl;
            copy = true;
            break;
        }
    }

    if (copy) {
        //there are old patch files that need to be removed, so copy the other files to new archive
        std::string tempPath = archivePath.string() + ".temp";
        if (verbose) std::cout << "Copy to temporary pack." << tempPath << std::endl;

        mz_zip_archive out_archive;
        memset(&out_archive, 0, sizeof(out_archive));
        status = mz_zip_writer_init_file(&out_archive, tempPath.c_str(), 0);
        if (!status)
        {
            std::cerr << "Failed initialize temporary pack " << tempPath << ": " << zip_archive.m_last_error << std::endl;
            return false;
        }

        for (mz_uint i = 0; i < numFiles; ++i) {
            memset(&zip_file_stat, 0, sizeof(zip_file_stat));
            status = mz_zip_reader_file_stat(&zip_archive, i, &zip_file_stat);
            if (!status)
            {
                std::cerr << "Failed to file stat from " << archivePath << ": " << zip_archive.m_last_error << std::endl;
                return false;
            }

            std::string filename(zip_file_stat.m_filename);
            if (filename.find(patchExt) == std::string::npos) {
                if (verbose) std::cout << "Copy file: " << filename << std::endl;
                status = mz_zip_writer_add_from_zip_reader(&out_archive, &zip_archive, i);
                if (!status)
                {
                    std::cerr << "Failed to copy file " << filename << " to temporary pack " << tempPath << ": " << zip_archive.m_last_error << std::endl;
                    return false;
                }
            }
            else {
                if (verbose) std::cout << "Ignore old patch file: " << filename << std::endl;
            }
        }

        for (const auto& [filename, file] : patchFiles) {
            std::ostringstream oss(std::ios::binary);
            file->Serialize(oss);
            std::string dataString = oss.str();
            const char* data = dataString.c_str();

            if (verbose) std::cout << "Write new patch file: " << filename << std::endl;

            status = mz_zip_writer_add_mem(&out_archive, filename.c_str(), data, strlen(data), MZ_BEST_COMPRESSION);
            if (!status)
            {
                std::cerr << "Failed to write new patch file " << filename << " to temporary pack " << tempPath << ": " << zip_archive.m_last_error << std::endl;
                return false;
            }
        }

        if (verbose) std::cout << "Finalize temporary pack." << std::endl;
        status = mz_zip_reader_end(&zip_archive);
        status = mz_zip_writer_finalize_archive(&out_archive);
        if (!status)
        {
            std::cerr << "Failed to finalize temporary pack " << tempPath << ": " << zip_archive.m_last_error << std::endl;
            return false;
        }
        status = mz_zip_writer_end(&out_archive);

        if (verbose) std::cout << "Remove old mod pack." << std::endl;
        std::filesystem::remove(archivePath);
        if (verbose) std::cout << "Rename temporary pack." << std::endl;
        std::filesystem::rename(tempPath, archivePath);
    }
    else {
        if (verbose) std::cout << "Write patches to mod pack." << std::endl;
        status = mz_zip_writer_init_from_reader(&zip_archive, path.c_str());

        for (const auto& [filename, file] : patchFiles) {
            std::ostringstream oss(std::ios::binary);
            file->Serialize(oss);
            std::string dataString = oss.str();
            const char* data = dataString.c_str();

            if (verbose) std::cout << "Write new patch file: " << filename << std::endl;
            status = mz_zip_writer_add_mem(&zip_archive, filename.c_str(), data, strlen(data), MZ_BEST_COMPRESSION);
            if (!status)
            {
                std::cerr << "Failed to write new patch file " << filename << " to pack " << path << ": " << zip_archive.m_last_error << std::endl;
                return false;
            }
        }


        if (verbose) std::cout << "Finalize mod pack." << std::endl;
        status = mz_zip_writer_finalize_archive(&zip_archive);
        if (!status)
        {
            std::cerr << "Failed to finalize pack " << path << ": " << zip_archive.m_last_error << std::endl;
            return false;
        }
        status = mz_zip_writer_end(&zip_archive);
    }

    return true;
}

int createPatch(std::filesystem::path& packPath, std::string& modPackName, const bool verbose) {

    std::filesystem::path modPackPath = std::filesystem::path(packPath).append(modPackName);
    if (!std::filesystem::exists(modPackPath)) {
        std::cerr << std::endl << "Mod pack  '" << modPackName << "' does not exist." << std::endl;
        return 0;
    }

    std::cout << std::endl << "Creating patch files for  '" << modPackName << "':" << std::endl;


    size_t numFiles = 0;
    size_t failed = 0;
    auto mergeFilesRules = getKnownMergeFilesRules();

    std::map<std::string, std::shared_ptr<RBFile>> patchFiles;
    for (const auto& mergeFilesRule : mergeFilesRules)
    {
        std::vector<std::string> files = mergeFilesRule.first;
        std::shared_ptr<RBMergeRules> rules = mergeFilesRule.second;
        for (const std::string file : files) {
            ++numFiles;
            auto status = createPatchFile(packPath, file, modPackName, rules, verbose);
            if (status.first == MergeStatus::FAILED) {
                ++failed;
            }
            else if (status.first == MergeStatus::OK) {
                patchFiles.emplace(file + patchExt, status.second);
            }
        }
    }

    std::cout << std::endl;
    if (failed > 0) {
        std::cout << failed << " of " << numFiles << " FAILED to patch." << std::endl;
    }
    else {
        std::cout << "All " << numFiles << " files patched SUCCESSFULLY." << std::endl;
    }

    if (verbose) std::cout << "Writing " << patchFiles.size() << " patches to mod pack." << std::endl;
    updatePatchesInPack(modPackPath, patchFiles, verbose);

    return 0;
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
    try
       {
        std::cout << "Riftbreaker Research Tree Merger." << std::endl << std::endl;
    
        std::filesystem::path packPath;
        std::string mergedPackName;
        std::string makePatchModPackName;
        bool verbose = true;

        try {
            Argparse args(argc, argv);
            packPath = args.GetString("packpath");
            mergedPackName = args.GetString("outname");
            makePatchModPackName = args.GetString("makepatch");
            verbose = args.GetBool("verbose");
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to read arguments:\n\t" << e.what() << std::endl;
            std::cerr << "Available arguments:\n-packpath <path to pack files> -rtpath <unused> -outpath <name of merge file>";
            waitForExit();
            return -1;
        }

        int status = 0;
        if (!makePatchModPackName.empty()) {
            // create a minimal patch file and write it to the mod archive
            status = createPatch(packPath, makePatchModPackName, verbose);
        }
        else {
            status = mergeKnownFiles(packPath, mergedPackName, verbose);
        }
        waitForExit();
        return status;
    }
    catch (const std::exception& e)
    {

        std::cerr << "ERROR: Riftbreaker Merger general error:\n\t" << e.what() << std::endl;
        waitForExit();
        return -1;
    }


}
