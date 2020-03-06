#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


using namespace NFE;
using namespace NFE::RTTI;
using namespace NFE::Common;


//////////////////////////////////////////////////////////////////////////
// Test types definitions
//////////////////////////////////////////////////////////////////////////

NFE_DEFINE_CLASS(TestClassWithFundamentalMembers)
{
    NFE_CLASS_MEMBER(intValue);
    NFE_CLASS_MEMBER(floatValue);
    NFE_CLASS_MEMBER(boolValue);
    NFE_CLASS_MEMBER(strValue);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(TestClassWithNestedType)
{
    NFE_CLASS_MEMBER(foo);
    NFE_CLASS_MEMBER(bar);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(TestClassWithArrayType)
{
    NFE_CLASS_MEMBER(arrayOfInts);
    NFE_CLASS_MEMBER(foo);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(TestClassWithDynArrayType)
{
    NFE_CLASS_MEMBER(arrayOfInts);
    NFE_CLASS_MEMBER(arrayOfObjects);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(TestClassWithUniquePtrType)
{
    NFE_CLASS_MEMBER(pointerToInt);
    NFE_CLASS_MEMBER(pointerToObject);
    NFE_CLASS_MEMBER(pointerToArrayOfInts);
    NFE_CLASS_MEMBER(arrayOfPointersToInts);
}
NFE_END_DEFINE_CLASS()


// Polymorphic classes:

NFE_DEFINE_POLYMORPHIC_CLASS(TestAbstractClass)
{
    NFE_CLASS_MEMBER(intVal);
    NFE_CLASS_MEMBER(floatVal);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(TestChildClassA)
{
    NFE_CLASS_PARENT(TestBaseClass);
    NFE_CLASS_MEMBER(foo);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(TestChildClassB)
{
    NFE_CLASS_PARENT(TestBaseClass);
    NFE_CLASS_MEMBER(bar);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(TestBaseClass)
{
    NFE_CLASS_MEMBER(intVal);
    NFE_CLASS_MEMBER(floatVal);
    NFE_CLASS_MEMBER(mPrivateBool);
}
NFE_END_DEFINE_CLASS()


// Enum type

NFE_BEGIN_DEFINE_ENUM(TestEnum)
    NFE_ENUM_OPTION(OptionA)
    NFE_ENUM_OPTION(OptionB)
    NFE_ENUM_OPTION(OptionC)
NFE_END_DEFINE_ENUM()

//////////////////////////////////////////////////////////////////////////


namespace helper {

bool SerializeObject(const Type* type, const void* object, String& outString)
{
    Common::Config config;
    Common::ConfigValue value;
    if (!type->Serialize(object, config, value))
    {
        return false;
    }

    Common::ConfigObject root;
    config.AddValue(root, StringView("obj"), value);
    config.SetRoot(root);

    outString = config.ToString(false);
    return true;
}

bool DeserializeObject(const Type* type, void* outObject, const String& string)
{
    Common::Config config;
    config.Parse(string.Str());
    Common::ConfigGenericValue genericValue(&config);
    Common::ConfigValue value = genericValue["obj"];

    return type->Deserialize(outObject, config, value);
}

} // namespace helper