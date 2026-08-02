#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/Scripting/Bindings/ScriptBindingRegistry.h>
#include <Core/World/World.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Types/Variant.h>

thread_local plWorld* tl_pWorld = nullptr;

void plAngelScriptUtils::SetThreadLocalWorld(plWorld* pWorld)
{
  tl_pWorld = pWorld;
}

plWorld* plAngelScriptUtils::GetThreadLocalWorld()
{
  return tl_pWorld;
}

const char* plAngelScriptUtils::GetAsTypeName(asIScriptEngine* pEngine, int iAsTypeID)
{
  switch (iAsTypeID)
  {
    case asTYPEID_BOOL:
      return "bool";
    case asTYPEID_INT8:
      return "int8";
    case asTYPEID_INT16:
      return "int16";
    case asTYPEID_INT32:
      return "int32";
    case asTYPEID_INT64:
      return "int64";
    case asTYPEID_UINT8:
      return "uint8";
    case asTYPEID_UINT16:
      return "uint16";
    case asTYPEID_UINT32:
      return "uint32";
    case asTYPEID_UINT64:
      return "uint64";
    case asTYPEID_FLOAT:
      return "float";
    case asTYPEID_DOUBLE:
      return "double";

    default:
      if (const asITypeInfo* pInfo = pEngine->GetTypeInfoById(iAsTypeID))
      {
        return pInfo->GetName();
      }

      return nullptr;
  }
}


plString plAngelScriptUtils::GetNiceFunctionDeclaration(const asIScriptFunction* pFunc, bool bIncludeObjectName, bool bIncludeNamespace)
{
  plStringBuilder tmp;

  tmp = pFunc->GetDeclaration(bIncludeObjectName, bIncludeNamespace, true);

  tmp.ReplaceAll(" :: ", "::");
  tmp.ReplaceAll(" (", "(");
  tmp.ReplaceAll("( ", "(");
  tmp.ReplaceAll(" )", ")");
  tmp.ReplaceAll(") ", ")");
  tmp.ReplaceAll(" ,", ",");
  tmp.ReplaceAll(")const", ") const");

  return tmp;
}

const plRTTI* plAngelScriptUtils::MapToRTTI(int iAsTypeID, asIScriptEngine* pEngine)
{
  if (iAsTypeID == asTYPEID_BOOL)
    return plGetStaticRTTI<bool>();

  if (iAsTypeID == asTYPEID_INT8)
    return plGetStaticRTTI<plInt8>();

  if (iAsTypeID == asTYPEID_INT16)
    return plGetStaticRTTI<plInt16>();

  if (iAsTypeID == asTYPEID_INT32)
    return plGetStaticRTTI<plInt32>();

  if (iAsTypeID == asTYPEID_INT64)
    return plGetStaticRTTI<plInt64>();

  if (iAsTypeID == asTYPEID_UINT8)
    return plGetStaticRTTI<plUInt8>();

  if (iAsTypeID == asTYPEID_UINT16)
    return plGetStaticRTTI<plUInt16>();

  if (iAsTypeID == asTYPEID_UINT32)
    return plGetStaticRTTI<plUInt32>();

  if (iAsTypeID == asTYPEID_UINT64)
    return plGetStaticRTTI<plUInt64>();

  if (iAsTypeID == asTYPEID_FLOAT)
    return plGetStaticRTTI<float>();

  if (iAsTypeID == asTYPEID_DOUBLE)
    return plGetStaticRTTI<double>();

  if (const asITypeInfo* pInfo = pEngine->GetTypeInfoById(iAsTypeID))
  {
    return (const plRTTI*)pInfo->GetUserData(plAsUserData::RttiPtr);
  }

  return nullptr;
}

plResult plAngelScriptUtils::WriteToAsTypeAtLocation(asIScriptEngine* pEngine, int iAsTypeID, void* pMemoryLocation, const plVariant& value)
{
  void* pMemDst = pMemoryLocation;

  switch (iAsTypeID)
  {
    case asTYPEID_BOOL:
      *static_cast<plInt8*>(pMemDst) = value.ConvertTo<bool>() ? 1 : 0;
      return PL_SUCCESS;
    case asTYPEID_INT8:
      *static_cast<plInt8*>(pMemDst) = value.ConvertTo<plInt8>();
      return PL_SUCCESS;
    case asTYPEID_INT16:
      *static_cast<plInt16*>(pMemDst) = value.ConvertTo<plInt16>();
      return PL_SUCCESS;
    case asTYPEID_INT32:
      *static_cast<plInt32*>(pMemDst) = value.ConvertTo<plInt32>();
      return PL_SUCCESS;
    case asTYPEID_INT64:
      *static_cast<plInt64*>(pMemDst) = value.ConvertTo<plInt64>();
      return PL_SUCCESS;
    case asTYPEID_UINT8:
      *static_cast<plUInt8*>(pMemDst) = value.ConvertTo<plUInt8>();
      return PL_SUCCESS;
    case asTYPEID_UINT16:
      *static_cast<plUInt16*>(pMemDst) = value.ConvertTo<plUInt16>();
      return PL_SUCCESS;
    case asTYPEID_UINT32:
      *static_cast<plUInt32*>(pMemDst) = value.ConvertTo<plUInt32>();
      return PL_SUCCESS;
    case asTYPEID_UINT64:
      *static_cast<plUInt64*>(pMemDst) = value.ConvertTo<plUInt64>();
      return PL_SUCCESS;
    case asTYPEID_FLOAT:
      *static_cast<float*>(pMemDst) = value.ConvertTo<float>();
      return PL_SUCCESS;
    case asTYPEID_DOUBLE:
      *static_cast<double*>(pMemDst) = value.ConvertTo<double>();
      return PL_SUCCESS;
  }

  if (const asITypeInfo* pInfo = pEngine->GetTypeInfoById(iAsTypeID))
  {
    const plRTTI* pRtti = (const plRTTI*)pInfo->GetUserData(plAsUserData::RttiPtr);

    if (pRtti->GetTypeFlags().IsAnySet(plTypeFlags::IsEnum | plTypeFlags::Bitflags))
    {
      *static_cast<plUInt32*>(pMemDst) = value.ConvertTo<plUInt32>();
      return PL_SUCCESS;
    }

    if (pRtti == value.GetReflectedType() || value.CanConvertTo(pRtti->GetVariantType()))
    {
      if (pRtti == plGetStaticRTTI<plString>())
      {
        *static_cast<plString*>(pMemDst) = value.ConvertTo<plString>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plHashedString>())
      {
        static_cast<plHashedString*>(pMemDst)->Assign(value.ConvertTo<plString>());
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plAngle>())
      {
        *static_cast<plAngle*>(pMemDst) = value.ConvertTo<plAngle>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plTime>())
      {
        *static_cast<plTime*>(pMemDst) = value.ConvertTo<plTime>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plColor>())
      {
        *static_cast<plColor*>(pMemDst) = value.ConvertTo<plColor>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plColorGammaUB>())
      {
        *static_cast<plColorGammaUB*>(pMemDst) = value.ConvertTo<plColorGammaUB>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plVec2>())
      {
        *static_cast<plVec2*>(pMemDst) = value.ConvertTo<plVec2>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plVec3>())
      {
        *static_cast<plVec3*>(pMemDst) = value.ConvertTo<plVec3>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plVec4>())
      {
        *static_cast<plVec4*>(pMemDst) = value.ConvertTo<plVec4>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plQuat>())
      {
        *static_cast<plQuat*>(pMemDst) = value.ConvertTo<plQuat>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plTransform>())
      {
        *static_cast<plTransform*>(pMemDst) = value.ConvertTo<plTransform>();
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plGameObjectHandle>())
      {
        *static_cast<plGameObjectHandle*>(pMemDst) = *((const plGameObjectHandle*)value.GetData());
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plComponentHandle>())
      {
        *static_cast<plComponentHandle*>(pMemDst) = *((const plComponentHandle*)value.GetData());
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plWorld>())
      {
        *static_cast<plWorld**>(pMemDst) = ((plWorld*)value.GetData());
        return PL_SUCCESS;
      }
      else if (pRtti == plGetStaticRTTI<plGameObject>())
      {
        *static_cast<plGameObject**>(pMemDst) = ((plGameObject*)value.GetData());
        return PL_SUCCESS;
      }
    }
  }

  // currently unsupported type for exposed parameter
  return PL_FAILURE;
}

plResult plAngelScriptUtils::ReadFromAsTypeAtLocation(asIScriptEngine* pEngine, int iAsTypeID, void* pMemoryLocation, plVariant& out_value)
{
  void* pMemLoc = pMemoryLocation;

  if ((iAsTypeID & asTYPEID_APPOBJECT) == 0)
  {
    switch (iAsTypeID)
    {
      case asTYPEID_VOID:
        return PL_FAILURE;

      case asTYPEID_BOOL:
        out_value = (*static_cast<plInt8*>(pMemLoc) != 0) ? true : false;
        return PL_SUCCESS;

      case asTYPEID_INT8:
        out_value = *static_cast<plInt8*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_INT16:
        out_value = *static_cast<plInt16*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_INT32:
        out_value = *static_cast<plInt32*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_INT64:
        out_value = *static_cast<plInt64*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_UINT8:
        out_value = *static_cast<plUInt8*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_UINT16:
        out_value = *static_cast<plUInt16*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_UINT32:
        out_value = *static_cast<plUInt32*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_UINT64:
        out_value = *static_cast<plUInt64*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_FLOAT:
        out_value = *static_cast<float*>(pMemLoc);
        return PL_SUCCESS;

      case asTYPEID_DOUBLE:
        out_value = *static_cast<double*>(pMemLoc);
        return PL_SUCCESS;
    }

    if (const asITypeInfo* pInfo = pEngine->GetTypeInfoById(iAsTypeID))
    {
      if (pInfo->GetFlags() & asOBJ_ENUM)
      {
        out_value = *static_cast<plInt32*>(pMemLoc);
        return PL_SUCCESS;
      }
    }
  }
  else if (const asITypeInfo* pInfo = pEngine->GetTypeInfoById(iAsTypeID))
  {
    const plRTTI* pRtti = (const plRTTI*)pInfo->GetUserData(plAsUserData::RttiPtr);

    if (pRtti->GetTypeFlags().IsAnySet(plTypeFlags::IsEnum | plTypeFlags::Bitflags))
    {
      out_value = *static_cast<plUInt32*>(pMemLoc);
      return PL_SUCCESS;
    }

    if (pRtti == plGetStaticRTTI<plAngle>())
    {
      out_value = *static_cast<plAngle*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plTime>())
    {
      out_value = *static_cast<plTime*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plColor>())
    {
      out_value = *static_cast<plColor*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plColorGammaUB>())
    {
      out_value = *static_cast<plColorGammaUB*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plVec2>())
    {
      out_value = *static_cast<plVec2*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plVec3>())
    {
      out_value = *static_cast<plVec3*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plVec4>())
    {
      out_value = *static_cast<plVec4*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plQuat>())
    {
      out_value = *static_cast<plQuat*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plTransform>())
    {
      out_value = *static_cast<plTransform*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plString>())
    {
      out_value = *static_cast<plString*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plStringView>())
    {
      out_value = *static_cast<plStringView*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plHashedString>())
    {
      out_value = *static_cast<plHashedString*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plTempHashedString>())
    {
      out_value = *static_cast<plTempHashedString*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plGameObjectHandle>())
    {
      out_value = *static_cast<plGameObjectHandle*>(pMemLoc);
      return PL_SUCCESS;
    }
    else if (pRtti == plGetStaticRTTI<plComponentHandle>())
    {
      out_value = *static_cast<plComponentHandle*>(pMemLoc);
      return PL_SUCCESS;
    }
  }

  // currently unsupported type for exposed parameter
  return PL_FAILURE;
}

const char* plAngelScriptUtils::VariantTypeToString(plVariantType::Enum type)
{
  switch (type)
  {
    case plVariantType::Bool:
      return "bool";
    case plVariantType::Double:
      return "double";
    case plVariantType::Float:
      return "float";
    case plVariantType::Int8:
      return "int8";
    case plVariantType::Int16:
      return "int16";
    case plVariantType::Int32:
      return "int32";
    case plVariantType::Int64:
      return "int64";
    case plVariantType::UInt8:
      return "uint8";
    case plVariantType::UInt16:
      return "uint16";
    case plVariantType::UInt32:
      return "uint32";
    case plVariantType::UInt64:
      return "uint64";
    case plVariantType::Angle:
      return "plAngle";
    case plVariantType::Matrix3:
      return "plMat3";
    case plVariantType::Matrix4:
      return "plMat4";
    case plVariantType::Quaternion:
      return "plQuat";
    case plVariantType::Time:
      return "plTime";
    case plVariantType::Transform:
      return "plTransform";
    case plVariantType::Vector2:
      return "plVec2";
    case plVariantType::Vector3:
      return "plVec3";
    case plVariantType::Vector4:
      return "plVec4";
    case plVariantType::Color:
      return "plColor";
    case plVariantType::ColorGamma:
      return "plColorGammaUB";
    case plVariantType::String:
      return "plString";
    case plVariantType::HashedString:
      return "plHashedString";
    case plVariantType::StringView:
      return "plStringView";
    case plVariantType::TempHashedString:
      return "plTempHashedString";

    default:
      break;
  }

  return nullptr;
}

plString plAngelScriptUtils::DefaultValueToString(const plVariant& value, plVariantType::Enum expectedType)
{
  plStringBuilder s;
  switch (expectedType)
  {
    case plVariantType::Angle:
      s.SetFormat("plAngle::MakeDegrees({})", value.Get<plAngle>().GetDegree());
      return s;
    case plVariantType::Bool:
      s.Set(value.ConvertTo<bool>() ? "true" : "false");
      return s;
    case plVariantType::Color:
      s.SetFormat("plColor({}, {}, {}, {})", value.Get<plColor>().r, value.Get<plColor>().g, value.Get<plColor>().b, value.Get<plColor>().a);
      return s;

    case plVariantType::ColorGamma:
      s.SetFormat("plColorGammaUB({}, {}, {}, {})", value.Get<plColorGammaUB>().r, value.Get<plColorGammaUB>().g, value.Get<plColorGammaUB>().b, value.Get<plColorGammaUB>().a);
      return s;

    case plVariantType::Double:
      s.SetFormat("{}", value.ConvertTo<double>());
      return s;

    case plVariantType::Float:
      s.SetFormat("{}", value.ConvertTo<float>());
      return s;

    case plVariantType::HashedString:
    case plVariantType::String:
    case plVariantType::StringView:
      s.SetFormat("\"{}\"", value.ConvertTo<plString>());
      return s;

    case plVariantType::Int8:
    case plVariantType::Int16:
    case plVariantType::Int32:
    case plVariantType::Int64:
      s.SetFormat("{}", value.ConvertTo<plInt64>());
      return s;

    case plVariantType::UInt8:
    case plVariantType::UInt16:
    case plVariantType::UInt32:
    case plVariantType::UInt64:
      s.SetFormat("{}", value.ConvertTo<plUInt64>());
      return s;

    case plVariantType::Matrix3:
      s.Set("plMat3::MakeIdentity()");
      return s;
    case plVariantType::Matrix4:
      s.Set("plMat4::MakeIdentity()");
      return s;

    case plVariantType::Quaternion:
      s.Set("plQuat::MakeIdentity()");
      return s;

    case plVariantType::Time:
      s.SetFormat("plTime::Seconds({})", value.Get<plTime>().GetSeconds());
      return s;

    case plVariantType::Transform:
      s.Set("plTransform::MakeIdentity()");
      return s;

    case plVariantType::Vector2:
      s.SetFormat("plVec2({}, {})", value.Get<plVec2>().x, value.Get<plVec2>().y);
      return s;
    case plVariantType::Vector3:
      s.SetFormat("plVec3({}, {}, {})", value.Get<plVec3>().x, value.Get<plVec3>().y, value.Get<plVec3>().z);
      return s;
    case plVariantType::Vector4:
      s.SetFormat("plVec4({}, {}, {}, {})", value.Get<plVec4>().x, value.Get<plVec4>().y, value.Get<plVec4>().z, value.Get<plVec4>().w);
      return s;

    default:
      break;
  }

  PL_ASSERT_NOT_IMPLEMENTED;
  return "";
}

void plAngelScriptUtils::RetrieveArg(asIScriptGeneric* pGen, plUInt32 uiRealArg, plInt32& ref_iSkippedArg, const plAbstractFunctionProperty* pAbstractFuncProp, plVariant& out_arg)
{
  const plRTTI* pArgRtti = pAbstractFuncProp->GetArgumentType(uiRealArg);

  if (pArgRtti->GetTypeFlags().IsAnySet(plTypeFlags::IsEnum | plTypeFlags::Bitflags))
  {
    out_arg = (plInt32)pGen->GetArgDWord(ref_iSkippedArg);
    return;
  }

  const plVariantType::Enum type = pArgRtti->GetVariantType();
  switch (type)
  {
    case plVariantType::Bool:
      out_arg = pGen->GetArgByte(ref_iSkippedArg) != 0;
      return;
    case plVariantType::Double:
      out_arg = pGen->GetArgDouble(ref_iSkippedArg);
      return;
    case plVariantType::Float:
      out_arg = pGen->GetArgFloat(ref_iSkippedArg);
      return;
    case plVariantType::Int8:
      out_arg = (plInt8)pGen->GetArgByte(ref_iSkippedArg);
      return;
    case plVariantType::Int16:
      out_arg = (plInt16)pGen->GetArgWord(ref_iSkippedArg);
      return;
    case plVariantType::Int32:
      out_arg = (plInt32)pGen->GetArgDWord(ref_iSkippedArg);
      return;
    case plVariantType::Int64:
      out_arg = (plInt64)pGen->GetArgQWord(ref_iSkippedArg);
      return;
    case plVariantType::UInt8:
      out_arg = (plUInt8)pGen->GetArgByte(ref_iSkippedArg);
      return;
    case plVariantType::UInt16:
      out_arg = (plUInt16)pGen->GetArgWord(ref_iSkippedArg);
      return;
    case plVariantType::UInt32:
      out_arg = (plUInt32)pGen->GetArgDWord(ref_iSkippedArg);
      return;
    case plVariantType::UInt64:
      out_arg = (plUInt64)pGen->GetArgQWord(ref_iSkippedArg);
      return;

    case plVariantType::Vector2:
      out_arg = *((const plVec2*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Vector3:
      out_arg = *((const plVec3*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Vector4:
      out_arg = *((const plVec4*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Quaternion:
      out_arg = *((const plQuat*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Matrix3:
      out_arg = *((const plMat3*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Matrix4:
      out_arg = *((const plMat4*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Transform:
      out_arg = *((const plTransform*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Time:
      out_arg = *((const plTime*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Angle:
      out_arg = *((const plAngle*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::Color:
      out_arg = *((const plColor*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::ColorGamma:
      out_arg = *((const plColorGammaUB*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::String:
      out_arg = *((const plString*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::HashedString:
      out_arg = *((const plHashedString*)pGen->GetArgObject(ref_iSkippedArg));
      return;
    case plVariantType::StringView:
      out_arg = plVariant(*(const plStringView*)pGen->GetArgObject(ref_iSkippedArg), false);
      return;
    case plVariantType::TempHashedString:
      out_arg = *((const plTempHashedString*)pGen->GetArgObject(ref_iSkippedArg));
      return;

    case plVariantType::VariantArray:
      RetrieveVarArgs(pGen, ref_iSkippedArg, pAbstractFuncProp, out_arg);
      return;

    case plVariantType::Invalid:
    case plVariantType::TypedObject:
      // handled below
      break;

      PL_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  if (pArgRtti == plGetStaticRTTI<plWorld>())
  {
    out_arg = plAngelScriptUtils::GetThreadLocalWorld();
    // out_arg = (plWorld*)pGen->GetArgObject(uiArg);
    --ref_iSkippedArg;
    return;
  }

  if (pArgRtti == plGetStaticRTTI<plGameObjectHandle>())
  {
    out_arg = (plGameObjectHandle*)pGen->GetArgObject(ref_iSkippedArg);
    return;
  }

  if (pArgRtti == plGetStaticRTTI<plComponentHandle>())
  {
    out_arg = (plComponentHandle*)pGen->GetArgObject(ref_iSkippedArg);
    return;
  }

  if (pArgRtti == plGetStaticRTTI<plGameObject>())
  {
    out_arg = (plGameObject*)pGen->GetArgObject(ref_iSkippedArg);
    return;
  }

  if (pArgRtti->IsDerivedFrom(plGetStaticRTTI<plComponent>()))
  {
    out_arg = (plComponent*)pGen->GetArgObject(ref_iSkippedArg);
    return;
  }

  if (pArgRtti->GetTypeName().StartsWith("plVariant"))
  {
    auto argTypeId = pGen->GetArgTypeId(ref_iSkippedArg);

    if (plAngelScriptUtils::ReadFromAsTypeAtLocation(pGen->GetEngine(), argTypeId, pGen->GetArgAddress(ref_iSkippedArg), out_arg).Succeeded())
      return;

    const char* typeName = "null";
    if (const asITypeInfo* pInfo = pGen->GetEngine()->GetTypeInfoById(argTypeId))
    {
      typeName = pInfo->GetName();
    }

    plLog::Error("Call to '{}': Argument {} got an unsupported type '{}' ({})", pAbstractFuncProp->GetPropertyName(), ref_iSkippedArg, typeName, argTypeId);
    return;
  }

  PL_ASSERT_NOT_IMPLEMENTED;
  // out_arg = plVariant(pGen->GetArgObject(uiArg), pArgRtti); // works, but currently isn't needed
}

void plAngelScriptUtils::RetrieveVarArgs(asIScriptGeneric* pGen, plUInt32 uiStartArg, const plAbstractFunctionProperty* pAbstractFuncProp, plVariant& out_arg)
{
  plVariantArray resArr;

  for (plUInt32 uiArg = uiStartArg; uiArg < (plUInt32)pGen->GetArgCount(); ++uiArg)
  {
    auto argTypeId = pGen->GetArgTypeId(uiArg);

    plVariant res;
    if (plAngelScriptUtils::ReadFromAsTypeAtLocation(pGen->GetEngine(), argTypeId, pGen->GetArgAddress(uiArg), res).Succeeded())
    {
      resArr.PushBack(res);
      continue;
    }

    plStringBuilder typeName("null");
    if (const asITypeInfo* pInfo = pGen->GetEngine()->GetTypeInfoById(argTypeId))
    {
      typeName = pInfo->GetName();
    }

    plLog::Error("Call to '{}': Argument {} got an unsupported type '{}' ({})", pAbstractFuncProp->GetPropertyName(), uiArg, typeName, argTypeId);
    break;
  }

  out_arg = resArr;
}

void plAngelScriptUtils::MakeGenericFunctionCall(asIScriptGeneric* pGen)
{
  const plAbstractFunctionProperty* pAbstractFuncProp = (const plAbstractFunctionProperty*)pGen->GetAuxiliary();
  const plScriptableFunctionAttribute* pFuncAttr = pAbstractFuncProp->GetAttributeByType<plScriptableFunctionAttribute>();
  void* pObject = pGen->GetObject();

  PL_ASSERT_DEBUG(pAbstractFuncProp->GetArgumentCount() < 12, "Too many arguments");
  plVariant args[12];
  bool bHasOutArgs = false;

  plInt32 iNextArg = 0;
  for (plUInt32 uiArg = 0; uiArg < pAbstractFuncProp->GetArgumentCount(); ++uiArg)
  {
    if (pFuncAttr->GetArgumentType(uiArg) == plScriptableFunctionAttribute::ArgType::Out)
    {
      bHasOutArgs = true;
    }

    plAngelScriptUtils::RetrieveArg(pGen, uiArg, iNextArg, pAbstractFuncProp, args[uiArg]);
    ++iNextArg;
  }

  plVariant ret;
  pAbstractFuncProp->Execute(pObject, args, ret);

  if (bHasOutArgs)
  {
    for (plUInt32 uiArg = 0; uiArg < pAbstractFuncProp->GetArgumentCount(); ++uiArg)
    {
      if (pFuncAttr->GetArgumentType(uiArg) == plScriptableFunctionAttribute::ArgType::Out)
      {
        plAngelScriptUtils::WriteToAsTypeAtLocation(pGen->GetEngine(), pGen->GetArgTypeId(uiArg), pGen->GetArgAddress(uiArg), args[uiArg]).AssertSuccess();
      }
    }
  }

  const plRTTI* pReturnType = pAbstractFuncProp->GetReturnType();
  if (pReturnType != nullptr && pReturnType != plGetStaticRTTI<plVariantArray>())
  {
    const plRTTI* pRetRtti = plAngelScriptUtils::MapToRTTI(pGen->GetReturnTypeId(), pGen->GetEngine());
    plAngelScriptUtils::DefaultConstructInPlace(pGen->GetAddressOfReturnLocation(), pRetRtti);
    plAngelScriptUtils::WriteToAsTypeAtLocation(pGen->GetEngine(), pGen->GetReturnTypeId(), pGen->GetAddressOfReturnLocation(), ret).AssertSuccess();
  }
}

void plAngelScriptUtils::DefaultConstructInPlace(void* pPtr, const plRTTI* pRtti)
{
  if (pRtti == plGetStaticRTTI<plString>())
  {
    new (pPtr) plString();
    return;
  }

  if (pRtti == plGetStaticRTTI<plStringBuilder>())
  {
    new (pPtr) plStringBuilder();
    return;
  }

  if (pRtti == plGetStaticRTTI<plHashedString>())
  {
    new (pPtr) plHashedString();
    return;
  }

  // TODO: add other non-POD types here as needed
}

plString plAngelScriptUtils::RegisterEnumType(asIScriptEngine* pEngine, const plRTTI* pEnumType)
{
  plStringBuilder enumName = pEnumType->GetTypeName();
  enumName.ReplaceAll("::", "_");

  asITypeInfo* pEnumTypeInfo = pEngine->GetTypeInfoByName(enumName);
  if (pEnumTypeInfo != nullptr)
    return enumName;

  pEngine->RegisterEnum(enumName);

  pEnumTypeInfo = pEngine->GetTypeInfoByName(enumName);
  pEnumTypeInfo->SetUserData((void*)pEnumType, plAsUserData::RttiPtr);

  plHybridArray<plReflectionUtils::EnumKeyValuePair, 16> enumValues;
  plReflectionUtils::GetEnumKeysAndValues(pEnumType, enumValues, plReflectionUtils::EnumConversionMode::ValueNameOnly);
  for (auto& enumValue : enumValues)
  {
    pEngine->RegisterEnumValue(enumName, enumValue.m_sKey, enumValue.m_iValue);
  }

  return enumName;
}

static void SetPropertyGeneric(asIScriptGeneric* pGen)
{
  const plAbstractMemberProperty* pMember = static_cast<const plAbstractMemberProperty*>(pGen->GetAuxiliary());

  plVariant value;
  plAngelScriptUtils::ReadFromAsTypeAtLocation(pGen->GetEngine(), pGen->GetArgTypeId(0), pGen->GetAddressOfArg(0), value).AssertSuccess();

  plReflectionUtils::SetMemberPropertyValue(pMember, pGen->GetObject(), value);
}

static void GetPropertyGeneric(asIScriptGeneric* pGen)
{
  const plAbstractMemberProperty* pMember = static_cast<const plAbstractMemberProperty*>(pGen->GetAuxiliary());

  const plVariant value = plReflectionUtils::GetMemberPropertyValue(pMember, pGen->GetObject());

  const plRTTI* pRtti = plAngelScriptUtils::MapToRTTI(pGen->GetReturnTypeId(), pGen->GetEngine());
  plAngelScriptUtils::DefaultConstructInPlace(pGen->GetAddressOfReturnLocation(), pRtti);

  plAngelScriptUtils::WriteToAsTypeAtLocation(pGen->GetEngine(), pGen->GetReturnTypeId(), pGen->GetAddressOfReturnLocation(), value).AssertSuccess();
}

void plAngelScriptUtils::RegisterTypeProperties(
  asIScriptEngine* pEngine,
  const char* szTypeName,
  const plScriptBindingSnapshot& bindingSnapshot,
  const plRTTI* pRtti,
  bool bIsInherited)
{
  if (pRtti == nullptr)
    return;

  intptr_t flags = 0;
  if (bIsInherited)
  {
    flags |= 0x01;
  }

  plDynamicArray<const plScriptBindingPropertyDesc*> properties;
  bindingSnapshot.GetProperties(pRtti, properties);

  plStringBuilder funcName, sVarTypeName;

  for (const plScriptBindingPropertyDesc* pProperty : properties)
  {
    if (pProperty->m_Category == plPropertyCategory::Member)
    {
      const plAbstractProperty* pProp = pProperty->m_pProperty;
      const plAbstractMemberProperty* pMember = static_cast<const plAbstractMemberProperty*>(pProp);

      sVarTypeName.Clear();

      if (pMember->GetFlags().IsAnySet(plPropertyFlags::IsEnum | plPropertyFlags::Bitflags))
      {
        const plAbstractEnumerationProperty* pEnumProp = static_cast<const plAbstractEnumerationProperty*>(pMember);

        sVarTypeName = plAngelScriptUtils::RegisterEnumType(pEngine, pEnumProp->GetSpecificType());
      }
      else
      {
        const plRTTI* pPropRtti = pMember->GetSpecificType();

        sVarTypeName = plAngelScriptUtils::VariantTypeToString(pPropRtti->GetVariantType());

        if (sVarTypeName.IsEmpty())
        {
          if (pPropRtti->GetTypeName() == "plGameObjectHandle" || pPropRtti->GetTypeName() == "plComponentHandle")
          {
            sVarTypeName = pPropRtti->GetTypeName();
          }
        }
      }

      if (!sVarTypeName.IsEmpty())
      {
        if (!pMember->GetFlags().IsAnySet(plPropertyFlags::Const | plPropertyFlags::ReadOnly))
        {
          funcName.Set("void set_", pMember->GetPropertyName(), "(", sVarTypeName, ") property ");
          const int funcID = pEngine->RegisterObjectMethod(szTypeName, funcName, asFUNCTION(SetPropertyGeneric), asCALL_GENERIC, (void*)pMember);
          AS_CHECK(funcID);
          pEngine->GetFunctionById(funcID)->SetUserData(reinterpret_cast<void*>(flags), plAsUserData::FuncFlags);
        }

        {
          funcName.Set(sVarTypeName, " get_", pMember->GetPropertyName(), "() const property");
          const int funcID = pEngine->RegisterObjectMethod(szTypeName, funcName, asFUNCTION(GetPropertyGeneric), asCALL_GENERIC, (void*)pMember);
          AS_CHECK(funcID);
          pEngine->GetFunctionById(funcID)->SetUserData(reinterpret_cast<void*>(flags), plAsUserData::FuncFlags);
        }
      }
    }
  }

  RegisterTypeProperties(pEngine, szTypeName, bindingSnapshot, pRtti->GetParentType(), true);
}
