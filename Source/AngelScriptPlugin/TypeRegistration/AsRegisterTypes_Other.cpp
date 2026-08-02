#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/World/SpatialData.h>
#include <Foundation/Math/Random.h>
#include <Foundation/Time/Clock.h>

//////////////////////////////////////////////////////////////////////////
// plRTTI
//////////////////////////////////////////////////////////////////////////

const plRTTI* plRTTI_GetType(plStringView sName)
{
  return plRTTI::FindTypeByName(sName);
}

static void plRTTI_GetType_Generic(asIScriptGeneric* pGen)
{
  const plStringView& sName = *static_cast<plStringView*>(pGen->GetArgObject(0));
  pGen->SetReturnAddress(const_cast<plRTTI*>(plRTTI_GetType(sName)));
}

void plAngelScriptEngineSingleton::Register_RTTI()
{
  // static functions
  {
    m_pEngine->SetDefaultNamespace("plRTTI");

    AS_CHECK(m_pEngine->RegisterGlobalFunction("const plRTTI@ GetType(const plStringView& in)", asFUNCTION(plRTTI_GetType_Generic), asCALL_GENERIC));

    m_pEngine->SetDefaultNamespace("");
  }
}

//////////////////////////////////////////////////////////////////////////
// plTime
//////////////////////////////////////////////////////////////////////////

static int plTime_opCmp(const plTime& lhs, const plTime& rhs)
{
  if (lhs < rhs)
    return -1;
  if (rhs < lhs)
    return +1;

  return 0;
}

void plAngelScriptEngineSingleton::Register_Time()
{
  // static functions
  {
    m_pEngine->SetDefaultNamespace("plTime");

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime Now()", asFUNCTION(plTime::Now), asCALL_CDECL));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime MakeFromNanoseconds(double fNanoSeconds)", asFUNCTION(plTime::MakeFromNanoseconds), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime Nanoseconds(double fNanoSeconds)", asFUNCTION(plTime::Nanoseconds), asCALL_CDECL));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime MakeFromMicroseconds(double fMicroSeconds)", asFUNCTION(plTime::MakeFromMicroseconds), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime Microseconds(double fMicroSeconds)", asFUNCTION(plTime::Microseconds), asCALL_CDECL));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime MakeFromMilliseconds(double fMilliSeconds)", asFUNCTION(plTime::MakeFromMilliseconds), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime Milliseconds(double fMilliSeconds)", asFUNCTION(plTime::Milliseconds), asCALL_CDECL));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime MakeFromSeconds(double fSeconds)", asFUNCTION(plTime::MakeFromSeconds), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime Seconds(double fSeconds)", asFUNCTION(plTime::Seconds), asCALL_CDECL));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime MakeFromMinutes(double fMinutes)", asFUNCTION(plTime::MakeFromMinutes), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime Minutes(double fMinutes)", asFUNCTION(plTime::Minutes), asCALL_CDECL));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime MakeFromHours(double fHours)", asFUNCTION(plTime::MakeFromHours), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime Hours(double fHours)", asFUNCTION(plTime::Hours), asCALL_CDECL));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plTime MakeZero()", asFUNCTION(plTime::MakeZero), asCALL_CDECL));

    m_pEngine->SetDefaultNamespace("");
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "bool IsZero() const", asMETHOD(plTime, IsZero), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "bool IsNegative() const", asMETHOD(plTime, IsNegative), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "bool IsPositive() const", asMETHOD(plTime, IsPositive), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "bool IsZeroOrNegative() const", asMETHOD(plTime, IsZeroOrNegative), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "bool IsZeroOrPositive() const", asMETHOD(plTime, IsZeroOrPositive), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "float AsFloatInSeconds() const", asMETHOD(plTime, AsFloatInSeconds), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "double GetNanoseconds() const", asMETHOD(plTime, GetNanoseconds), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "double GetMicroseconds() const", asMETHOD(plTime, GetMicroseconds), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "double GetMilliseconds() const", asMETHOD(plTime, GetMilliseconds), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "double GetSeconds() const", asMETHOD(plTime, GetSeconds), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "double GetMinutes() const", asMETHOD(plTime, GetMinutes), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "double GetHours() const", asMETHOD(plTime, GetHours), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "void opSubAssign(const plTime& in)", asMETHOD(plTime, operator-=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "void opAddAssign(const plTime& in)", asMETHOD(plTime, operator+=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "void opMulAssign(double)", asMETHOD(plTime, operator*=), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "void opDivAssign(double)", asMETHOD(plTime, operator/=), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opSub(const plTime& in) const", asMETHODPR(plTime, operator-, (const plTime&) const, plTime), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opAdd(const plTime& in) const", asMETHODPR(plTime, operator+, (const plTime&) const, plTime), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opNeg() const", asMETHODPR(plTime, operator-, () const, plTime), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "int opCmp(const plTime& in) const", asFUNCTIONPR(plTime_opCmp, (const plTime&, const plTime&), int), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "bool opEquals(const plTime& in) const", asMETHODPR(plTime, operator==, (const plTime&) const, bool), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opMul(double) const", asFUNCTIONPR(operator*, (const plTime&, double), plTime), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opMul_r(double) const", asFUNCTIONPR(operator*, (double, const plTime&), plTime), asCALL_CDECL_OBJLAST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opMul(const plTime& in) const", asFUNCTIONPR(operator*, (const plTime&, const plTime&), plTime), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opDiv(double) const", asFUNCTIONPR(operator/, (const plTime&, double), plTime), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opDiv_r(double) const", asFUNCTIONPR(operator/, (double, const plTime&), plTime), asCALL_CDECL_OBJLAST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTime", "plTime opDiv(const plTime& in) const", asFUNCTIONPR(operator/, (const plTime&, const plTime&), plTime), asCALL_CDECL_OBJFIRST));
}

//////////////////////////////////////////////////////////////////////////
// plClock
//////////////////////////////////////////////////////////////////////////

void plAngelScriptEngineSingleton::Register_Clock()
{
  AS_CHECK(m_pEngine->RegisterObjectMethod("plClock", "void SetPaused(bool)", asMETHOD(plClock, SetPaused), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plClock", "bool GetPaused() const", asMETHOD(plClock, GetPaused), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plClock", "plTime GetTimeDiff() const", asMETHOD(plClock, GetTimeDiff), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plClock", "void SetSpeed(double)", asMETHOD(plClock, SetSpeed), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plClock", "double GetSpeed() const", asMETHOD(plClock, GetSpeed), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plClock", "plTime GetAccumulatedTime() const", asMETHOD(plClock, GetAccumulatedTime), asCALL_THISCALL));
}


//////////////////////////////////////////////////////////////////////////
// plRandom
//////////////////////////////////////////////////////////////////////////

void plAngelScriptEngineSingleton::Register_Random()
{
  // Methods
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "uint32 UInt()", asMETHOD(plRandom, UInt), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "uint32 UIntInRange(uint32 uiRange)", asMETHOD(plRandom, UIntInRange), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "int32 IntMinMax(plInt32 iMinValue, plInt32 iMaxValue)", asMETHOD(plRandom, IntMinMax), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "bool Bool()", asMETHOD(plRandom, Bool), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "double DoubleZeroToOneExclusive()", asMETHOD(plRandom, DoubleZeroToOneExclusive), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "double DoubleZeroToOneInclusive()", asMETHOD(plRandom, DoubleZeroToOneInclusive), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "double DoubleMinMax(double fMinValue, double fMaxValue)", asMETHOD(plRandom, DoubleMinMax), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "double DoubleVariance(double fValue, double fVariance)", asMETHOD(plRandom, DoubleVariance), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "double DoubleVarianceAroundZero(double fAbsMaxValue)", asMETHOD(plRandom, DoubleVarianceAroundZero), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "float FloatZeroToOneExclusive()", asMETHOD(plRandom, FloatZeroToOneExclusive), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "float FloatZeroToOneInclusive()", asMETHOD(plRandom, FloatZeroToOneInclusive), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "float FloatMinMax(float fMinValue, float fMaxValue)", asMETHOD(plRandom, FloatMinMax), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "float FloatVariance(float fValue, float fVariance)", asMETHOD(plRandom, FloatVariance), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plRandom", "float FloatVarianceAroundZero(float fAbsMaxValue)", asMETHOD(plRandom, FloatVarianceAroundZero), asCALL_THISCALL));
  }
}

//////////////////////////////////////////////////////////////////////////
// plColor
//////////////////////////////////////////////////////////////////////////

static void plColor_ConstructRGBA(void* pMemory, float r, float g, float b, float a)
{
  new (pMemory) plColor(r, g, b, a);
}

static void plColor_ConstructGamma(void* pMemory, const plColorGammaUB& col)
{
  new (pMemory) plColor(col);
}

void plAngelScriptEngineSingleton::Register_Color()
{
  AS_CHECK(m_pEngine->RegisterObjectProperty("plColor", "float r", asOFFSET(plColor, r)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plColor", "float g", asOFFSET(plColor, g)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plColor", "float b", asOFFSET(plColor, b)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plColor", "float a", asOFFSET(plColor, a)));

  // static functions
  {
    m_pEngine->SetDefaultNamespace("plColor");

    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor AliceBlue", (void*)&plColor::AliceBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor AntiqueWhite", (void*)&plColor::AntiqueWhite));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Aqua", (void*)&plColor::Aqua));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Aquamarine", (void*)&plColor::Aquamarine));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Azure", (void*)&plColor::Azure));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Beige", (void*)&plColor::Beige));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Bisque", (void*)&plColor::Bisque));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Black", (void*)&plColor::Black));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor BlanchedAlmond", (void*)&plColor::BlanchedAlmond));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Blue", (void*)&plColor::Blue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor BlueViolet", (void*)&plColor::BlueViolet));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Brown", (void*)&plColor::Brown));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor BurlyWood", (void*)&plColor::BurlyWood));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor CadetBlue", (void*)&plColor::CadetBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Chartreuse", (void*)&plColor::Chartreuse));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Chocolate", (void*)&plColor::Chocolate));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Coral", (void*)&plColor::Coral));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor CornflowerBlue", (void*)&plColor::CornflowerBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Cornsilk", (void*)&plColor::Cornsilk));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Crimson", (void*)&plColor::Crimson));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Cyan", (void*)&plColor::Cyan));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkBlue", (void*)&plColor::DarkBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkCyan", (void*)&plColor::DarkCyan));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkGoldenRod", (void*)&plColor::DarkGoldenRod));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkGray", (void*)&plColor::DarkGray));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkGrey", (void*)&plColor::DarkGrey));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkGreen", (void*)&plColor::DarkGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkKhaki", (void*)&plColor::DarkKhaki));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkMagenta", (void*)&plColor::DarkMagenta));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkOliveGreen", (void*)&plColor::DarkOliveGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkOrange", (void*)&plColor::DarkOrange));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkOrchid", (void*)&plColor::DarkOrchid));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkRed", (void*)&plColor::DarkRed));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkSalmon", (void*)&plColor::DarkSalmon));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkSeaGreen", (void*)&plColor::DarkSeaGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkSlateBlue", (void*)&plColor::DarkSlateBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkSlateGray", (void*)&plColor::DarkSlateGray));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkSlateGrey", (void*)&plColor::DarkSlateGrey));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkTurquoise", (void*)&plColor::DarkTurquoise));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DarkViolet", (void*)&plColor::DarkViolet));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DeepPink", (void*)&plColor::DeepPink));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DeepSkyBlue", (void*)&plColor::DeepSkyBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DimGray", (void*)&plColor::DimGray));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DimGrey", (void*)&plColor::DimGrey));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor DodgerBlue", (void*)&plColor::DodgerBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor FireBrick", (void*)&plColor::FireBrick));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor FloralWhite", (void*)&plColor::FloralWhite));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor ForestGreen", (void*)&plColor::ForestGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Fuchsia", (void*)&plColor::Fuchsia));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Gainsboro", (void*)&plColor::Gainsboro));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor GhostWhite", (void*)&plColor::GhostWhite));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Gold", (void*)&plColor::Gold));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor GoldenRod", (void*)&plColor::GoldenRod));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Gray", (void*)&plColor::Gray));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Grey", (void*)&plColor::Grey));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Green", (void*)&plColor::Green));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor GreenYellow", (void*)&plColor::GreenYellow));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor HoneyDew", (void*)&plColor::HoneyDew));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor HotPink", (void*)&plColor::HotPink));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor IndianRed", (void*)&plColor::IndianRed));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Indigo", (void*)&plColor::Indigo));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Ivory", (void*)&plColor::Ivory));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Khaki", (void*)&plColor::Khaki));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Lavender", (void*)&plColor::Lavender));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LavenderBlush", (void*)&plColor::LavenderBlush));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LawnGreen", (void*)&plColor::LawnGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LemonChiffon", (void*)&plColor::LemonChiffon));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightBlue", (void*)&plColor::LightBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightCoral", (void*)&plColor::LightCoral));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightCyan", (void*)&plColor::LightCyan));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightGoldenRodYellow", (void*)&plColor::LightGoldenRodYellow));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightGray", (void*)&plColor::LightGray));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightGrey", (void*)&plColor::LightGrey));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightGreen", (void*)&plColor::LightGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightPink", (void*)&plColor::LightPink));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightSalmon", (void*)&plColor::LightSalmon));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightSeaGreen", (void*)&plColor::LightSeaGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightSkyBlue", (void*)&plColor::LightSkyBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightSlateGray", (void*)&plColor::LightSlateGray));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightSlateGrey", (void*)&plColor::LightSlateGrey));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightSteelBlue", (void*)&plColor::LightSteelBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LightYellow", (void*)&plColor::LightYellow));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Lime", (void*)&plColor::Lime));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor LimeGreen", (void*)&plColor::LimeGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Linen", (void*)&plColor::Linen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Magenta", (void*)&plColor::Magenta));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Maroon", (void*)&plColor::Maroon));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumAquaMarine", (void*)&plColor::MediumAquaMarine));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumBlue", (void*)&plColor::MediumBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumOrchid", (void*)&plColor::MediumOrchid));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumPurple", (void*)&plColor::MediumPurple));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumSeaGreen", (void*)&plColor::MediumSeaGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumSlateBlue", (void*)&plColor::MediumSlateBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumSpringGreen", (void*)&plColor::MediumSpringGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumTurquoise", (void*)&plColor::MediumTurquoise));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MediumVioletRed", (void*)&plColor::MediumVioletRed));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MidnightBlue", (void*)&plColor::MidnightBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MintCream", (void*)&plColor::MintCream));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor MistyRose", (void*)&plColor::MistyRose));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Moccasin", (void*)&plColor::Moccasin));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor NavajoWhite", (void*)&plColor::NavajoWhite));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Navy", (void*)&plColor::Navy));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor OldLace", (void*)&plColor::OldLace));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Olive", (void*)&plColor::Olive));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor OliveDrab", (void*)&plColor::OliveDrab));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Orange", (void*)&plColor::Orange));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor OrangeRed", (void*)&plColor::OrangeRed));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Orchid", (void*)&plColor::Orchid));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor PaleGoldenRod", (void*)&plColor::PaleGoldenRod));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor PaleGreen", (void*)&plColor::PaleGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor PaleTurquoise", (void*)&plColor::PaleTurquoise));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor PaleVioletRed", (void*)&plColor::PaleVioletRed));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor PapayaWhip", (void*)&plColor::PapayaWhip));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor PeachPuff", (void*)&plColor::PeachPuff));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Peru", (void*)&plColor::Peru));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Pink", (void*)&plColor::Pink));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Plum", (void*)&plColor::Plum));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor PowderBlue", (void*)&plColor::PowderBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Purple", (void*)&plColor::Purple));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor RebeccaPurple", (void*)&plColor::RebeccaPurple));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Red", (void*)&plColor::Red));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor RosyBrown", (void*)&plColor::RosyBrown));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor RoyalBlue", (void*)&plColor::RoyalBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SaddleBrown", (void*)&plColor::SaddleBrown));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Salmon", (void*)&plColor::Salmon));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SandyBrown", (void*)&plColor::SandyBrown));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SeaGreen", (void*)&plColor::SeaGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SeaShell", (void*)&plColor::SeaShell));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Sienna", (void*)&plColor::Sienna));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Silver", (void*)&plColor::Silver));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SkyBlue", (void*)&plColor::SkyBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SlateBlue", (void*)&plColor::SlateBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SlateGray", (void*)&plColor::SlateGray));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SlateGrey", (void*)&plColor::SlateGrey));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Snow", (void*)&plColor::Snow));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SpringGreen", (void*)&plColor::SpringGreen));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor SteelBlue", (void*)&plColor::SteelBlue));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Tan", (void*)&plColor::Tan));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Teal", (void*)&plColor::Teal));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Thistle", (void*)&plColor::Thistle));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Tomato", (void*)&plColor::Tomato));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Turquoise", (void*)&plColor::Turquoise));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Violet", (void*)&plColor::Violet));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Wheat", (void*)&plColor::Wheat));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor White", (void*)&plColor::White));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor WhiteSmoke", (void*)&plColor::WhiteSmoke));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor Yellow", (void*)&plColor::Yellow));
    AS_CHECK(m_pEngine->RegisterGlobalProperty("const plColor YellowGreen", (void*)&plColor::YellowGreen));

    AS_CHECK(m_pEngine->RegisterGlobalFunction("plColor MakeNaN()", asFUNCTION(plColor::MakeNaN), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plColor MakeZero()", asFUNCTION(plColor::MakeZero), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plColor MakeRGBA(float r, float g, float b, float a)", asFUNCTION(plColor::MakeRGBA), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plColor MakeFromKelvin(plUInt32 uiKelvin)", asFUNCTION(plColor::MakeFromKelvin), asCALL_CDECL));
    AS_CHECK(m_pEngine->RegisterGlobalFunction("plColor MakeHSV(float fHue, float fSat, float fVal)", asFUNCTION(plColor::MakeHSV), asCALL_CDECL));

    m_pEngine->SetDefaultNamespace("");
  }

  // Constructors
  {
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plColor", asBEHAVE_CONSTRUCT, "void f(float r, float g, float b, float a = 1.0f)", asFUNCTION(plColor_ConstructRGBA), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plColor", asBEHAVE_CONSTRUCT, "void f(const plColorGammaUB& in)", asFUNCTION(plColor_ConstructGamma), asCALL_CDECL_OBJFIRST));
  }

  // Operators
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void opAssign(const plColorGammaUB& in)", asMETHODPR(plColor, operator=, (const plColorGammaUB&), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void opAddAssign(const plColor& in)", asMETHODPR(plColor, operator+=, (const plColor&), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void opSubAssign(const plColor& in)", asMETHODPR(plColor, operator-=, (const plColor&), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void opMulAssign(const plColor& in)", asMETHODPR(plColor, operator*=, (const plColor&), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void opMulAssign(float)", asMETHODPR(plColor, operator*=, (float), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void opDivAssign(float)", asMETHODPR(plColor, operator/=, (float), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void opMulAssign(const plMat4& in)", asMETHODPR(plColor, operator*=, (const plMat4&), void), asCALL_THISCALL));


    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor opAdd(const plColor& in) const", asFUNCTIONPR(operator+, (const plColor&, const plColor&), const plColor), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor opSub(const plColor& in) const", asFUNCTIONPR(operator-, (const plColor&, const plColor&), const plColor), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor opMul(const plColor& in) const", asFUNCTIONPR(operator*, (const plColor&, const plColor&), const plColor), asCALL_CDECL_OBJFIRST));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor opMul(float) const", asFUNCTIONPR(operator*, (const plColor&, float), const plColor), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor opMul_r(float) const", asFUNCTIONPR(operator*, (const plColor&, float), const plColor), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor opDiv(float) const", asFUNCTIONPR(operator/, (const plColor&, float), const plColor), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor opMul_r(const plMat4& in) const", asFUNCTIONPR(operator*, (const plMat4&, const plColor&), const plColor), asCALL_CDECL_OBJLAST));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "bool opEquals(const plColor& in) const", asFUNCTIONPR(operator==, (const plColor&, const plColor&), bool), asCALL_CDECL_OBJFIRST));
  }

  // Methods
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void SetRGB(float r, float g, float b)", asMETHOD(plColor, SetRGB), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void SetRGBA(float r, float g, float b, float a = 1.0f)", asMETHOD(plColor, SetRGBA), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void GetHSV(float& out fHue, float& out fSaturation, float& out fValue) const", asMETHOD(plColor, GetHSV), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plVec4 GetAsVec4() const", asMETHOD(plColor, GetAsVec4), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "bool IsNormalized() const", asMETHOD(plColor, IsNormalized), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "float CalcAverageRGB() const", asMETHOD(plColor, CalcAverageRGB), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "float GetSaturation() const", asMETHOD(plColor, GetSaturation), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "float GetLuminance() const", asMETHOD(plColor, GetLuminance), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor GetInvertedColor() const", asMETHOD(plColor, GetInvertedColor), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor GetComplementaryColor() const", asMETHOD(plColor, GetComplementaryColor), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void ScaleRGB(float)", asMETHOD(plColor, ScaleRGB), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void ScaleRGBA(float)", asMETHOD(plColor, ScaleRGBA), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "float ComputeHdrMultiplier() const", asMETHOD(plColor, ComputeHdrMultiplier), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "float ComputeHdrExposureValue() const", asMETHOD(plColor, ComputeHdrExposureValue), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void ApplyHdrExposureValue(float fExposure)", asMETHOD(plColor, ApplyHdrExposureValue), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "void NormalizeToLdrRange()", asMETHOD(plColor, NormalizeToLdrRange), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor GetDarker(float fFactor = 2.0f) const", asMETHOD(plColor, GetDarker), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "bool IsNaN() const", asMETHOD(plColor, IsNaN), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "bool IsValid() const", asMETHOD(plColor, IsValid), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "bool IsIdenticalRGB(const plColor& in) const", asMETHOD(plColor, IsIdenticalRGB), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "bool IsIdenticalRGBA(const plColor& in) const", asMETHOD(plColor, IsIdenticalRGBA), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "bool IsEqualRGB(const plColor& in, float fEpsilon) const", asMETHOD(plColor, IsEqualRGB), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "bool IsEqualRGBA(const plColor& in, float fEpsilon) const", asMETHOD(plColor, IsEqualRGBA), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColor", "plColor WithAlpha(float fAlpha) const", asMETHOD(plColor, WithAlpha), asCALL_THISCALL));
  }
}

//////////////////////////////////////////////////////////////////////////
// plColorGammaUB
//////////////////////////////////////////////////////////////////////////

void plColorGamma_ConstructRGBA(void* pMemory, plUInt8 r, plUInt8 g, plUInt8 b, plUInt8 a)
{
  new (pMemory) plColorGammaUB(r, g, b, a);
}

void plColorGamma_ConstructColor(void* pMemory, const plColor& col)
{
  new (pMemory) plColorGammaUB(col);
}

void plAngelScriptEngineSingleton::Register_ColorGammaUB()
{
  AS_CHECK(m_pEngine->RegisterObjectProperty("plColorGammaUB", "uint8 r", asOFFSET(plColorGammaUB, r)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plColorGammaUB", "uint8 g", asOFFSET(plColorGammaUB, g)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plColorGammaUB", "uint8 b", asOFFSET(plColorGammaUB, b)));
  AS_CHECK(m_pEngine->RegisterObjectProperty("plColorGammaUB", "uint8 a", asOFFSET(plColorGammaUB, a)));

  // Constructors
  {
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plColorGammaUB", asBEHAVE_CONSTRUCT, "void f(uint8 r, uint8 g, uint8 b, uint8 a = 255)", asFUNCTION(plColorGamma_ConstructRGBA), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plColorGammaUB", asBEHAVE_CONSTRUCT, "void f(const plColor& in)", asFUNCTION(plColorGamma_ConstructColor), asCALL_CDECL_OBJFIRST));
  }

  // Operators
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColorGammaUB", "void opAssign(const plColor& in)", asMETHODPR(plColorGammaUB, operator=, (const plColor&), void), asCALL_THISCALL));
  }

  // Methods
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plColorGammaUB", "plColor ToLinearFloat() const", asMETHOD(plColorGammaUB, ToLinearFloat), asCALL_THISCALL));
  }
}

//////////////////////////////////////////////////////////////////////////
// plSpatial
//////////////////////////////////////////////////////////////////////////

void plSpatial_FindObjectsInSphere(plStringView sCategory, const plVec3& vCenter, float fRadius, asIScriptFunction* pCallback)
{
  plWorld* pWorld = plAngelScriptUtils::GetThreadLocalWorld();

  auto category = plSpatialData::FindCategory(sCategory);
  if (category != plInvalidSpatialDataCategory)
  {
    plSpatialSystem::QueryParams params;
    params.m_uiCategoryBitmask = category.GetBitmask();

    pWorld->GetSpatialSystem()->FindObjectsInSphere(plBoundingSphere::MakeFromCenterAndRadius(vCenter, fRadius), params, [pCallback](plGameObject* go) -> plVisitorExecution::Enum
      {
        asIScriptContext* pCtx = asGetActiveContext();
        pCtx->PushState();

        pCtx->Prepare(pCallback);
        pCtx->SetArgObject(0, go);
        pCtx->Execute();

        const plVisitorExecution::Enum res = (pCtx->GetReturnByte() != 0) ? plVisitorExecution::Continue : plVisitorExecution::Stop;

        pCtx->PopState();

        return res;
        //
      });
  }

  // need to release the refcount
  pCallback->Release();
}

void plAngelScriptEngineSingleton::Register_Spatial()
{
  AS_CHECK(m_pEngine->RegisterFuncdef("bool ReportObjectCB(plGameObject@)"));

  m_pEngine->SetDefaultNamespace("plSpatial");

  AS_CHECK(m_pEngine->RegisterGlobalFunction("void FindObjectsInSphere(plStringView sCategory, const plVec3& in vCenter, float fRadius, ReportObjectCB@ callback)", asFUNCTION(plSpatial_FindObjectsInSphere), asCALL_CDECL));

  m_pEngine->SetDefaultNamespace("");
}
