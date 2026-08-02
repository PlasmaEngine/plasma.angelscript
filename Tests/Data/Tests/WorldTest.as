#include "TestFramework.as"

enum Phase
{
    CreateObj,
    CheckExists,
    CheckDeleted,
    Done,
}

class ScriptObject : plAngelScriptTestClass
{
    private Phase m_Phase = Phase::CreateObj;
    private plGameObjectHandle m_hCreated;
    private plTime m_LastUpdate;

    ScriptObject()
    {
        super("WorldTest");
    }
    
    bool ExecuteTests()
    {
        plGameObject@ owner = GetOwner();
        plWorld@ world = GetWorld();

        plGameObject@ allTestsObj;
        PL_TEST_BOOL(world.TryGetObjectWithGlobalKey("Tests", @allTestsObj));
        PL_TEST_STRING(allTestsObj.GetName(), "All Tests");

        plClock@ clock = world.GetClock();
        plTime tNow = clock.GetAccumulatedTime();
        PL_TEST_BOOL(tNow > m_LastUpdate);
        m_LastUpdate = tNow;

        if (m_Phase == Phase::CreateObj)
        {
            plGameObjectDesc gd;
            gd.m_LocalPosition.Set(1, 2, 3);
            gd.m_sName = "TestObj";
            gd.m_hParent = owner.GetHandle();
            
            plGameObject@ go;
            m_hCreated = GetWorld().CreateObject(gd, go);
            PL_TEST_BOOL(world.IsValidObject(m_hCreated));

            m_Phase = Phase::CheckExists;
            return true;
        }

        if (m_Phase == Phase::CheckExists)
        {
            plGameObject@ obj1 = owner.FindChildByName("TestObj");
            PL_TEST_BOOL(@obj1 != null);
            PL_TEST_BOOL(world.IsValidObject(m_hCreated));
            PL_TEST_BOOL(obj1.GetHandle() == m_hCreated);

            plGameObject@ obj2;
            PL_TEST_BOOL(world.TryGetObject(m_hCreated, @obj2));

            PL_TEST_BOOL(@obj1 == @obj2);

            world.DeleteObjectDelayed(obj1.GetHandle());

            PL_TEST_STRING(obj2.GetName(), "TestObj");

            m_Phase = Phase::CheckDeleted;
            return true;            
        }

        if (m_Phase == Phase::CheckDeleted)
        {
            plGameObject@ obj = owner.FindChildByName("TestObj");
            PL_TEST_BOOL(@obj == null);

            PL_TEST_BOOL(!world.IsValidObject(m_hCreated));

            plGameObject@ obj2;
            PL_TEST_BOOL(!world.TryGetObject(m_hCreated, @obj2));

            m_Phase = Phase::Done;
            return false;
        }

        return false;
    }
}

