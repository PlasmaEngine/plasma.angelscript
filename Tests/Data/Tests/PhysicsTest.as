#include "TestFramework.as"

enum Phase
{
    Raycast,
    Spatial,
    Done
}

class ScriptObject : plAngelScriptTestClass
{
    private Phase m_Phase = Phase::Raycast;
    private int m_iFoundInside = 0;
    private int m_iFoundOther = 0;
    private array<plGameObjectHandle> m_Found;

    ScriptObject()
    {
        super("PhysicsTest");
    }

    bool FoundObject(plGameObject@ obj)
    {
        if (obj.HasName("Inside"))
            ++m_iFoundInside;
        else
            ++m_iFoundOther;

        m_Found.PushBack(obj.GetHandle());

        return true;
    }

    bool ExecuteTests()
    {
        if (m_Phase == Phase::Raycast)
        {
            plVec3 vHitPosition, vHitNormal;
            plGameObjectHandle hHitObject;

            PL_TEST_BOOL(!plPhysics::Raycast(vHitPosition, vHitNormal, hHitObject, plVec3(1, 2, 3), plVec3(0, 0, -1) * 10.0f, 0, plPhysicsShapeType::Dynamic));

            PL_TEST_BOOL(plPhysics::Raycast(vHitPosition, vHitNormal, hHitObject, plVec3(1, 2, 3), plVec3(0, 0, -1) * 10.0f, 0, plPhysicsShapeType::Static));

            PL_TEST_VEC3(vHitPosition, plVec3(1, 2, 0));
            PL_TEST_VEC3(vHitNormal, plVec3(0, 0, 1));

            m_Phase = Phase::Spatial;
        }
        else if (m_Phase == Phase::Spatial)
        {
            plSpatial::FindObjectsInSphere("Marker", plVec3(5.5f, 5.5f, 5.0f), 1.0f, ReportObjectCB(FoundObject));

            PL_TEST_INT(m_iFoundInside, 4);
            PL_TEST_INT(m_iFoundOther, 0);
            PL_TEST_INT(m_Found.GetCount(), 4);

            m_Phase = Phase::Done;
        }

        return m_Phase != Phase::Done;
    }
}
