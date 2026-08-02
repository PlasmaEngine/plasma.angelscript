#pragma once

#include <EditorFramework/Assets/SimpleAssetDocument.h>
#include <ToolsFoundation/VisualGraph/VisualGraphObjectManager.h>

class plAngelScriptAssetDocument;
struct plPropertyMetaStateEvent;

struct plAngelScriptCodeMode
{
  using StorageType = plUInt8;

  enum Enum : StorageType
  {
    Inline,
    FromFile,

    Default = Inline
  };
};

PL_DECLARE_REFLECTABLE_TYPE(PL_EDITORPLUGINANGELSCRIPT_DLL, plAngelScriptCodeMode);

class plAngelScriptParameter : public plReflectedClass
{
  PL_ADD_DYNAMIC_REFLECTION(plAngelScriptParameter, plReflectedClass);

public:
  bool m_bExpose = false;
  plString m_sDeclaration;
  plString m_sName;
  plVariant m_DefaultValue;
};

class plAngelScriptAssetProperties : public plReflectedClass
{
  PL_ADD_DYNAMIC_REFLECTION(plAngelScriptAssetProperties, plReflectedClass);

  plEnum<plAngelScriptCodeMode> m_CodeMode;
  plString m_sScriptFile;
  plString m_sClassName;
  plString m_sCode;

  plDynamicArray<plAngelScriptParameter> m_Parameters;
  plDynamicArray<plString> m_Dependencies;
};

class plAngelScriptAssetDocument : public plSimpleAssetDocument<plAngelScriptAssetProperties>
{
  PL_ADD_DYNAMIC_REFLECTION(plAngelScriptAssetDocument, plSimpleAssetDocument<plAngelScriptAssetProperties>);

public:
  plAngelScriptAssetDocument(plStringView sDocumentPath);

  void OpenExternalEditor();
  void SyncExposedParameters();

  static void PropertyMetaStateEventHandler(plPropertyMetaStateEvent& e);

protected:
  virtual plTransformStatus InternalTransformAsset(const char* szTargetFile, plStringView sOutputTag, const plPlatformProfile* pAssetProfile, const plAssetFileHeader& AssetHeader, plBitflags<plTransformFlags> transformFlags) override;
  virtual plTransformStatus InternalTransformAsset(plStreamWriter& stream, plStringView sOutputTag, const plPlatformProfile* pAssetProfile, const plAssetFileHeader& AssetHeader, plBitflags<plTransformFlags> transformFlags) override;

  void SyncInfos();

  virtual void UpdateAssetDocumentInfo(plAssetDocumentInfo* pInfo) const override;
};
