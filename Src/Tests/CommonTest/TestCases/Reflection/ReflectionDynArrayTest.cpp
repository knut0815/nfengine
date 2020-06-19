#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Reflection/ReflectionUtils.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"
#include "Engine/Common/Reflection/SerializationContext.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"

using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;

namespace {

using TestDynArray = DynArray<int32>;

} // namespace

TEST(ReflectionDynArrayTest, Verify)
{
    const auto* type = GetType<TestDynArray>();
    ASSERT_NE(nullptr, type);

    EXPECT_TRUE("DynArray<NFE::int32>" == type->GetName());
    EXPECT_EQ(TypeKind::DynArray, type->GetKind());
    EXPECT_EQ(sizeof(TestDynArray), type->GetSize());
    EXPECT_EQ(alignof(TestDynArray), type->GetAlignment());
}

TEST(ReflectionDynArrayTest, DefaultObject)
{
    const TestDynArray* defaultObjectPtr = GetDefaultObject<TestDynArray>();
    ASSERT_NE(nullptr, defaultObjectPtr);

    const TestDynArray& defaultObject = *defaultObjectPtr;
    EXPECT_EQ(0u, defaultObject.Size());
}

TEST(ReflectionDynArrayTest, Compare)
{
    const auto* type = GetType<TestDynArray>();

    TestDynArray emptyArrayA;
    TestDynArray emptyArrayB;

    TestDynArray arrayA;
    arrayA.PushBack(1);

    TestDynArray arrayB;
    arrayB.PushBack(1);

    TestDynArray arrayC;
    arrayC.PushBack(1);
    arrayC.PushBack(2);

    EXPECT_TRUE(type->Compare(&emptyArrayA, &emptyArrayB));
    EXPECT_TRUE(type->Compare(&emptyArrayA, &emptyArrayA));
    EXPECT_TRUE(type->Compare(&arrayA, &arrayB));
    EXPECT_FALSE(type->Compare(&emptyArrayA, &arrayA));
    EXPECT_FALSE(type->Compare(&arrayA, &arrayC));
    EXPECT_FALSE(type->Compare(&arrayC, &arrayA));
}

TEST(ReflectionDynArrayTest, Resize)
{
    const auto* type = GetType<TestDynArray>();

    TestDynArray array;
    array.PushBack(123);

    EXPECT_TRUE(type->ResizeArray(&array, 4));
    ASSERT_EQ(4u, array.Size());

    EXPECT_EQ(123, array[0]);
    EXPECT_EQ(0, array[1]);
    EXPECT_EQ(0, array[2]);
    EXPECT_EQ(0, array[3]);
}

TEST(ReflectionDynArrayTest, Clone_Empty)
{
    const auto* type = GetType<TestDynArray>();

    TestDynArray arrayA;

    TestDynArray arrayB;
    arrayB.PushBack(1);
    EXPECT_EQ(1u, arrayB.Size());

    EXPECT_TRUE(type->Clone(&arrayB, &arrayA));
    EXPECT_EQ(0u, arrayB.Size());
}

TEST(ReflectionDynArrayTest, Clone_SingleElement)
{
    const auto* type = GetType<TestDynArray>();

    TestDynArray arrayA;
    arrayA.PushBack(123);

    TestDynArray arrayB;
    arrayB.PushBack(1);
    arrayB.PushBack(2);
    EXPECT_EQ(2u, arrayB.Size());

    EXPECT_TRUE(type->Clone(&arrayB, &arrayA));
    ASSERT_EQ(1u, arrayB.Size());
    EXPECT_EQ(123, arrayB[0]);
}

TEST(ReflectionDynArrayTest, SerializeBinary_Empty)
{
    const auto* type = GetType<TestDynArray>();

    Buffer buffer;
    SerializationContext context;
    {
        TestDynArray obj;
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        TestDynArray readObj;
        readObj.PushBack(1);

        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        EXPECT_EQ(0u, readObj.Size());
    }
}

TEST(ReflectionDynArrayTest, SerializeBinary)
{
    const auto* type = GetType<TestDynArray>();

    Buffer buffer;
    SerializationContext context;
    {
        TestDynArray obj;
        obj.PushBack(123);
        obj.PushBack(456);
        obj.PushBack(789);

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        TestDynArray readObj;
        readObj.PushBack(1);

        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        ASSERT_EQ(3u, readObj.Size());

        EXPECT_EQ(123, readObj[0]);
        EXPECT_EQ(456, readObj[1]);
        EXPECT_EQ(789, readObj[2]);
    }
}
