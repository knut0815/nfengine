/**
* @file
* @author mkulagowski (mkkulagowski(at)gmail.com)
* @brief  Unit tests for FileBuffered class.
*/

#include "PCH.hpp"

#include "Engine/Common/FileSystem/FileBuffered.hpp"
#include "Engine/Common/FileSystem/FileSystem.hpp"
#include "Engine/Common/Math/Random.hpp"
#include "Engine/Common/Containers/String.hpp"

using namespace NFE;
using namespace NFE::Common;

// Global variables for the tests
namespace {
const uint32 bufferSize = 1000;                              //< Size of the test buffer
const NFE::uint8 operationsUpperLimit = 10;               //< Number of operations to perform on the buffer
const String testPath("./testFile.buffered");
static NFE::Math::Random gRandom;
} // namespace

class FileBufferedTest : public testing::Test
{
public:
    NFE::uint8 mBufferExpected[bufferSize];

    void SetUp()
    {
        // Fill buffer with gRandom data
        for (int i = 0; i < bufferSize; i++)
            mBufferExpected[i] = static_cast<NFE::uint8>(i);// mRand.GetInt());
    }

    void TearDown()
    {
        // Clean up after tests
        FileSystem::Remove(testPath);
    }
};

TEST_F(FileBufferedTest, Constructors)
{
    // Due to a bogus path, no file will be open for Read operation, but it shouldn't throw
    const String path("./some/path");
    FileBuffered();
    FileBuffered(path, AccessMode::Read);
    FileBuffered(path, AccessMode::Read, true);
    FileBuffered(FileBuffered(path, AccessMode::Read, true));

    // Just to make sure...
    ASSERT_TRUE(std::is_move_constructible<FileBuffered>::value);
    ASSERT_FALSE(std::is_copy_constructible<FileBuffered>::value);
}

TEST_F(FileBufferedTest, Read)
{
    NFE::uint8 bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    // Save values buffer to the file
    File testFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Write(mBufferExpected, bufferSize));
    testFile.Close();

    FileBuffered testBufferedFile(testPath, AccessMode::Read);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Do tha read, yo!
    size_t readSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        NFE::uint64 shift = i * readSize;
        ASSERT_EQ(readSize, testBufferedFile.Read(bufferActual + shift, readSize));
    }
    testBufferedFile.Close();

    // Check that data has been read successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));
}

TEST_F(FileBufferedTest, ReadSeek)
{
    NFE::uint8 bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    // Save values buffer to the file
    File testFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Write(mBufferExpected, bufferSize));
    testFile.Close();

    FileBuffered testBufferedFile(testPath, AccessMode::Read);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Do tha read, yo!
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        // Roll a gRandom number for Seek
        float rand = gRandom.GetFloat();

        // Scale it to range (0; bufferSize/2)
        NFE::int64 gRandomShift = static_cast<NFE::int64>((rand * bufferSize) / 2);

        // Roll a gRandom number for read size
        rand = gRandom.GetFloat();

        // Scale it to range (0; bufferSize)
        size_t gRandomSize = static_cast<size_t>((rand * bufferSize));

        // Make gRandom Seek
        ASSERT_TRUE(testBufferedFile.Seek(gRandomShift, SeekMode::Begin));

        // Read gRandom number of bytes from file
        size_t bytesRead = testBufferedFile.Read(bufferActual, gRandomSize);

        // It should be greater then 0, yet with gRandom reads we can't tell
        EXPECT_GT(bytesRead, static_cast<size_t>(0));

        // Check that data has been read successfully
        ASSERT_EQ(0, memcmp(mBufferExpected + gRandomShift, bufferActual, bytesRead));
    }
    testBufferedFile.Close();
}

TEST_F(FileBufferedTest, ReadWrite)
{
    NFE::uint8 bufferActual[bufferSize];
    memset(bufferActual, 0, bufferSize * sizeof(bufferActual[0]));

    // Save values buffer to the file
    File testFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Write(mBufferExpected, bufferSize));
    testFile.Close();

    // Open file in ReadWrite mode
    FileBuffered testBufferedFile(testPath, AccessMode::ReadWrite);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // First read whole previously written file to some buffer
    size_t operationSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        NFE::uint64 shift = i * operationSize;
        ASSERT_EQ(operationSize, testBufferedFile.Read(bufferActual + shift, operationSize));
    }

    // There should be nothing more to read, as we've reached EOF
    ASSERT_EQ(0u, testBufferedFile.Read(bufferActual, operationSize));
    //ASSERT_EQ(testBufferedFile.GetSize(), testBufferedFile.GetPos());

    // Then append buffer values to the file
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        NFE::uint64 shift = i * operationSize;
        ASSERT_EQ(operationSize, testBufferedFile.Write(mBufferExpected + shift, operationSize));
    }

    // File should have double the size of the buffer
    ASSERT_EQ(2 * bufferSize, testBufferedFile.GetSize());
    testBufferedFile.Close();

    // Open the file and perform 2 reads with the size of the buffer and check values
    testFile.Open(testPath, AccessMode::Read);
    ASSERT_TRUE(testFile.IsOpened());

    memset(bufferActual, 0, bufferSize * sizeof(bufferActual[0]));
    ASSERT_EQ(bufferSize, testFile.Read(bufferActual, bufferSize));
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));

    memset(bufferActual, 0, bufferSize * sizeof(bufferActual[0]));
    ASSERT_EQ(bufferSize, testFile.Read(bufferActual, bufferSize));
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));

    testFile.Close();
}

TEST_F(FileBufferedTest, Write)
{
    NFE::uint8 bufferActual[bufferSize];
    for (int i = 0; i < bufferSize; i++)
        bufferActual[i] = 0;

    FileBuffered testBufferedFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // So for one last time nigga make some writes
    size_t writeSize = bufferSize / operationsUpperLimit;
    for (int i = 0; i < operationsUpperLimit; i++)
    {
        NFE::uint64 shift = i * writeSize;
        ASSERT_EQ(writeSize, testBufferedFile.Write(mBufferExpected + shift, writeSize));
    }

    // Close must be performed after all operations has been done.
    // Otherwise they'll be canceled.
    testBufferedFile.Close();

    File testFile(testPath, AccessMode::Read);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Read(bufferActual, bufferSize));
    testFile.Close();

    // Check that data has been written successfully
    ASSERT_EQ(0, memcmp(mBufferExpected, bufferActual, bufferSize));
}

TEST_F(FileBufferedTest, OpenClose)
{
    // Make sure file is opened after constructor
    FileBuffered testBufferedFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Make sure file is closed after Close() method
    testBufferedFile.Close();
    ASSERT_FALSE(testBufferedFile.IsOpened());

    // Make sure file is opened after Open() method
    testBufferedFile.Open(testPath, AccessMode::Write);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Make sure file is closed after Close() method
    testBufferedFile.Close();
    ASSERT_FALSE(testBufferedFile.IsOpened());
}

TEST_F(FileBufferedTest, OperationsOnClosed)
{
    // Make sure file is opened after constructor
    FileBuffered testBufferedFile(testPath, AccessMode::ReadWrite, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    // Make sure file is closed after Close() method
    testBufferedFile.Close();
    ASSERT_FALSE(testBufferedFile.IsOpened());

    // Make sure no operations may be performed on closed file
    char buffer[bufferSize];
    ASSERT_EQ(0u, testBufferedFile.Write(buffer, bufferSize));
    ASSERT_EQ(0u, testBufferedFile.Read(buffer, bufferSize));
}

TEST_F(FileBufferedTest, InvalidOperations)
{
    char buffer[bufferSize];

    // Open file for writing, then try to read
    FileBuffered testBufferedFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    ASSERT_EQ(0u, testBufferedFile.Read(buffer, bufferSize));

    testBufferedFile.Close();

    // Reopen file for reading, then try to write
    testBufferedFile.Open(testPath, AccessMode::Read, true);
    ASSERT_TRUE(testBufferedFile.IsOpened());

    ASSERT_EQ(0u, testBufferedFile.Write(buffer, bufferSize));

    testBufferedFile.Close();
}

TEST_F(FileBufferedTest, GetSize)
{
    // Save values buffer to the file
    File testFile(testPath, AccessMode::Write, true);
    ASSERT_TRUE(testFile.IsOpened());
    ASSERT_EQ(bufferSize, testFile.Write(mBufferExpected, bufferSize));
    NFE::int64 expectedSize = testFile.GetSize();
    ASSERT_GT(expectedSize, 0);
    testFile.Close();

    FileBuffered bufFile(testPath, AccessMode::Write);
    ASSERT_TRUE(bufFile.IsOpened());
    ASSERT_EQ(expectedSize, bufFile.GetSize());
    bufFile.Close();

    bufFile.Open(testPath, AccessMode::Read);
    ASSERT_TRUE(bufFile.IsOpened());
    ASSERT_EQ(expectedSize, bufFile.GetSize());
    bufFile.Close();

    bufFile.Open(testPath, AccessMode::ReadWrite);
    ASSERT_TRUE(bufFile.IsOpened());
    ASSERT_EQ(expectedSize, bufFile.GetSize());
    bufFile.Close();
}