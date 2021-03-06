/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unit tests for shared locking objects.
 */

#include "PCH.hpp"
#include "Engine/Common/Utils/Latch.hpp"
#include "Engine/Common/System/RWLock.hpp"
#include "Engine/Common/System/RWSpinLock.hpp"
#include "Engine/Common/Math/Math.hpp"
#include "Engine/Common/Containers/DynArray.hpp"

using namespace NFE;
using namespace NFE::Common;


template<typename T>
class SharedLockTest : public ::testing::Test
{
};

// TODO RWSpinLock
using LockTestTypes = ::testing::Types<RWLock, RWSpinLock>;
TYPED_TEST_SUITE(SharedLockTest, LockTestTypes);


TYPED_TEST(SharedLockTest, SimpleLock)
{
    TypeParam lock;
    {
        ScopedSharedLock<TypeParam> scopedLock(lock);
        ASSERT_TRUE(scopedLock.IsLocked());
    }
}

TYPED_TEST(SharedLockTest, SimpleTryAcquireShared)
{
    TypeParam lock;

    ASSERT_TRUE(lock.TryAcquireShared());
    ASSERT_TRUE(lock.TryAcquireShared());
    lock.ReleaseShared();
    lock.ReleaseShared();
}

TYPED_TEST(SharedLockTest, SimpleTryAcquireShared_Locked)
{
    TypeParam lock;

    lock.AcquireExclusive();

    for (uint32 i = 0; i < 1000; ++i)
    {
        ASSERT_FALSE(lock.TryAcquireShared());
    }

    lock.ReleaseExclusive();
}

TYPED_TEST(SharedLockTest, SimpleTryAcquireExclusive_Locked)
{
    TypeParam lock;

    lock.AcquireShared();

    for (uint32 i = 0; i < 1000; ++i)
    {
        ASSERT_FALSE(lock.TryAcquireExclusive());
    }

    lock.ReleaseShared();
}

TYPED_TEST(SharedLockTest, Multithreaded_Simple)
{
    TypeParam testLock; // the lock being tested

    uint32 counter;
    ConditionVariable hasEnteredCV;
    Mutex hasEnteredMutex;

    bool canContinue;
    ConditionVariable canContinueCV;
    Mutex canContinueMutex;

    const auto func = [&]()
    {
        ScopedSharedLock<TypeParam> scopedLock(testLock);

        // notify main thread about reaching synchronization point
        {
            ScopedExclusiveLock<Mutex> lock(hasEnteredMutex);
            counter++;
            hasEnteredCV.SignalOne();
        }

        // if we are here, it means that all the threads are inside shared lock

        // exit the shared lock only when notified by the main thread
        {
            ScopedExclusiveLock<Mutex> lock(canContinueMutex);
            while (!canContinue)
            {
                canContinueCV.Wait(lock);
            }
        }
    };

    counter = 0;
    canContinue = false;

    // spawn threads
    const uint32 numThreads = std::min(1024u, Thread::GetSystemThreadsCount());
    DynArray<Thread> threads(numThreads);
    for (uint32 i = 0; i < numThreads; ++i)
    {
        threads[i].Run(func);
    }

    // wait until all the threads reach synchronization point
    {
        ScopedExclusiveLock<Mutex> lock(hasEnteredMutex);
        while (counter < numThreads)
        {
            hasEnteredCV.Wait(lock);
        }
    }

    // when program execution reaches this place, this means
    // that counter has been incremented to maximum value
    ASSERT_EQ(counter, static_cast<uint32>(numThreads));

    // notify threads to continue
    {
        ScopedExclusiveLock<Mutex> lock(canContinueMutex);
        canContinue = true;
        canContinueCV.SignalAll();
    }

    threads.Clear();

    ASSERT_EQ(counter, static_cast<uint32>(numThreads));
}

// multiple writer threads write data to a buffer,
// multiple reader threads check if the buffer contains expected values
TYPED_TEST(SharedLockTest, Multithreaded_Validate)
{
    const uint32 maxIterations = 10000u;

    alignas(64) TypeParam lock; // for synchronizing counter access

    alignas(64) std::atomic<uint32> iteration(0u);

    alignas(64) std::atomic<uint32> numReaders(0u);
    alignas(64) std::atomic<uint32> numWriters(0u);

    alignas(64) std::atomic_bool finish(false);
    std::atomic_bool errorFound(false);

    const auto writerFunc = [&]()
    {
        while (!finish)
        {
            ScopedExclusiveLock<TypeParam> scopedLock(lock);
            numWriters++;

            const bool readersCheck = numReaders == 0;
            const bool writersCheck = numWriters == 1;

            NFE_ASSERT(readersCheck, "Readers number violation");
            NFE_ASSERT(writersCheck, "Writers number violation");

            if (!writersCheck || !readersCheck)
            {
                errorFound = true;
            }

            if (iteration++ >= maxIterations)
            {
                finish = true;
            }

            numWriters--;
        }
    };

    const auto readerFunc = [&]()
    {
        while (!finish)
        {
            ScopedSharedLock<TypeParam> scopedLock(lock);
            numReaders++;

            const bool writersCheck = numWriters == 0;
            NFE_ASSERT(writersCheck, "Writers number violation");
            if (!writersCheck)
            {
                errorFound = true;
            }

            numReaders--;
        }
    };

    const uint32 numReaderThreads = Math::Clamp(Thread::GetSystemThreadsCount() / 2, 4u, 64u);
    const uint32 numWriterThreads = Math::Clamp(Thread::GetSystemThreadsCount() / 4, 2u, 32u);

    DynArray<Thread> readerThreads(numReaderThreads);
    for (uint32 i = 0; i < numReaderThreads; ++i)
    {
        readerThreads[i].Run(readerFunc);
    }

    DynArray<Thread> writerThreads(numWriterThreads);
    for (uint32 i = 0; i < numWriterThreads; ++i)
    {
        writerThreads[i].Run(writerFunc);
    }

    readerThreads.Clear();
    writerThreads.Clear();

    ASSERT_FALSE(errorFound);
}
