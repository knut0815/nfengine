#include "PCH.hpp"
#include "Transcendental.hpp"
#include "VectorInt8.hpp"

namespace NFE {
namespace Math {

namespace {

NFE_FORCE_INLINE int32 FloatAsInt(const float f)
{
    Common::FundamentalTypesUnion bits;
    bits.f = f;
    return bits.i32;
}

NFE_FORCE_INLINE float IntAsFloat(const int32 i)
{
    Common::FundamentalTypesUnion bits;
    bits.i32 = i;
    return bits.f;
}

} // namespace

namespace sinCoeffs
{
    static const float c0 =  9.9999970197e-01f;
    static const float c1 = -1.6666577756e-01f;
    static const float c2 =  8.3325579762e-03f;
    static const float c3 = -1.9812576647e-04f;
    static const float c4 =  2.7040521217e-06f;
    static const float c5 = -2.0532988642e-08f;
}

float Sin(float x)
{
    using namespace sinCoeffs;

    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const int32 i = static_cast<int32>(x * (1.0f / NFE_MATH_PI));
    x -= static_cast<float>(i) * NFE_MATH_PI;

    const float x2 = x * x;

    float y = x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * c5)))));

    return (i & 1) ? -y : y;
}

const Vector4 Sin(const Vector4& a)
{
    using namespace sinCoeffs;

    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const VectorInt4 i = VectorInt4::Convert(a * (1.0f / NFE_MATH_PI));
    const Vector4 x = Vector4::NegMulAndAdd(i.ConvertToFloat(), NFE_MATH_PI, a);

    const Vector4 x2 = x * x;

    Vector4 y = Vector4::MulAndAdd(Vector4(c5), x2, Vector4(c4));
    y = Vector4::MulAndAdd(y, x2, Vector4(c3));
    y = Vector4::MulAndAdd(y, x2, Vector4(c2));
    y = Vector4::MulAndAdd(y, x2, Vector4(c1));
    y = Vector4::MulAndAdd(y, x2, Vector4(c0));
    y *= x;

    // equivalent of: (i & 1) ? -y : y;
    return y ^ (i << 31).CastToFloat();
}

const Vector8 Sin(const Vector8& a)
{
#ifdef NFE_USE_AVX2
    using namespace sinCoeffs;

    // based on:
    // https://www.gamedev.net/forums/topic/681723-faster-sin-and-cos/

    // range reduction
    const VectorInt8 i = VectorInt8::Convert(a * (1.0f / NFE_MATH_PI));
    const Vector8 x = Vector8::NegMulAndAdd(i.ConvertToFloat(), NFE_MATH_PI, a);

    const Vector8 x2 = x * x;

    Vector8 y = Vector8::MulAndAdd(Vector8(c5), x2, Vector8(c4));
    y = Vector8::MulAndAdd(y, x2, Vector8(c3));
    y = Vector8::MulAndAdd(y, x2, Vector8(c2));
    y = Vector8::MulAndAdd(y, x2, Vector8(c1));
    y = Vector8::MulAndAdd(y, x2, Vector8(c0));
    y *= x;

    // equivalent of: (i & 1) ? -y : y;
    return y ^ (i << 31).CastToFloat();
#else
    return Vector8{sinf(a[0]), sinf(a[1]), sinf(a[2]), sinf(a[3]), sinf(a[4]), sinf(a[5]), sinf(a[6]), sinf(a[7])};
#endif // NFE_USE_AVX2
}

// calculates acos(|x|)
static NFE_INLINE float ACosAbs(float x)
{
    // based on DirectXMath implementation:
    // https://github.com/Microsoft/DirectXMath/blob/master/Inc/DirectXMathMisc.inl

    x = fabsf(x);
    float root = sqrtf(1.0f - x);

    const float c0 = 1.5707963050f;
    const float c1 = -0.2145988016f;
    const float c2 = 0.0889789874f;
    const float c3 = -0.0501743046f;
    const float c4 = 0.0308918810f;
    const float c5 = -0.0170881256f;
    const float c6 = 0.0066700901f;
    const float c7 = -0.0012624911f;

    return root * (c0 + x * (c1 + x * (c2 + x * (c3 + x * (c4 + x * (c5 + x * (c6 + x * c7)))))));
}

float ACos(float x)
{
    NFE_ASSERT(x >= -1.0f && x <= 1.0f, "Invalid argument");

    bool nonnegative = (x >= 0.0f);
    const float acosAbs = ACosAbs(x);

    // acos(x) = pi - acos(-x) when x < 0
    return nonnegative ? acosAbs : Constants::pi<float> -acosAbs;
}

float ASin(float x)
{
    NFE_ASSERT(x >= -1.0f && x <= 1.0f, "Invalid argument");

    bool nonnegative = (x >= 0.0f);
    const float acosAbs = ACosAbs(x);

    // acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
    return nonnegative ?
        Constants::pi<float> / 2.0f - acosAbs :
        acosAbs - Constants::pi<float> / 2.0f;
}

float ATan(float x)
{
    // based on:
    // https://stackoverflow.com/questions/26692859/best-machine-optimized-polynomial-minimax-approximation-to-arctangent-on-1-1

    float t = Abs(x);

    // range reduction
    float z = t;
    if (t > 1.0f)
    {
        z = 1.0f / z;
    }

    const float x2 = z * z;
    float y = 2.78569828e-3f;
    y = y * x2 - 1.58660226e-2f;
    y = y * x2 + 4.24722321e-2f;
    y = y * x2 - 7.49753043e-2f;
    y = y * x2 + 1.06448799e-1f;
    y = y * x2 - 1.42070308e-1f;
    y = y * x2 + 1.99934542e-1f;
    y = y * x2 - 3.33331466e-1f;
    y *= x2;
    y = y * z + z;

    // atan(x) = pi/2 - atan(1/x)
    if (t > 1.0f)
    {
        y = Constants::pi<float> / 2.0f - y;
    }

    return CopySign(y, x);
}

float FastACos(float x)
{
    // based on:
    // https://stackoverflow.com/a/26030435/10061517

    float negate = float(x < 0);
    x = fabsf(x);
    float ret = -0.0187293f;
    ret = ret * x + 0.0742610f;
    ret = ret * x - 0.2121144f;
    ret = ret * x + 1.5707288f;
    ret = ret * sqrtf(1.0f - x);
    ret = ret - 2.0f * negate * ret;
    return negate * 3.14159265358979f + ret;
}

float FastExp(float x)
{
    // implementation based on: "A more accurate, performance-competitive implementation of expf" by njuffa

    // handle special cases: severe overflow / underflow
    if (x >= 87.0f) // overflow
    {
        return std::numeric_limits<float>::infinity();
    }
    else if (x <= -87.0f) // underflow
    {
        return 0.0f;
    }

    const float t = x * 1.442695041f;
    const float fi = floorf(t);
    const int32 i = (int32)fi;
    const float f = t - fi;

    Common::FundamentalTypesUnion bits;
    bits.f = (0.3371894346f * f + 0.657636276f) * f + 1.00172476f;
    bits.i32 += (i << 23);
    return bits.f;
}

const Vector4 FastExp(const Vector4& a)
{
    const Vector4 t = a * 1.442695041f;
    const Vector4 fi = Vector4::Floor(t);
    const VectorInt4 i = VectorInt4::Convert(fi);
    const Vector4 f = t - fi;

    Vector4 y = Vector4::MulAndAdd(f, Vector4(0.3371894346f), Vector4(0.657636276f));
    y = Vector4::MulAndAdd(f, y, Vector4(1.00172476f));

    VectorInt4 yi = VectorInt4::Cast(y);
    yi += (i << 23);
    y = yi.CastToFloat();

    const Vector4 range(87.0f);
    y = Vector4::Select(y, Vector4::Zero(), -a >= range);
    y = Vector4::Select(y, VECTOR_INF, a >= range);
    return y;
}

const Vector8 FastExp(const Vector8& a)
{
    const Vector8 t = a * 1.442695041f;
    const Vector8 fi = Vector8::Floor(t);
    const VectorInt8 i = VectorInt8::Convert(fi);
    const Vector8 f = t - fi;

    Vector8 y = Vector8::MulAndAdd(f, Vector8(0.3371894346f), Vector8(0.657636276f));
    y = Vector8::MulAndAdd(f, y, Vector8(1.00172476f));

    VectorInt8 yi = VectorInt8::Cast(y);
    yi += (i << 23);
    y = yi.CastToFloat();

    const Vector8 range(87.0f);
    y = Vector8::Select(y, Vector8::Zero(), -a >= range);
    y = Vector8::Select(y, VECTOR8_INF, a >= range);
    return y;
}

float Log(float x)
{
    // based on:
    // https://stackoverflow.com/questions/39821367/very-fast-logarithm-natural-log-function-in-c

    // range reduction
    const int32 e = (FloatAsInt(x) - 0x3f2aaaab) & 0xff800000;
    const float m = IntAsFloat(FloatAsInt(x) - e);
    const float i = 1.19209290e-7f * (float)e;

    const float f = m - 1.0f;
    const float s = f * f;

    // Compute log1p(f) for f in [-1/3, 1/3]
    float r = -0.130187988f * f + 0.140889585f;
    float t = -0.121489584f * f + 0.139809534f;
    r = r * s + t;
    r = r * f - 0.166845024f;
    r = r * f + 0.200121149f;
    r = r * f - 0.249996364f;
    r = r * f + 0.333331943f;
    r = r * f - 0.500000000f;
    r = r * s + f;
    r = i * 0.693147182f + r; // log(2)
    return r;
}

float FastLog(float x)
{
    // based on:
    // https://stackoverflow.com/questions/39821367/very-fast-logarithm-natural-log-function-in-c

    // range reduction
    const int32 e = (FloatAsInt(x) - 0x3f2aaaab) & 0xff800000;
    const float m = IntAsFloat(FloatAsInt(x) - e);
    const float i = 1.19209290e-7f * (float)e;

    const float f = m - 1.0f;
    const float s = f * f;

    // Compute log1p(f) for f in [-1/3, 1/3]
    float r = 0.230836749f * f - 0.279208571f;
    float t = 0.331826031f * f - 0.498910338f;
    r = r * s + t;
    r = r * s + f;
    r = i * 0.693147182f + r; // log(2)
    return r;
}

const Vector4 FastLog(const Vector4& a)
{
    // range reduction
    const VectorInt4 e = (VectorInt4::Cast(a) - VectorInt4(0x3f2aaaab)) & VectorInt4(0xff800000);
    const Vector4 m = (VectorInt4::Cast(a) - e).CastToFloat();
    const Vector4 i = e.ConvertToFloat() * 1.19209290e-7f;

    const Vector4 f = m - Vector4(1.0f);
    const Vector4 s = f * f;

    // Compute log1p(f) for f in [-1/3, 1/3]
    Vector4 r = Vector4::MulAndAdd(f, Vector4(0.230836749f), Vector4(-0.279208571f));
    Vector4 t = Vector4::MulAndAdd(f, Vector4(0.331826031f), Vector4(-0.498910338f));
    r = Vector4::MulAndAdd(r, s, t);
    r = Vector4::MulAndAdd(r, s, f);
    r = Vector4::MulAndAdd(i, Vector4(0.693147182f), r); // log(2)
    return r;
}

float FastATan2(const float y, const float x)
{
    // https://stackoverflow.com/questions/46210708/atan2-approximation-with-11bits-in-mantissa-on-x86with-sse2-and-armwith-vfpv4

    const float ax = Math::Abs(x);
    const float ay = Math::Abs(y);
    const float mx = Math::Max(ay, ax);
    const float mn = Math::Min(ay, ax);
    const float a = mn / mx;

    // Minimax polynomial approximation to atan(a) on [0,1]
    const float s = a * a;
    const float c = s * a;
    const float q = s * s;
    const float t = -0.094097948f * q - 0.33213072f;
    float r = (0.024840285f * q + 0.18681418f);
    r = r * s + t;
    r = r * c + a;

    // Map to full circle
    if (ay > ax) r = 1.57079637f - r;
    if (x < 0.0f) r = NFE_MATH_PI - r;
    if (y < 0.0f) r = -r;
    return r;
}

} // namespace Math
} // namespace NFE