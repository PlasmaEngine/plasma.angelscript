#include "TestFramework.as"

enum Phase
{
    CVar,
    Array,
    Prefabs,
    Done
}

class ScriptObject : plAngelScriptTestClass
{
    private Phase m_Phase = Phase::CVar;

    ScriptObject()
    {
        super("MiscTest");
    }

    bool ExecuteTests()
    {
        if (m_Phase == Phase::CVar)
        {
            bool bOrgValue = plCVar::GetBoolValue("App.ShowFPS");

            bool bShowFPS1 = plCVar::GetBoolValue("App.ShowFPS");
            bool bShowFPS2 = plCVar::GetValue_asBool("App.ShowFPS");
            PL_TEST_BOOL(bShowFPS1 == bShowFPS2);

            plCVar::SetBoolValue("App.ShowFPS", !bOrgValue);
            
            bShowFPS1 = plCVar::GetBoolValue("App.ShowFPS");
            bShowFPS2 = plCVar::GetValue_asBool("App.ShowFPS");
            PL_TEST_BOOL(bShowFPS1 == bShowFPS2);

            plCVar::SetBoolValue("App.ShowFPS", false);

            m_Phase = Phase::Array;
        }
        else if (m_Phase == Phase::Array)
        {
            array<plInt32> a;
            PL_TEST_BOOL(a.IsEmpty());
            a.SetCount(32);
            PL_TEST_INT(a.GetCount(), 32);
            PL_TEST_BOOL(!a.IsEmpty());
            
            for (plUInt32 i = 0; i < 32; ++i)
            {
                PL_TEST_INT(a[i], 0);
                a[i] = i + 1;
                PL_TEST_INT(a[i], i + 1);
            }

            PL_TEST_INT(a.PeekBack(), 32);

            PL_TEST_BOOL(a.Contains(11));
            PL_TEST_BOOL(!a.Contains(0));
            PL_TEST_BOOL(!a.Contains(33));

            array<plInt32> b = a;
            PL_TEST_BOOL(a == b);

            a.ExpandAndGetRef() = 33;
            PL_TEST_INT(a.GetCount(), 33);
            PL_TEST_INT(a[32], 33);

            PL_TEST_INT(a.IndexOf(13), 12);

            PL_TEST_BOOL(a != b);
            b.ExpandAndGetRef() = 33;
            PL_TEST_BOOL(a == b);

            a.PopBack();
            PL_TEST_BOOL(a != b);
            b.PopBack();
            PL_TEST_BOOL(a == b);

            a.Reverse();

            for (plUInt32 i = 0; i < 32; ++i)
            {
                PL_TEST_INT(a[i], 32 - i);
            }

            PL_TEST_BOOL(a != b);
            a.Sort();
            PL_TEST_BOOL(a == b);

            a.Clear();
            PL_TEST_BOOL(a.IsEmpty());
            
            b.SetCount(0);
            PL_TEST_BOOL(b.IsEmpty());
            PL_TEST_BOOL(a == b);

            m_Phase = Phase::Prefabs;
        }
        else if (m_Phase == Phase::Prefabs)
        {
            plTransform localTransform = plTransform::Make(plVec3(1, 2, 3));
            plPrefabs::SpawnPrefab("{ a3ce5d3d-be5e-4bda-8820-b1ce3b3d33fd }", plTransform::MakeGlobalTransform(GetOwner().GetGlobalTransform(), localTransform));

            localTransform.m_vPosition = plVec3(2, 3, 4);
            plPrefabs::SpawnPrefabAsChild("{ 42e938fb-5523-4606-8e64-6fee83dd0c7b }", GetOwner(), localTransform);
            
            m_Phase = Phase::Done;
        }

        return m_Phase != Phase::Done;
    }
}
