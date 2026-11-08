#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>

template <typename T>
static void SetReturnObject(asIScriptGeneric* pGen, const T& value)
{
  new (pGen->GetAddressOfReturnLocation()) T(value);
}

// plMath::Min and Max also have variadic overloads, whose address clang will not take through a cast
template <typename T>
static T plMath_Min2(T a, T b)
{
  return plMath::Min(a, b);
}

template <typename T>
static T plMath_Max2(T a, T b)
{
  return plMath::Max(a, b);
}

static void plMath_ASin_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plMath::ASin(pGen->GetArgFloat(0)));
}

static void plMath_ACos_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plMath::ACos(pGen->GetArgFloat(0)));
}

static void plMath_ATan_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plMath::ATan(pGen->GetArgFloat(0)));
}

static void plMath_ATan2_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plMath::ATan2(pGen->GetArgFloat(0), pGen->GetArgFloat(1)));
}

static void plMath_Lerp_Vec2_Generic(asIScriptGeneric* pGen)
{
  const plVec2& from = *static_cast<plVec2*>(pGen->GetArgObject(0));
  const plVec2& to = *static_cast<plVec2*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plMath::Lerp(from, to, pGen->GetArgFloat(2)));
}

static void plMath_Lerp_Vec3_Generic(asIScriptGeneric* pGen)
{
  const plVec3& from = *static_cast<plVec3*>(pGen->GetArgObject(0));
  const plVec3& to = *static_cast<plVec3*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plMath::Lerp(from, to, pGen->GetArgFloat(2)));
}

static void plMath_Lerp_Vec4_Generic(asIScriptGeneric* pGen)
{
  const plVec4& from = *static_cast<plVec4*>(pGen->GetArgObject(0));
  const plVec4& to = *static_cast<plVec4*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plMath::Lerp(from, to, pGen->GetArgFloat(2)));
}

static void plMath_Lerp_Color_Generic(asIScriptGeneric* pGen)
{
  const plColor& from = *static_cast<plColor*>(pGen->GetArgObject(0));
  const plColor& to = *static_cast<plColor*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plMath::Lerp(from, to, pGen->GetArgFloat(2)));
}

//////////////////////////////////////////////////////////////////////////
// plMath
//////////////////////////////////////////////////////////////////////////

void plAngelScriptEngineSingleton::Register_Math()
{
  // static functions
  m_pEngine->SetDefaultNamespace("plMath");

  AS_CHECK(m_pEngine->RegisterGlobalFunction("bool IsNaN(float value)", asFUNCTION(plMath::IsNaN<float>), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("bool IsNaN(double value)", asFUNCTION(plMath::IsNaN<double>), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("bool IsFinite(float value)", asFUNCTION(plMath::IsFinite<float>), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("bool IsFinite(double value)", asFUNCTION(plMath::IsFinite<double>), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Sin(plAngle a)", asFUNCTION(plMath::Sin), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Cos(plAngle a)", asFUNCTION(plMath::Cos), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Tan(plAngle a)", asFUNCTION(plMath::Tan), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("plAngle ASin(float f)", asFUNCTION(plMath_ASin_Generic), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plAngle ACos(float f)", asFUNCTION(plMath_ACos_Generic), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plAngle ATan(float f)", asFUNCTION(plMath_ATan_Generic), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plAngle ATan2(float x, float y)", asFUNCTION(plMath_ATan2_Generic), asCALL_GENERIC));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Exp(float f)", asFUNCTIONPR(plMath::Exp, (float), float), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Ln(float f)", asFUNCTIONPR(plMath::Ln, (float), float), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Log2(float f)", asFUNCTIONPR(plMath::Log2, (float), float), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("uint32 Log2i(uint32 uiVal)", asFUNCTIONPR(plMath::Log2i, (plUInt32), plUInt32), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Log10(float f)", asFUNCTIONPR(plMath::Log10, (float), float), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Log(float fBase, float f)", asFUNCTIONPR(plMath::Log, (float, float), float), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Pow2(float f)", asFUNCTIONPR(plMath::Pow2, (float), float), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Pow(float fBase, float fExp)", asFUNCTIONPR(plMath::Pow, (float, float), float), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 Pow2(plInt32 i)", asFUNCTIONPR(plMath::Pow2, (plInt32), plInt32), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 Pow(plInt32 iBase, plInt32 iExp)", asFUNCTIONPR(plMath::Pow, (plInt32, plInt32), plInt32), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Sqrt(float f)", asFUNCTIONPR(plMath::Sqrt, (float), float), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("double Sqrt(double f)", asFUNCTIONPR(plMath::Sqrt, (double), double), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Sign(float f)", asFUNCTION(plMath::Sign<float>), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 Sign(plInt32 f)", asFUNCTION(plMath::Sign<plInt32>), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Abs(float f)", asFUNCTION(plMath::Abs<float>), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 Abs(plInt32 f)", asFUNCTION(plMath::Abs<plInt32>), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 Min(plInt32 f1, plInt32 f2)", asFUNCTION(plMath_Min2<plInt32>), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Min(float f1, float f2)", asFUNCTION(plMath_Min2<float>), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 Max(plInt32 f1, plInt32 f2)", asFUNCTION(plMath_Max2<plInt32>), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Max(float f1, float f2)", asFUNCTION(plMath_Max2<float>), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 Clamp(plInt32 val, plInt32 min, plInt32 max)", asFUNCTIONPR(plMath::Clamp, (plInt32, plInt32, plInt32), plInt32), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Clamp(float val, float min, float max)", asFUNCTIONPR(plMath::Clamp, (float, float, float), float), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Floor(float f)", asFUNCTIONPR(plMath::Floor, (float), float), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Ceil(float f)", asFUNCTIONPR(plMath::Ceil, (float), float), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 FloorToInt(float f)", asFUNCTIONPR(plMath::FloorToInt, (float), plInt32), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plInt32 CeilToInt(float f)", asFUNCTIONPR(plMath::CeilToInt, (float), plInt32), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Lerp(float from, float to, float factor)", asFUNCTIONPR(plMath::Lerp, (float, float, float), float), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec2 Lerp(plVec2 from, plVec2 to, float factor)", asFUNCTION(plMath_Lerp_Vec2_Generic), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 Lerp(plVec3 from, plVec3 to, float factor)", asFUNCTION(plMath_Lerp_Vec3_Generic), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec4 Lerp(plVec4 from, plVec4 to, float factor)", asFUNCTION(plMath_Lerp_Vec4_Generic), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plColor Lerp(plColor from, plColor to, float factor)", asFUNCTION(plMath_Lerp_Color_Generic), asCALL_GENERIC));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("float Unlerp(float from, float to, float value)", asFUNCTIONPR(plMath::Unlerp, (float, float, float), float), asCALL_CDECL));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("bool IsEqual(float lhs, float rhs, float fEpsilon)", asFUNCTIONPR(plMath::IsEqual, (float, float, float), bool), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("bool IsZero(float value, float fEpsilon)", asFUNCTIONPR(plMath::IsZero, (float, float), bool), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("bool IsInRange(float value, float min, float max)", asFUNCTIONPR(plMath::IsInRange, (float, float, float), bool), asCALL_CDECL));

  // TODO AngelScript: finish plMath registration

  /* not exposed yet:

  plUInt32 WrapUInt(plUInt32 uiValue, plUInt32 uiExcludedMaxValue);
  plInt32 WrapInt(plInt32 iValue, plUInt32 uiExcludedMaxValue);
  plInt32 WrapInt(plInt32 iValue, plInt32 iMinValue, plInt32 iExcludedMaxValue);
  float WrapFloat01(float fValue);
  float WrapFloat(float fValue, float fMinValue, float fMaxValue);

  T Saturate(T value);

  plInt32 FloatToInt(float value);

  float Round(float f);
  plInt32 RoundToInt(float f);
  double Round(double f);
  float RoundToMultiple(float f, float fMultiple);
  double RoundToMultiple(double f, double fMultiple);

  Type Fraction(Type f);

  float Mod(float value, float fDiv);
  double Mod(double f, double fDiv);

  float RoundDown(float f, float fMultiple);
  double RoundDown(double f, double fMultiple);
  float RoundUp(float f, float fMultiple);
  double RoundUp(double f, double fMultiple);

  plInt32 RoundUp(plInt32 value, plUInt16 uiMultiple);
  plInt32 RoundDown(plInt32 value, plUInt16 uiMultiple);
  plUInt32 RoundUp(plUInt32 value, plUInt16 uiMultiple);
  plUInt32 RoundDown(plUInt32 value, plUInt16 uiMultiple);

  bool IsOdd(plInt32 i);
  bool IsEven(plInt32 i);

  T Step(T value, T edge);
  Type SmoothStep(Type value, Type edge1, Type edge2);
  Type SmootherStep(Type value, Type edge1, Type edge2);

  bool IsPowerOf(plInt32 value, plInt32 iBase);
  bool IsPowerOf2(plInt32 value);
  bool IsPowerOf2(plUInt32 value);
  bool IsPowerOf2(plUInt64 value);

  plUInt32 PowerOfTwo_Floor(plUInt32 value);
  plUInt64 PowerOfTwo_Floor(plUInt64 value);
  plUInt32 PowerOfTwo_Ceil(plUInt32 value);
  plUInt64 PowerOfTwo_Ceil(plUInt64 value);

  plUInt32 GreatestCommonDivisor(plUInt32 a, plUInt32 b);

  plUInt32 ColorFloatToUnsignedInt(float value);
  plUInt8 ColorFloatToByte(float value);
  plUInt16 ColorFloatToShort(float value);
  plInt8 ColorFloatToSignedByte(float value);
  plInt16 ColorFloatToSignedShort(float value);

  float ColorByteToFloat(plUInt8 value);
  float ColorShortToFloat(plUInt16 value);
  float ColorSignedByteToFloat(plInt8 value);
  float ColorSignedShortToFloat(plInt16 value);
  */

  m_pEngine->SetDefaultNamespace("");
}

//////////////////////////////////////////////////////////////////////////
// plAngle
//////////////////////////////////////////////////////////////////////////

static int plAngle_opCmp(const plAngle& lhs, const plAngle& rhs)
{
  if (lhs < rhs)
    return -1;
  if (rhs < lhs)
    return +1;

  return 0;
}

static void plAngle_MakeZero_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plAngle::MakeZero());
}

static void plAngle_MakeFromDegree_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plAngle::MakeFromDegree(pGen->GetArgFloat(0)));
}

static void plAngle_MakeFromRadian_Generic(asIScriptGeneric* pGen)
{
  const float fRadians = pGen->GetArgFloat(0);

  SetReturnObject(pGen, plAngle::MakeFromRadian(fRadians));
}

static void plAngle_AngleBetween_Generic(asIScriptGeneric* pGen)
{
  const plAngle& a1 = *static_cast<plAngle*>(pGen->GetArgObject(0));
  const plAngle& a2 = *static_cast<plAngle*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plAngle::AngleBetween(a1, a2));
}

void plAngelScriptEngineSingleton::Register_Angle()
{
  // static functions
  {
    m_pEngine->SetDefaultNamespace("plAngle");

    AS_CHECK(m_pEngine->RegisterGlobalFunction("float DegToRad(float fDegree)", asFUNCTION(plAngle::DegToRad<float>), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("float RadToDeg(float fRadians)", asFUNCTION(plAngle::RadToDeg<float>), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plAngle MakeZero()", asFUNCTION(plAngle_MakeZero_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plAngle MakeFromDegree(float fDegree)", asFUNCTION(plAngle_MakeFromDegree_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plAngle MakeFromRadian(float fRadians)", asFUNCTION(plAngle_MakeFromRadian_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plAngle AngleBetween(plAngle a1, plAngle a2)", asFUNCTION(plAngle_AngleBetween_Generic), asCALL_GENERIC));

    m_pEngine->SetDefaultNamespace("");
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "float GetDegree() const", asMETHOD(plAngle, GetDegree), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "float GetRadian() const", asMETHOD(plAngle, GetRadian), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "void SetRadian(float fRadians)", asMETHOD(plAngle, SetRadian), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "void NormalizeRange()", asMETHOD(plAngle, NormalizeRange), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "plAngle GetNormalizedRange() const", asMETHOD(plAngle, GetNormalizedRange), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "bool IsEqualSimple(plAngle rhs, plAngle epsilon) const", asMETHOD(plAngle, IsEqualSimple), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "bool IsEqualNormalized(plAngle rhs, plAngle epsilon) const", asMETHOD(plAngle, IsEqualNormalized), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "plAngle opNeg() const", asMETHODPR(plAngle, operator-, () const, plAngle), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "plAngle opAdd(plAngle) const", asMETHODPR(plAngle, operator+, (plAngle) const, plAngle), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "plAngle opSub(plAngle) const", asMETHODPR(plAngle, operator-, (plAngle) const, plAngle), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "void opAddAssign(plAngle)", asMETHOD(plAngle, operator+=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "void opSubAssign(plAngle)", asMETHOD(plAngle, operator-=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "bool opEquals(const plAngle& in) const", asMETHODPR(plAngle, operator==, (const plAngle&) const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "int opCmp(const plAngle& in) const", asFUNCTION(plAngle_opCmp), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "plAngle opMul(float) const", asFUNCTIONPR(operator*, (const plAngle&, float), plAngle), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "plAngle opMul_r(float) const", asFUNCTIONPR(operator*, (const plAngle&, float), plAngle), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "plAngle opDiv(float) const", asFUNCTIONPR(operator/, (const plAngle&, float), plAngle), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plAngle", "float opDiv(const plAngle& in) const", asFUNCTIONPR(operator/, (const plAngle&, const plAngle&), float), asCALL_CDECL_OBJFIRST));
}


//////////////////////////////////////////////////////////////////////////
// plVec2
//////////////////////////////////////////////////////////////////////////

static int plVec2_opCmp(const plVec2& lhs, const plVec2& rhs)
{
  if (lhs < rhs)
    return -1;
  if (rhs < lhs)
    return +1;

  return 0;
}

static void plVec2_Construct1(void* pMemory, float fXyz)
{
  new (pMemory) plVec2(fXyz);
}

static void plVec2_Construct2(void* pMemory, float x, float y)
{
  new (pMemory) plVec2(x, y);
}

static plVec2 plVec2_MakeNaN()
{
  return plVec2(plMath::NaN<float>());
}

static void plVec2_MakeNaN_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec2_MakeNaN());
}

static void plVec2_MakeZero_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec2::MakeZero());
}

void plAngelScriptEngineSingleton::Register_Vec2()
{
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec2", "float x", asOFFSET(plVec2, x)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec2", "float y", asOFFSET(plVec2, y)));

  // static functions
  {
    m_pEngine->SetDefaultNamespace("plVec2");
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec2 MakeNaN()", asFUNCTION(plVec2_MakeNaN_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec2 MakeZero()", asFUNCTION(plVec2_MakeZero_Generic), asCALL_GENERIC));
    m_pEngine->SetDefaultNamespace("");
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec3 GetAsVec3(float z) const", asMETHOD(plVec2, GetAsVec3), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec4 GetAsVec4(float z, float w) const", asMETHOD(plVec2, GetAsVec4), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void Set(float xyz)", asMETHODPR(plVec2, Set, (float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void Set(float x, float y)", asMETHODPR(plVec2, Set, (float, float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void SetZero()", asMETHOD(plVec2, SetZero), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "float GetLength() const", asMETHODPR(plVec2, GetLength, () const, float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "float GetDistanceTo(const plVec2& in rhs) const", asMETHODPR(plVec2, GetDistanceTo, (const plVec2&) const, float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "float GetSquaredDistanceTo(const plVec2& in rhs) const", asMETHODPR(plVec2, GetSquaredDistanceTo, (const plVec2&) const, float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "float GetLengthSquared() const", asMETHOD(plVec2, GetLengthSquared), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "float GetLengthAndNormalize()", asMETHODPR(plVec2, GetLengthAndNormalize, (), float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 GetNormalized() const", asMETHODPR(plVec2, GetNormalized, () const, const plVec2), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void Normalize()", asMETHODPR(plVec2, Normalize, (), void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "bool IsZero() const", asMETHODPR(plVec2, IsZero, () const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "bool IsZero(float fEpsilon) const", asMETHODPR(plVec2, IsZero, (float) const, bool), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "bool IsNormalized(float fEpsilon = 0.001f) const", asMETHODPR(plVec2, IsNormalized, (float) const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "bool IsNaN() const", asMETHOD(plVec2, IsNaN), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "bool IsValid() const", asMETHOD(plVec2, IsValid), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void opAddAssign(const plVec2& in)", asMETHOD(plVec2, operator+=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void opSubAssign(const plVec2& in)", asMETHOD(plVec2, operator-=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void opMulAssign(float)", asMETHODPR(plVec2, operator*=, (float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void opDivAssign(float)", asMETHODPR(plVec2, operator/=, (float), void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "bool IsIdentical(const plVec2& in) const", asMETHOD(plVec2, IsIdentical), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "bool IsEqual(const plVec2& in, float fEpsilon) const", asMETHOD(plVec2, IsEqual), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "float Dot(const plVec2& in) const", asMETHOD(plVec2, Dot), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 CompMin(const plVec2& in rhs) const", asMETHOD(plVec2, CompMin), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 CompMax(const plVec2& in rhs) const", asMETHOD(plVec2, CompMax), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 CompClamp(const plVec2& in rhs) const", asMETHOD(plVec2, CompClamp), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 CompMul(const plVec2& in rhs) const", asMETHOD(plVec2, CompMul), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 CompDiv(const plVec2& in rhs) const", asMETHOD(plVec2, CompDiv), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 Abs() const", asMETHOD(plVec2, Abs), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "void MakeOrthogonalTo(const plVec2& in)", asMETHODPR(plVec2, MakeOrthogonalTo, (const plVec2&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 GetOrthogonalVector() const", asMETHOD(plVec2, GetOrthogonalVector), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 GetReflectedVector(const plVec2& in) const", asMETHODPR(plVec2, GetReflectedVector, (const plVec2&) const, const plVec2), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 opNeg() const", asMETHODPR(plVec2, operator-, () const, const plVec2), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 opAdd(const plVec2& in) const", asFUNCTIONPR(operator+, (const plVec2&, const plVec2&), const plVec2), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 opSub(const plVec2& in) const", asFUNCTIONPR(operator-, (const plVec2&, const plVec2&), const plVec2), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 opMul(float) const", asFUNCTIONPR(operator*, (const plVec2&, float), const plVec2), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 opMul_r(float) const", asFUNCTIONPR(operator*, (float, const plVec2&), const plVec2), asCALL_CDECL_OBJLAST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "plVec2 opDiv(float) const", asFUNCTIONPR(operator/, (const plVec2&, float), const plVec2), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "bool opEquals(const plVec2& in) const", asFUNCTIONPR(operator==, (const plVec2&, const plVec2&), bool), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec2", "int opCmp(const plVec2& in) const", asFUNCTIONPR(plVec2_opCmp, (const plVec2&, const plVec2&), int), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plVec2", asBEHAVE_CONSTRUCT, "void f(float x, float y)", asFUNCTION(plVec2_Construct2), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plVec2", asBEHAVE_CONSTRUCT, "void f(float xyz)", asFUNCTION(plVec2_Construct1), asCALL_CDECL_OBJFIRST));
}


//////////////////////////////////////////////////////////////////////////
// plVec3
//////////////////////////////////////////////////////////////////////////

static int plVec3_opCmp(const plVec3& lhs, const plVec3& rhs)
{
  if (lhs < rhs)
    return -1;
  if (rhs < lhs)
    return +1;

  return 0;
}

static void plVec3_Construct1(void* pMemory, float fXyz)
{
  new (pMemory) plVec3(fXyz);
}
static void plVec3_Construct3(void* pMemory, float x, float y, float z)
{
  new (pMemory) plVec3(x, y, z);
}

static plVec3 plVec3_MakeNaN()
{
  return plVec3(plMath::NaN<float>());
}

static void plVec3_MakeNaN_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec3_MakeNaN());
}

static void plVec3_MakeZero_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec3::MakeZero());
}

static void plVec3_MakeAxisX_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec3::MakeAxisX());
}

static void plVec3_MakeAxisY_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec3::MakeAxisY());
}

static void plVec3_MakeAxisZ_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec3::MakeAxisZ());
}

static void plVec3_Make_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec3::Make(pGen->GetArgFloat(0), pGen->GetArgFloat(1), pGen->GetArgFloat(2)));
}

static void plVec3_MakeRandomDirection_Generic(asIScriptGeneric* pGen)
{
  plRandom& rng = *static_cast<plRandom*>(pGen->GetArgObject(0));

  SetReturnObject(pGen, plVec3::MakeRandomDirection<float>(rng));
}

static void plVec3_MakeRandomPointInSphere_Generic(asIScriptGeneric* pGen)
{
  plRandom& rng = *static_cast<plRandom*>(pGen->GetArgObject(0));

  SetReturnObject(pGen, plVec3::MakeRandomPointInSphere<float>(rng));
}

static void plVec3_MakeRandomDeviationX_Generic(asIScriptGeneric* pGen)
{
  plRandom& rng = *static_cast<plRandom*>(pGen->GetArgObject(0));
  const plAngle& maxDeviation = *static_cast<plAngle*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plVec3::MakeRandomDeviationX<float>(rng, maxDeviation));
}

static void plVec3_MakeRandomDeviationY_Generic(asIScriptGeneric* pGen)
{
  plRandom& rng = *static_cast<plRandom*>(pGen->GetArgObject(0));
  const plAngle& maxDeviation = *static_cast<plAngle*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plVec3::MakeRandomDeviationY<float>(rng, maxDeviation));
}

static void plVec3_MakeRandomDeviationZ_Generic(asIScriptGeneric* pGen)
{
  plRandom& rng = *static_cast<plRandom*>(pGen->GetArgObject(0));
  const plAngle& maxDeviation = *static_cast<plAngle*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plVec3::MakeRandomDeviationZ<float>(rng, maxDeviation));
}

static void plVec3_MakeRandomDeviation_Generic(asIScriptGeneric* pGen)
{
  plRandom& rng = *static_cast<plRandom*>(pGen->GetArgObject(0));
  const plAngle& maxDeviation = *static_cast<plAngle*>(pGen->GetArgObject(1));
  const plVec3& normal = *static_cast<plVec3*>(pGen->GetArgObject(2));

  SetReturnObject(pGen, plVec3::MakeRandomDeviation<float>(rng, maxDeviation, normal));
}

void plAngelScriptEngineSingleton::Register_Vec3()
{
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec3", "float x", asOFFSET(plVec3, x)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec3", "float y", asOFFSET(plVec3, y)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec3", "float z", asOFFSET(plVec3, z)));

  // static functions
  {
    m_pEngine->SetDefaultNamespace("plVec3");

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeNaN()", asFUNCTION(plVec3_MakeNaN_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeZero()", asFUNCTION(plVec3_MakeZero_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeAxisX()", asFUNCTION(plVec3_MakeAxisX_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeAxisY()", asFUNCTION(plVec3_MakeAxisY_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeAxisZ()", asFUNCTION(plVec3_MakeAxisZ_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 Make(float x, float y, float z)", asFUNCTION(plVec3_Make_Generic), asCALL_GENERIC));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeRandomDirection(plRandom& inout rng)", asFUNCTION(plVec3_MakeRandomDirection_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeRandomPointInSphere(plRandom& inout rng)", asFUNCTION(plVec3_MakeRandomPointInSphere_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeRandomDeviationX(plRandom& inout rng, const plAngle& in maxDeviation)", asFUNCTION(plVec3_MakeRandomDeviationX_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeRandomDeviationY(plRandom& inout rng, const plAngle& in maxDeviation)", asFUNCTION(plVec3_MakeRandomDeviationY_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeRandomDeviationZ(plRandom& inout rng, const plAngle& in maxDeviation)", asFUNCTION(plVec3_MakeRandomDeviationZ_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec3 MakeRandomDeviation(plRandom& inout rng, const plAngle& in maxDeviation, const plVec3& in normal)", asFUNCTION(plVec3_MakeRandomDeviation_Generic), asCALL_GENERIC));

    m_pEngine->SetDefaultNamespace("");
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec2 GetAsVec2() const", asMETHOD(plVec3, GetAsVec2), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec4 GetAsVec4(float w) const", asMETHOD(plVec3, GetAsVec4), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec4 GetAsPositionVec4() const", asMETHOD(plVec3, GetAsPositionVec4), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec4 GetAsDirectionVec4() const", asMETHOD(plVec3, GetAsDirectionVec4), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void Set(float xyz)", asMETHODPR(plVec3, Set, (float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void Set(float x, float y, float z)", asMETHODPR(plVec3, Set, (float, float, float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void SetZero()", asMETHOD(plVec3, SetZero), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "float GetLength() const", asMETHODPR(plVec3, GetLength, () const, float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "float GetDistanceTo(const plVec3& in rhs) const", asMETHODPR(plVec3, GetDistanceTo, (const plVec3&) const, float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "float GetSquaredDistanceTo(const plVec3& in rhs) const", asMETHODPR(plVec3, GetSquaredDistanceTo, (const plVec3&) const, float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "float GetLengthSquared() const", asMETHOD(plVec3, GetLengthSquared), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "float GetLengthAndNormalize()", asMETHODPR(plVec3, GetLengthAndNormalize, (), float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 GetNormalized() const", asMETHODPR(plVec3, GetNormalized, () const, const plVec3), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void Normalize()", asMETHODPR(plVec3, Normalize, (), void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "bool IsZero() const", asMETHODPR(plVec3, IsZero, () const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "bool IsZero(float fEpsilon) const", asMETHODPR(plVec3, IsZero, (float) const, bool), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "bool IsNormalized(float fEpsilon = 0.001f) const", asMETHODPR(plVec3, IsNormalized, (float) const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "bool IsNaN() const", asMETHOD(plVec3, IsNaN), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "bool IsValid() const", asMETHOD(plVec3, IsValid), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void opAddAssign(const plVec3& in)", asMETHOD(plVec3, operator+=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void opSubAssign(const plVec3& in)", asMETHOD(plVec3, operator-=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void opMulAssign(const plVec3& in)", asMETHODPR(plVec3, operator*=, (const plVec3&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void opDivAssign(const plVec3& in)", asMETHODPR(plVec3, operator/=, (const plVec3&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void opMulAssign(float)", asMETHODPR(plVec3, operator*=, (float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void opDivAssign(float)", asMETHODPR(plVec3, operator/=, (float), void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "bool IsIdentical(const plVec3& in) const", asMETHOD(plVec3, IsIdentical), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "bool IsEqual(const plVec3& in, float fEpsilon) const", asMETHOD(plVec3, IsEqual), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "float Dot(const plVec3& in) const", asMETHOD(plVec3, Dot), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 CrossRH(const plVec3& in) const", asMETHOD(plVec3, CrossRH), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 CompMin(const plVec3& in) const", asMETHOD(plVec3, CompMin), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 CompMax(const plVec3& in) const", asMETHOD(plVec3, CompMax), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 CompClamp(const plVec3& in) const", asMETHOD(plVec3, CompClamp), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 CompMul(const plVec3& in) const", asMETHOD(plVec3, CompMul), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 CompDiv(const plVec3& in) const", asMETHOD(plVec3, CompDiv), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 Abs() const", asMETHOD(plVec3, Abs), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "void MakeOrthogonalTo(const plVec3& in)", asMETHODPR(plVec3, MakeOrthogonalTo, (const plVec3&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 GetOrthogonalVector() const", asMETHODPR(plVec3, GetOrthogonalVector, () const, const plVec3), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 GetReflectedVector(const plVec3& in) const", asMETHODPR(plVec3, GetReflectedVector, (const plVec3&) const, const plVec3), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 opNeg() const", asMETHODPR(plVec3, operator-, () const, const plVec3), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 opAdd(const plVec3& in) const", asFUNCTIONPR(operator+, (const plVec3&, const plVec3&), const plVec3), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 opSub(const plVec3& in) const", asFUNCTIONPR(operator-, (const plVec3&, const plVec3&), const plVec3), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 opMul(float) const", asFUNCTIONPR(operator*, (const plVec3&, float), const plVec3), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 opMul_r(float) const", asFUNCTIONPR(operator*, (float, const plVec3&), const plVec3), asCALL_CDECL_OBJLAST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "plVec3 opDiv(float) const", asFUNCTIONPR(operator/, (const plVec3&, float), const plVec3), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "bool opEquals(const plVec3& in) const", asFUNCTIONPR(operator==, (const plVec3&, const plVec3&), bool), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec3", "int opCmp(const plVec3& in) const", asFUNCTIONPR(plVec3_opCmp, (const plVec3&, const plVec3&), int), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plVec3", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z)", asFUNCTION(plVec3_Construct3), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plVec3", asBEHAVE_CONSTRUCT, "void f(float xyz)", asFUNCTION(plVec3_Construct1), asCALL_CDECL_OBJFIRST));
}


//////////////////////////////////////////////////////////////////////////
// plVec4
//////////////////////////////////////////////////////////////////////////

static int plVec4_opCmp(const plVec4& lhs, const plVec4& rhs)
{
  if (lhs < rhs)
    return -1;
  if (rhs < lhs)
    return +1;

  return 0;
}

static void plVec4_Construct1(void* pMemory, float fXyzw)
{
  new (pMemory) plVec4(fXyzw);
}
static void plVec4_Construct4(void* pMemory, float x, float y, float z, float w)
{
  new (pMemory) plVec4(x, y, z, w);
}

static plVec4 plVec4_MakeNaN()
{
  return plVec4(plMath::NaN<float>());
}

static void plVec4_MakeNaN_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec4_MakeNaN());
}

static void plVec4_MakeZero_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plVec4::MakeZero());
}

void plAngelScriptEngineSingleton::Register_Vec4()
{
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec4", "float x", asOFFSET(plVec4, x)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec4", "float y", asOFFSET(plVec4, y)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec4", "float z", asOFFSET(plVec4, z)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plVec4", "float w", asOFFSET(plVec4, w)));

  // static functions
  {
    m_pEngine->SetDefaultNamespace("plVec4");
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec4 MakeNaN()", asFUNCTION(plVec4_MakeNaN_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plVec4 MakeZero()", asFUNCTION(plVec4_MakeZero_Generic), asCALL_GENERIC));
    m_pEngine->SetDefaultNamespace("");
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec2 GetAsVec2() const", asMETHOD(plVec4, GetAsVec2), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec3 GetAsVec3() const", asMETHOD(plVec4, GetAsVec3), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "void Set(float xyzw)", asMETHODPR(plVec4, Set, (float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "void Set(float x, float y, float z, float w)", asMETHODPR(plVec4, Set, (float, float, float, float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "void SetZero()", asMETHOD(plVec4, SetZero), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "float GetLength() const", asMETHODPR(plVec4, GetLength, () const, float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "float GetLengthSquared() const", asMETHOD(plVec4, GetLengthSquared), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "float GetLengthAndNormalize()", asMETHODPR(plVec4, GetLengthAndNormalize, (), float), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 GetNormalized() const", asMETHODPR(plVec4, GetNormalized, () const, const plVec4), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "void Normalize()", asMETHODPR(plVec4, Normalize, (), void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "bool IsZero() const", asMETHODPR(plVec4, IsZero, () const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "bool IsZero(float fEpsilon) const", asMETHODPR(plVec4, IsZero, (float) const, bool), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "bool IsNormalized(float fEpsilon = 0.001f) const", asMETHODPR(plVec4, IsNormalized, (float) const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "bool IsNaN() const", asMETHOD(plVec4, IsNaN), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "bool IsValid() const", asMETHOD(plVec4, IsValid), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "void opAddAssign(const plVec4& in)", asMETHOD(plVec4, operator+=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "void opSubAssign(const plVec4& in)", asMETHOD(plVec4, operator-=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "void opMulAssign(float)", asMETHODPR(plVec4, operator*=, (float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "void opDivAssign(float)", asMETHODPR(plVec4, operator/=, (float), void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "bool IsIdentical(const plVec4& in) const", asMETHOD(plVec4, IsIdentical), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "bool IsEqual(const plVec4& in, float) const", asMETHOD(plVec4, IsEqual), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "float Dot(const plVec4& in) const", asMETHOD(plVec4, Dot), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 CompMin(const plVec4& in) const", asMETHOD(plVec4, CompMin), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 CompMax(const plVec4& in) const", asMETHOD(plVec4, CompMax), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 CompClamp(const plVec4& in) const", asMETHOD(plVec4, CompClamp), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 CompMul(const plVec4& in) const", asMETHOD(plVec4, CompMul), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 CompDiv(const plVec4& in) const", asMETHOD(plVec4, CompDiv), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 Abs() const", asMETHOD(plVec4, Abs), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 opNeg() const", asMETHODPR(plVec4, operator-, () const, const plVec4), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 opAdd(const plVec4& in) const", asFUNCTIONPR(operator+, (const plVec4&, const plVec4&), const plVec4), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 opSub(const plVec4& in) const", asFUNCTIONPR(operator-, (const plVec4&, const plVec4&), const plVec4), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 opMul(float) const", asFUNCTIONPR(operator*, (const plVec4&, float), const plVec4), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 opMul_r(float) const", asFUNCTIONPR(operator*, (float, const plVec4&), const plVec4), asCALL_CDECL_OBJLAST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "plVec4 opDiv(float) const", asFUNCTIONPR(operator/, (const plVec4&, float), const plVec4), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "bool opEquals(const plVec4& in) const", asFUNCTIONPR(operator==, (const plVec4&, const plVec4&), bool), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plVec4", "int opCmp(const plVec4& in) const", asFUNCTIONPR(plVec4_opCmp, (const plVec4&, const plVec4&), int), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plVec4", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z, float w)", asFUNCTION(plVec4_Construct4), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plVec4", asBEHAVE_CONSTRUCT, "void f(float xyzw)", asFUNCTION(plVec4_Construct1), asCALL_CDECL_OBJFIRST));
}

//////////////////////////////////////////////////////////////////////////
// plQuat
//////////////////////////////////////////////////////////////////////////

static void plQuat_MakeIdentity_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plQuat::MakeIdentity());
}

static void plQuat_MakeFromElements_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plQuat::MakeFromElements(pGen->GetArgFloat(0), pGen->GetArgFloat(1), pGen->GetArgFloat(2), pGen->GetArgFloat(3)));
}

static void plQuat_MakeFromAxisAndAngle_Generic(asIScriptGeneric* pGen)
{
  const plVec3& vAxis = *static_cast<plVec3*>(pGen->GetArgObject(0));
  const plAngle& angle = *static_cast<plAngle*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plQuat::MakeFromAxisAndAngle(vAxis, angle));
}

static void plQuat_MakeShortestRotation_Generic(asIScriptGeneric* pGen)
{
  const plVec3& vFrom = *static_cast<plVec3*>(pGen->GetArgObject(0));
  const plVec3& vTo = *static_cast<plVec3*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plQuat::MakeShortestRotation(vFrom, vTo));
}

static void plQuat_MakeFromMat3_Generic(asIScriptGeneric* pGen)
{
  const plMat3& m = *static_cast<plMat3*>(pGen->GetArgObject(0));

  SetReturnObject(pGen, plQuat::MakeFromMat3(m));
}

static void plQuat_MakeSlerp_Generic(asIScriptGeneric* pGen)
{
  const plQuat& qFrom = *static_cast<plQuat*>(pGen->GetArgObject(0));
  const plQuat& qTo = *static_cast<plQuat*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plQuat::MakeSlerp(qFrom, qTo, pGen->GetArgFloat(2)));
}

static void plQuat_MakeFromEulerAngles_Generic(asIScriptGeneric* pGen)
{
  const plAngle& x = *static_cast<plAngle*>(pGen->GetArgObject(0));
  const plAngle& y = *static_cast<plAngle*>(pGen->GetArgObject(1));
  const plAngle& z = *static_cast<plAngle*>(pGen->GetArgObject(2));

  SetReturnObject(pGen, plQuat::MakeFromEulerAngles(x, y, z));
}

void plAngelScriptEngineSingleton::Register_Quat()
{
  AS_CHECK(m_pEngine->RegisterObjectProperty("plQuat", "float x", asOFFSET(plQuat, x)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plQuat", "float y", asOFFSET(plQuat, y)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plQuat", "float z", asOFFSET(plQuat, z)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plQuat", "float w", asOFFSET(plQuat, w)));

  // static functions
  {
    m_pEngine->SetDefaultNamespace("plQuat");

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plQuat MakeIdentity()", asFUNCTION(plQuat_MakeIdentity_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plQuat MakeFromElements(float x, float y, float z, float w)", asFUNCTION(plQuat_MakeFromElements_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plQuat MakeFromAxisAndAngle(const plVec3& in vAxis , plAngle angle)", asFUNCTION(plQuat_MakeFromAxisAndAngle_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plQuat MakeShortestRotation(const plVec3& in vFrom, const plVec3& in vTo)", asFUNCTION(plQuat_MakeShortestRotation_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plQuat MakeFromMat3(const plMat3& in)", asFUNCTION(plQuat_MakeFromMat3_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plQuat MakeSlerp(const plQuat& in qFrom, const plQuat& in qTo, float fFactor)", asFUNCTION(plQuat_MakeSlerp_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plQuat MakeFromEulerAngles(plAngle x, plAngle y, plAngle z)", asFUNCTION(plQuat_MakeFromEulerAngles_Generic), asCALL_GENERIC));

    m_pEngine->SetDefaultNamespace("");
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "void SetIdentity()", asMETHOD(plQuat, SetIdentity), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "void ReconstructFromMat3(const plMat3& in)", asMETHOD(plQuat, ReconstructFromMat3), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "void ReconstructFromMat4(const plMat4& in)", asMETHOD(plQuat, ReconstructFromMat4), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "void Normalize()", asMETHOD(plQuat, Normalize), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "void GetRotationAxisAndAngle(plVec3& out, plAngle& out, float fEpsilon = 0.00001) const", asMETHOD(plQuat, GetRotationAxisAndAngle), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "plVec3 GetVectorPart() const", asMETHOD(plQuat, GetVectorPart), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "plMat3 GetAsMat3() const", asMETHOD(plQuat, GetAsMat3), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "plMat4 GetAsMat4() const", asMETHOD(plQuat, GetAsMat4), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "bool IsValid(float fEpsilon = 0.00001) const", asMETHOD(plQuat, IsValid), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "bool IsNaN() const", asMETHOD(plQuat, IsNaN), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "bool IsEqualRotation(const plQuat& in, float fEpsilon = 0.00001) const", asMETHOD(plQuat, IsEqualRotation), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "void Invert()", asMETHOD(plQuat, Invert), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "plQuat GetInverse() const", asMETHOD(plQuat, GetInverse), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "plQuat GetNegated() const", asMETHOD(plQuat, GetNegated), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "float Dot(const plQuat& in) const", asMETHOD(plQuat, Dot), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "plVec3 Rotate(const plVec3& in) const", asMETHOD(plQuat, Rotate), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "void GetAsEulerAngles(float& out, float& out, float& out) const", asMETHOD(plQuat, GetAsEulerAngles), asCALL_THISCALL));


  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "plQuat opMul(const plQuat& in) const", asFUNCTIONPR(operator*, (const plQuat&, const plQuat&), const plQuat), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "plVec3 opMul(const plVec3& in) const", asFUNCTIONPR(operator*, (const plQuat&, const plVec3&), const plVec3), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plQuat", "bool opEquals(const plQuat& in) const", asFUNCTIONPR(operator==, (const plQuat&, const plQuat&), bool), asCALL_CDECL_OBJFIRST));
}


//////////////////////////////////////////////////////////////////////////
// plTransform
//////////////////////////////////////////////////////////////////////////

static void plTransform_Construct3(void* pMemory, const plVec3& v, const plQuat& r, const plVec3& s)
{
  new (pMemory) plTransform(v, r, s);
}

static void plTransform_Make_Generic(asIScriptGeneric* pGen)
{
  const plVec3& vPosition = *static_cast<plVec3*>(pGen->GetArgObject(0));
  const plQuat& qRotation = *static_cast<plQuat*>(pGen->GetArgObject(1));
  const plVec3& vScale = *static_cast<plVec3*>(pGen->GetArgObject(2));

  SetReturnObject(pGen, plTransform::Make(vPosition, qRotation, vScale));
}

static void plTransform_MakeIdentity_Generic(asIScriptGeneric* pGen)
{
  SetReturnObject(pGen, plTransform::MakeIdentity());
}

static void plTransform_MakeFromMat4_Generic(asIScriptGeneric* pGen)
{
  const plMat4& m = *static_cast<plMat4*>(pGen->GetArgObject(0));

  SetReturnObject(pGen, plTransform::MakeFromMat4(m));
}

static void plTransform_MakeLocalTransform_Generic(asIScriptGeneric* pGen)
{
  const plTransform& globalTransformParent = *static_cast<plTransform*>(pGen->GetArgObject(0));
  const plTransform& globalTransformChild = *static_cast<plTransform*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plTransform::MakeLocalTransform(globalTransformParent, globalTransformChild));
}

static void plTransform_MakeGlobalTransform_Generic(asIScriptGeneric* pGen)
{
  const plTransform& globalTransformParent = *static_cast<plTransform*>(pGen->GetArgObject(0));
  const plTransform& localTransformChild = *static_cast<plTransform*>(pGen->GetArgObject(1));

  SetReturnObject(pGen, plTransform::MakeGlobalTransform(globalTransformParent, localTransformChild));
}

void plAngelScriptEngineSingleton::Register_Transform()
{
  AS_CHECK(m_pEngine->RegisterObjectProperty("plTransform", "plVec3 m_vPosition", asOFFSET(plTransform, m_vPosition)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plTransform", "plQuat m_qRotation", asOFFSET(plTransform, m_qRotation)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plTransform", "plVec3 m_vScale", asOFFSET(plTransform, m_vScale)));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plTransform", asBEHAVE_CONSTRUCT, "void f(const plVec3& in vPosition, const plQuat& in qRotation = plQuat::MakeIdentity(), const plVec3& in vScale = plVec3(1))", asFUNCTION(plTransform_Construct3), asCALL_CDECL_OBJFIRST));

  // static functions
  {
    m_pEngine->SetDefaultNamespace("plTransform");

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTransform Make(const plVec3& in vPosition, const plQuat& in qRotation = plQuat::MakeIdentity(), const plVec3& in vScale = plVec3(1))", asFUNCTION(plTransform_Make_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTransform MakeIdentity()", asFUNCTION(plTransform_MakeIdentity_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTransform MakeFromMat4(const plMat4& in)", asFUNCTION(plTransform_MakeFromMat4_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTransform MakeLocalTransform(const plTransform& in, const plTransform& in)", asFUNCTION(plTransform_MakeLocalTransform_Generic), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTransform MakeGlobalTransform(const plTransform& in, const plTransform& in)", asFUNCTION(plTransform_MakeGlobalTransform_Generic), asCALL_GENERIC));

    m_pEngine->SetDefaultNamespace("");
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "void SetIdentity()", asMETHOD(plTransform, SetIdentity), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "float GetMaxScale() const", asMETHOD(plTransform, GetMaxScale), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "bool HasMirrorScaling() const", asMETHOD(plTransform, HasMirrorScaling), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "bool ContainsUniformScale() const", asMETHOD(plTransform, ContainsUniformScale), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "bool IsValid() const", asMETHOD(plTransform, IsValid), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "bool IsIdentical(const plTransform& in) const", asMETHOD(plTransform, IsIdentical), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "bool IsEqual(const plTransform& in, float fEpsilon) const", asMETHOD(plTransform, IsEqual), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "void Invert()", asMETHOD(plTransform, Invert), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plTransform GetInverse() const", asMETHOD(plTransform, GetInverse), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plVec3 TransformPosition(const plVec3& in vPosition) const", asMETHOD(plTransform, TransformPosition), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plVec3 TransformDirection(const plVec3& in vDirection) const", asMETHOD(plTransform, TransformDirection), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "void opAddAssign(const plVec3& in)", asMETHOD(plTransform, operator+=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "void opSubAssign(const plVec3& in)", asMETHOD(plTransform, operator-=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plMat4 GetAsMat4() const", asMETHOD(plTransform, GetAsMat4), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plVec3 opMul(const plVec3& in) const", asFUNCTIONPR(operator*, (const plTransform&, const plVec3&), const plVec3), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plTransform opMul_r(const plQuat& in qRotation) const", asFUNCTIONPR(operator*, (const plQuat&, const plTransform&), const plTransform), asCALL_CDECL_OBJLAST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plTransform opMul(const plQuat& in qRotation) const", asFUNCTIONPR(operator*, (const plTransform&, const plQuat&), const plTransform), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plTransform opAdd(const plVec3& in) const", asFUNCTIONPR(operator+, (const plTransform&, const plVec3&), const plTransform), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plTransform opSub(const plVec3& in) const", asFUNCTIONPR(operator-, (const plTransform&, const plVec3&), const plTransform), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "plTransform opMul(const plTransform& in) const", asFUNCTIONPR(operator*, (const plTransform&, const plTransform&), const plTransform), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTransform", "bool opEquals(const plTransform& in) const", asFUNCTIONPR(operator==, (const plTransform&, const plTransform&), bool), asCALL_CDECL_OBJFIRST));
}

//////////////////////////////////////////////////////////////////////////
// plMat3
//////////////////////////////////////////////////////////////////////////

void plAngelScriptEngineSingleton::Register_Mat3()
{
  // static functions
  {
    m_pEngine->SetDefaultNamespace("plMat3");
    m_pEngine->SetDefaultNamespace("");
  }

  // TODO AngelScript: Register plMat3
}

//////////////////////////////////////////////////////////////////////////
// plMat4
//////////////////////////////////////////////////////////////////////////

void plAngelScriptEngineSingleton::Register_Mat4()
{
  // static functions
  {
    m_pEngine->SetDefaultNamespace("plMat4");
    m_pEngine->SetDefaultNamespace("");
  }

  // TODO AngelScript: Register plMat4
}
