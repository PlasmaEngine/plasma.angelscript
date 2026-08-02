#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>

class plAsWriteStream : public asIBinaryStream
{
public:
  int Write(const void* pPtr, asUINT size)
  {
    m_pBuffer->PushBackRange(plConstByteArrayPtr((const plUInt8*)pPtr, size));
    return size;
  }

  int Read(void* pPtr, asUINT size)
  {
    PL_ASSERT_NOT_IMPLEMENTED;
    return 0;
  }

  plDynamicArray<plUInt8>* m_pBuffer = nullptr;
};

class plAsReadStream : public asIBinaryStream
{
public:
  int Write(const void* pPtr, asUINT size)
  {
    PL_ASSERT_NOT_IMPLEMENTED;
    return 0;
  }

  int Read(void* pPtr, asUINT size)
  {
    const plUInt32 uiReadSize = plMath::Min(size, m_Buffer.GetCount() - m_uiReadPos);

    plMemoryUtils::RawByteCopy(pPtr, m_Buffer.GetPtr() + m_uiReadPos, uiReadSize);

    m_uiReadPos += uiReadSize;
    return uiReadSize;
  }

  plUInt32 m_uiReadPos = 0;
  plArrayPtr<plUInt8> m_Buffer;
};

void plAngelScriptUtils::SaveByteCode(asIScriptModule* pModule, plDynamicArray<plUInt8>& out_byteCode)
{
  plAsWriteStream stream;
  stream.m_pBuffer = &out_byteCode;

#if PL_ENABLED(PL_COMPILE_FOR_DEVELOPMENT)
  pModule->SaveByteCode(&stream, false); // TODO AS: strip debug info ? (flag?)
#else
  pModule->SaveByteCode(&stream, true);
#endif
}

asIScriptModule* plAngelScriptUtils::LoadFromByteCode(asIScriptEngine* pEngine, plStringView sModuleName, plArrayPtr<plUInt8> byteCode)
{
  plAsReadStream stream;
  stream.m_Buffer = byteCode;

  plStringBuilder tmp;
  asIScriptModule* pModule = pEngine->GetModule(sModuleName.GetData(tmp), asGM_ALWAYS_CREATE);

  if (pModule->LoadByteCode(&stream) < 0)
  {
    return nullptr;
  }

  return pModule;
}
