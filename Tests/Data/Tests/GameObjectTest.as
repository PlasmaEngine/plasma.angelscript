#include "TestFramework.as"

enum Phase
{
    Init,
    Render,

    Delete,
    CheckDelete,

    Done
}

class ScriptObject : plAngelScriptTestClass
{
    plTime RenderDelay = plTime::Seconds(0.2);

    private Phase m_Phase = Phase::Init;
    private plGameObjectHandle m_hChild1;
    private plGameObjectHandle m_hChild2;
    private plComponentHandle m_hFog;
    private plComponentHandle m_hLight;
    private plComponentHandle m_hBlackboard;

    private plTime m_tStart;

    ScriptObject()
    {
        super("GameObjectTest");
    }

    bool ExecuteTests()
    {
        if (m_Phase == Phase::Init)
        {
            PL_TEST_BOOL(m_hChild1.IsInvalidated());
            PL_TEST_BOOL(m_hChild2.IsInvalidated());

            plGameObject@ obj1 = GetOwner().FindChildByName("Child1");
            m_hChild1 = obj1.GetHandle();

            plGameObject@ obj2 = GetOwner().FindChildByPath("Child2");
            m_hChild2 = obj2.GetHandle();

            PL_TEST_BOOL(!m_hChild1.IsInvalidated());
            PL_TEST_BOOL(!m_hChild2.IsInvalidated());
            PL_TEST_BOOL(m_hChild1 == m_hChild1);
            PL_TEST_BOOL(m_hChild2 == m_hChild2);
            PL_TEST_BOOL(m_hChild1 != m_hChild2);

            PL_TEST_BOOL(obj1.GetParent().HasName("GameObject Test"));
            PL_TEST_BOOL(@obj1.GetWorld() == @GetWorld());

            PL_TEST_BOOL(!obj1.HasTag("Dummy"));
            PL_TEST_BOOL(obj2.HasTag("Dummy"));

            obj1.MakeDynamic();

            plFogComponent@ fog;
            PL_TEST_BOOL(obj1.TryGetComponentOfBaseType(@fog));

            PL_TEST_BOOL(m_hFog.IsInvalidated());
            m_hFog = fog.GetHandle();
            PL_TEST_BOOL(!m_hFog.IsInvalidated());

            fog.Color = plColor::OrangeRed;
            fog.Density = 5;

            plPointLightComponent@ light;
            PL_TEST_BOOL(obj2.TryGetComponentOfBaseType(@light));
            m_hLight = light.GetHandle();
            PL_TEST_BOOL(!light.Active);
            light.Active = true;

            m_tStart = plTime::Now();
            m_Phase = Phase::Render;

            plGameObjectDesc gd;
            gd.m_LocalPosition.Set(0, 0, 10);
            
            plGameObject@ spawnObj;
            GetWorld().CreateObject(gd, @spawnObj);

            plSpawnComponent@ spawnCmp;
            spawnObj.CreateComponent(@spawnCmp);

            spawnCmp.Prefab = "{ 42e938fb-5523-4606-8e64-6fee83dd0c7b }";
            spawnCmp.Deviation = plAngle::MakeFromDegree(30);
            spawnCmp.TriggerManualSpawn(true, plVec3::MakeZero());

            plAlwaysVisibleComponent@ vis;
            spawnObj.CreateComponent(@vis); // needed for the text to show up

            plDebugTextComponent@ text;
            spawnObj.CreateComponent(@text);
            text.Color = plColor::LightGreen;
            text.Text = "Hello World! ({})";
            text.Value0 = 42;

            plGameObject@ boxObj;
            gd.m_LocalPosition.z = 7;
            GetWorld().CreateObject(gd, @boxObj);

            plMeshComponent@ mesh;
            boxObj.CreateComponent(@mesh);
            mesh.Mesh = "{ abf38e0f-af01-4f2d-a167-d95defc809b7 }";
            mesh.Color = plColor::BlueViolet * 10;
            mesh.CustomData = plVec4(1, 2, 3, 4);

            plGameObject@ skybox;
            PL_TEST_BOOL(GetWorld().TryGetObjectWithGlobalKey("Skybox", skybox));

            plGameObject@ skybox2 = GetWorld().SearchForObject("G:Skybox");
            PL_TEST_BOOL(@skybox == @skybox2);

            plSkyBoxComponent@ skyboxComp;
            PL_TEST_BOOL(skybox.TryGetComponentOfBaseType(@skyboxComp));

            skyboxComp.CubeMap = "{ da307f05-9ef4-4a09-81d4-b86f5be99412 }";

            plGameObject@ bbObj;
            gd.m_LocalPosition.z = 7;
            gd.m_LocalPosition.y = 5;
            GetWorld().CreateObject(gd, @bbObj);            

            plLocalBlackboardComponent@ bbComp;
            bbObj.CreateComponent(@bbComp);
            m_hBlackboard = bbComp.GetHandle();

            bbComp.SetEntryValue("a", 1);
            bbComp.SetEntryValue("b", 2.0f);
            bbComp.SetEntryValue("c", plColor::RebeccaPurple);
            bbComp.SetEntryValue("d", plVec3(1, 2, 3));
            bbComp.SetEntryValue("e", plHashedString("HS"));
            bbComp.SetEntryValue("f", plColorGammaUB(50, 100, 150, 250));
            bbComp.SetEntryValue("g", m_hChild1);
            bbComp.SetEntryValue("h", m_hFog);
        }
        else if (m_Phase == Phase::Render)
        {
            plFogComponent@ fog;
            PL_TEST_BOOL(GetWorld().TryGetComponent(m_hFog, @fog));

            const float fLerp = plMath::Min(1.0f, ((plTime::Now() - m_tStart) / RenderDelay).AsFloatInSeconds());

            fog.Color = plMath::Lerp(plColor::OrangeRed, plColor::CornflowerBlue, fLerp);

            plWorld@ fogWorld = fog.GetWorld();
            plGameObject@ fogObj = fog.GetOwner();
            fog.GetOwner().SetGlobalPosition(plMath::Lerp(plVec3(0, 0, 5), plVec3(0, 0, 10), fLerp));

            if (plTime::Now() - m_tStart >= RenderDelay)
            {
                plPointLightComponent@ light;
                PL_TEST_BOOL(GetWorld().TryGetComponent(m_hLight, @light));
                light.Active = false;

                fogObj.SetActiveFlag(false);
    
                m_Phase = Phase::Delete;
            }
        }
        else if (m_Phase == Phase::Delete)
        {
            GetWorld().DeleteObjectDelayed(m_hChild1);

            plMsgDeleteGameObject msg;
            GetWorld().PostMessage(m_hChild2, msg, plTime::MakeZero(), plObjectMsgQueueType::NextFrame);

            {
                plLocalBlackboardComponent@ bbComp;
                PL_TEST_BOOL(GetWorld().TryGetComponent(m_hBlackboard, @bbComp));

                PL_TEST_INT(bbComp.GetEntryValue_asInt32("a"), 1);
                PL_TEST_FLOAT(bbComp.GetEntryValue_asFloat("b"), 2);
                PL_TEST_COLOR(bbComp.GetEntryValue_asColor("c"), plColor::RebeccaPurple);
                PL_TEST_VEC3(bbComp.GetEntryValue_asVec3("d"), plVec3(1, 2, 3));
                PL_TEST_STRING(bbComp.GetEntryValue_asString("e"), "HS");
                PL_TEST_COLOR(bbComp.GetEntryValue_asColor("f"), plColorGammaUB(50, 100, 150, 250));
                PL_TEST_BOOL(bbComp.GetEntryValue_asGameObjectHandle("g") == m_hChild1);
                PL_TEST_BOOL(bbComp.GetEntryValue_asComponentHandle("h") == m_hFog);
            }
            
            m_Phase = Phase::CheckDelete;
        }
        else if (m_Phase == Phase::CheckDelete)
        {
            plGameObject@ obj;

            PL_TEST_BOOL(!GetWorld().TryGetObject(m_hChild1, obj));
            PL_TEST_BOOL(!GetWorld().TryGetObject(m_hChild2, obj));
            
            m_Phase = Phase::Done;
        }

        return m_Phase != Phase::Done;
    }
}