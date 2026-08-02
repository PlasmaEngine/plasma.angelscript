#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAngelScript/AngelScriptAsset/AngelScriptAsset.h>
#include <EditorPluginAngelScript/AngelScriptAsset/AngelScriptAssetManager.h>
#include <EditorPluginAngelScript/AngelScriptWindow/AngelScriptWindow.moc.h>

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAngelScriptAssetManager, 1, plRTTIDefaultAllocator<plAngelScriptAssetManager>)
PL_END_DYNAMIC_REFLECTED_TYPE;

plAngelScriptAssetManager::plAngelScriptAssetManager()
{
  plDocumentManager::s_Events.AddEventHandler(plMakeDelegate(&plAngelScriptAssetManager::OnDocumentManagerEvent, this));

  m_DocTypeDesc.m_sDocumentTypeName = "AngelScript";
  m_DocTypeDesc.m_sFileExtension = "plAngelScriptAsset";
  m_DocTypeDesc.m_sIcon = ":/AssetIcons/AngelScript-AS.svg";
  m_DocTypeDesc.m_sAssetCategory = "Scripting";
  m_DocTypeDesc.m_pDocumentType = plGetStaticRTTI<plAngelScriptAssetDocument>();
  m_DocTypeDesc.m_pManager = this;
  m_DocTypeDesc.m_CompatibleTypes.PushBack("CompatibleAsset_ScriptClass");

  m_DocTypeDesc.m_sResourceFileExtension = "plBinAngelScript";
  m_DocTypeDesc.m_AssetDocumentFlags = plAssetDocumentFlags::AutoTransformOnSave;

  plQtImageCache::GetSingleton()->RegisterTypeImage("AngelScript", QPixmap(":/AssetIcons/AngelScript-Big-AS.svg"));
}

plAngelScriptAssetManager::~plAngelScriptAssetManager()
{
  plDocumentManager::s_Events.RemoveEventHandler(plMakeDelegate(&plAngelScriptAssetManager::OnDocumentManagerEvent, this));
}

void plAngelScriptAssetManager::OnDocumentManagerEvent(const plDocumentManager::Event& e)
{
  switch (e.m_Type)
  {
    case plDocumentManager::Event::Type::DocumentWindowRequested:
    {
      if (e.m_pDocument->GetDynamicRTTI() == plGetStaticRTTI<plAngelScriptAssetDocument>())
      {
        new plQtAngelScriptAssetDocumentWindow((plAngelScriptAssetDocument*)e.m_pDocument); // NOLINT: Not a memory leak
      }
    }
    break;

    default:
      break;
  }
}

void plAngelScriptAssetManager::InternalCreateDocument(
  plStringView sDocumentTypeName, plStringView sPath, bool bCreateNewDocument, plDocument*& out_pDocument, const plDocumentObject* pOpenContext)
{
  out_pDocument = new plAngelScriptAssetDocument(sPath);
}

void plAngelScriptAssetManager::InternalGetSupportedDocumentTypes(plDynamicArray<const plDocumentTypeDescriptor*>& inout_DocumentTypes) const
{
  inout_DocumentTypes.PushBack(&m_DocTypeDesc);
}
