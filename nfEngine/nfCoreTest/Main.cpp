#include "PCH.hpp"
#include "../nfCommon/FileSystem.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");
    int result = RUN_ALL_TESTS();

    _CrtDumpMemoryLeaks();
    return result;
}
