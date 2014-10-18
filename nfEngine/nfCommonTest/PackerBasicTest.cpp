#include "stdafx.hpp"
#include "../nfCommon/Packer/Packer.hpp"

using namespace NFE::Common;

const std::string testPackFilePath = "testfile.nfp";

class PackerBasicTest : public testing::Test
{
protected:
    void SetUp()
    {
        EXPECT_NO_THROW(mReader.reset(new PackReader()));
        EXPECT_NE(nullptr, mReader.get());

        EXPECT_NO_THROW(mWriter.reset(new PackWriter()));
        EXPECT_NE(nullptr, mWriter.get());
    }

    void TearDown()
    {
        mReader.reset();
        mWriter.reset();

        std::ifstream fs(testPackFilePath);
        if (fs.good())
        {
            fs.close();
            EXPECT_EQ(0, remove(testPackFilePath.c_str())) << "remove() failed. "
                    << "Error: " << errno << " (" << strerror(errno) << ")";
        }
    }

    std::unique_ptr<PackReader> mReader;
    std::unique_ptr<PackWriter> mWriter;
};

TEST_F(PackerBasicTest, WriterInitTest)
{
    PackResult pr;
    EXPECT_NO_THROW(pr = mWriter->Init(testPackFilePath));
    EXPECT_EQ(PackResult::OK, pr);
}

TEST_F(PackerBasicTest, WriterEmptyTest)
{
    PackResult pr;
    EXPECT_NO_THROW(pr = mWriter->Init(testPackFilePath));
    EXPECT_EQ(PackResult::OK, pr);

    EXPECT_NO_THROW(pr = mWriter->WritePAK());
    EXPECT_EQ(PackResult::OK, pr);

    // open file manually and check if it contains data in order:
    //   * version number
    //   * number of files (in this case 0)
    std::ifstream fs(testPackFilePath);
    EXPECT_TRUE(fs.good());

    uint32 readFileVersion;
    size_t readFileCount;

    fs.read(reinterpret_cast<char*>(&readFileVersion), sizeof(uint32));
    fs.read(reinterpret_cast<char*>(&readFileCount), sizeof(size_t));

    EXPECT_EQ(gPackFileVersion, readFileVersion);
    EXPECT_EQ(0, readFileCount);
}

TEST_F(PackerBasicTest, ReaderEmptyTest)
{
    // create correct file
    PackResult pr;
    EXPECT_NO_THROW(pr = mWriter->Init(testPackFilePath));
    EXPECT_EQ(PackResult::OK, pr);

    EXPECT_NO_THROW(pr = mWriter->WritePAK());
    EXPECT_EQ(PackResult::OK, pr);

    // open it with reader
    EXPECT_NO_THROW(pr = mReader->Init(testPackFilePath));
    EXPECT_EQ(PackResult::OK, pr);

    // get file version and file size
    uint32 readFileVersion;
    size_t readFileCount;
    EXPECT_NO_THROW(readFileVersion = mReader->GetFileVersion());
    EXPECT_EQ(gPackFileVersion, readFileVersion);

    EXPECT_NO_THROW(readFileCount = mReader->GetFileCount());
    EXPECT_EQ(0, readFileCount);
}

TEST_F(PackerBasicTest, ReaderBuggyPathTest)
{
    PackResult pr;

    // open path to non existing file with reader
    EXPECT_NO_THROW(pr = mReader->Init(testPackFilePath));
    EXPECT_EQ(PackResult::FileNotFound, pr);
}

TEST_F(PackerBasicTest, ReaderBuggyFileTest)
{
    PackResult pr;

    // create file with file version only
    std::ofstream file(testPackFilePath);
    EXPECT_TRUE(file.good());

    file.write(reinterpret_cast<const char*>(&gPackFileVersion), sizeof(gPackFileVersion));
    file.close();

    // initialization should fail - no information about file count inside archive
    EXPECT_NO_THROW(pr = mReader->Init(testPackFilePath));
    EXPECT_EQ(PackResult::ReadFailed, pr);
}
