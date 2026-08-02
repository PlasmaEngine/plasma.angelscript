#include "../TestFramework.as"

void ExecuteTests()
{
    // These tests only test the general AS/C++ binding.
    // The are meant to find cases where the binding wasn't set up correctly, 
    // which mostly happens for operators, constructors and other special functions.
    // They do not test all the functionality, because that is already covered by the C++ tests.

    // plAngle
    {
        plAngle a, b;
        a = plAngle::MakeFromDegree(90);

        PL_TEST_FLOAT(a.GetDegree(), 90.0f);
        b = -a;

        PL_TEST_FLOAT(b.GetDegree(), -90.0f);

        plAngle c = a + b;
        c += a;
        c -= b;

        PL_TEST_FLOAT(c.GetDegree(), c.GetDegree());
        PL_TEST_BOOL(c == c);
        PL_TEST_FLOAT(c.GetDegree(), (a * 2.0).GetDegree());
        PL_TEST_BOOL(c == 2.0f * a);
        PL_TEST_BOOL(c / 2.0f == a);
        PL_TEST_BOOL(c / a == 2.0f);

        PL_TEST_BOOL(a < c);
        PL_TEST_BOOL(c > a);

        plAngle d = plAngle::AngleBetween(a, b);
        PL_TEST_FLOAT(d.GetDegree(), 180);
    }

    // plTime
    {
        plTime t1 = plTime::MakeZero();
        PL_TEST_FLOAT(t1.AsFloatInSeconds(), 0.0f);

        t1 = plTime::MakeFromSeconds(1.23f);
        PL_TEST_FLOAT(t1.AsFloatInSeconds(), 1.23f);

        t1 += plTime::Seconds(2.0f);
        t1 -= plTime::Seconds(0.1f);
        PL_TEST_FLOAT(t1.AsFloatInSeconds(), 3.13f);

        t1 *= 4.0f;
        t1 /= 2.0f;
        t1 = -t1;
        PL_TEST_FLOAT(t1.AsFloatInSeconds(), -6.26f);

        plTime t2 = plTime::Seconds(6.26f);
        PL_TEST_FLOAT(t1.AsFloatInSeconds(), -t2.AsFloatInSeconds());

        plTime t3 = t1;
        PL_TEST_BOOL(t1 == t3);
        PL_TEST_BOOL(t1 < t2);
        PL_TEST_BOOL(t2 > t1);

        t3 *= 2.0f;
        PL_TEST_BOOL(t3 < t1);
        
        t3 /= 2.0f;
        PL_TEST_FLOAT(t1.AsFloatInSeconds(), t3.AsFloatInSeconds());

        t1 = 4 * (plTime::Seconds(2) * plTime::Seconds(3));
        t2 = plTime::Seconds(4) * 1.5f;
        PL_TEST_FLOAT(t1.AsFloatInSeconds(), 24);
        PL_TEST_FLOAT(t2.AsFloatInSeconds(), 6);
        
        t3 = t1 + t2;
        PL_TEST_FLOAT(t3.AsFloatInSeconds(), 30);
        
        t3 = t3 - t1;
        PL_TEST_FLOAT(t3.AsFloatInSeconds(), 6);

        t3 = t1 / t2;
        PL_TEST_FLOAT(t3.AsFloatInSeconds(), 4);

        t3 = t1 / 3.0f;
        PL_TEST_FLOAT(t3.AsFloatInSeconds(), 8);

        t3 = 48 / t1;
        PL_TEST_FLOAT(t3.AsFloatInSeconds(), 2);
    }

    // plColor / plColorGammaUB
    {
        plColor c1(0, 0, 0);
        PL_TEST_COLOR(c1, plColor::Black);

        plColor c2(plColorGammaUB(255, 255, 255));
        PL_TEST_COLOR(c2, plColor::White);

        c2 = c1;
        PL_TEST_COLOR(c2, plColor::Black);

        c1 = plColorGammaUB(255, 255, 255);
        PL_TEST_COLOR(c1, plColor::White);

        c2 += plColor::White;
        PL_TEST_COLOR(c2, plColor(1, 1, 1, 2));

        c2 -= plColor::White;
        PL_TEST_COLOR(c2, plColor::Black);

        c1 *= 2;
        PL_TEST_COLOR(c1, plColor::White * 2);

        c1 /= 2;
        PL_TEST_COLOR(c1, (2 * plColor::White) / 2);

        c1 *= c1 + c1;
        PL_TEST_COLOR(c1, plColor::White * 2);

        c1 = c1 - c1;
        PL_TEST_COLOR(c1, plColor::MakeZero());

        PL_TEST_BOOL(c1 == plColor::MakeZero());

        PL_TEST_VEC4(plColor::White.WithAlpha(0.5).GetAsVec4(), plVec4(1, 1, 1, 0.5));

        PL_TEST_COLOR(plColor::Red * plColor::Blue, plColor::Black);

        PL_TEST_VEC4((plColor::White / 2.0f).GetAsVec4(), plVec4(0.5f));

        plColorGammaUB cg = plColor::Lime;
        PL_TEST_INT(cg.r, 0);
        PL_TEST_INT(cg.g, 255);
        PL_TEST_INT(cg.b, 0);
        PL_TEST_INT(cg.a, 255);

        cg = plColor::Red;
        PL_TEST_INT(cg.r, 255);
        PL_TEST_INT(cg.g, 0);
        PL_TEST_INT(cg.b, 0);
        PL_TEST_INT(cg.a, 255);
    }

    // plVec2
    {
        const plVec2 c0 = plVec2::MakeZero();
        const plVec2 c1(2, 4);
        const plVec2 c2(4, 2);
        const plVec2 c3(6);

        PL_TEST_FLOAT(c0.x, 0);
        PL_TEST_FLOAT(c0.y, 0);

        PL_TEST_FLOAT(c1.x, 2);
        PL_TEST_FLOAT(c1.y, 4);

        PL_TEST_FLOAT(c3.x, 6);
        PL_TEST_FLOAT(c3.y, 6);

        PL_TEST_BOOL(c1 != c2);
        PL_TEST_BOOL(c1 == c1);
        PL_TEST_BOOL(c1 < c3);
        PL_TEST_BOOL(c3 > c1);

        plVec2 v;
        v = c0;
        PL_TEST_VEC2(v, c0);
        
        v += 2 * c1;
        PL_TEST_VEC2(v, c1 * 2);
        
        v -= c1 * 2;
        PL_TEST_VEC2(v, c0);

        v = c3;
        v *= 4;
        v /= 8;
        PL_TEST_VEC2(v, c3 / 2);
        
        v = -1 * (c1 + c2);
        PL_TEST_VEC2(v, -c3);
        
        v = c3 - c1;
        PL_TEST_VEC2(v, c2);
    }

    // plVec3
    {
        const plVec3 c0 = plVec3::MakeZero();
        const plVec3 c1(2, 4, 3);
        const plVec3 c2(4, 2, 3);
        const plVec3 c3(6);

        PL_TEST_FLOAT(c0.x, 0);
        PL_TEST_FLOAT(c0.y, 0);
        PL_TEST_FLOAT(c0.z, 0);

        PL_TEST_FLOAT(c1.x, 2);
        PL_TEST_FLOAT(c1.y, 4);
        PL_TEST_FLOAT(c1.z, 3);

        PL_TEST_FLOAT(c3.x, 6);
        PL_TEST_FLOAT(c3.y, 6);
        PL_TEST_FLOAT(c3.z, 6);

        PL_TEST_BOOL(c1 != c2);
        PL_TEST_BOOL(c1 == c1);
        PL_TEST_BOOL(c1 < c3);
        PL_TEST_BOOL(c3 > c1);

        plVec3 v;
        v = c0;
        PL_TEST_VEC3(v, c0);
        
        v += 2 * c1;
        PL_TEST_VEC3(v, c1 * 2);
        
        v -= c1 * 2;
        PL_TEST_VEC3(v, c0);

        v = c3;
        v *= 4;
        v /= 8;
        PL_TEST_VEC3(v, c3 / 2);
        
        v = -1 * (c1 + c2);
        PL_TEST_VEC3(v, -c3);
        
        v = c3 - c1;
        PL_TEST_VEC3(v, c2);
    }

    // plVec4
    {
        const plVec4 c0 = plVec4::MakeZero();
        const plVec4 c1(2, 4, 3, 1);
        const plVec4 c2(4, 2, 3, 5);
        const plVec4 c3(6);

        PL_TEST_FLOAT(c0.x, 0);
        PL_TEST_FLOAT(c0.y, 0);
        PL_TEST_FLOAT(c0.z, 0);
        PL_TEST_FLOAT(c0.w, 0);

        PL_TEST_FLOAT(c1.x, 2);
        PL_TEST_FLOAT(c1.y, 4);
        PL_TEST_FLOAT(c1.z, 3);
        PL_TEST_FLOAT(c1.w, 1);

        PL_TEST_FLOAT(c3.x, 6);
        PL_TEST_FLOAT(c3.y, 6);
        PL_TEST_FLOAT(c3.y, 6);
        PL_TEST_FLOAT(c3.y, 6);

        PL_TEST_BOOL(c1 != c2);
        PL_TEST_BOOL(c1 == c1);
        PL_TEST_BOOL(c1 < c3);
        PL_TEST_BOOL(c3 > c1);

        plVec4 v;
        v = c0;
        PL_TEST_VEC4(v, c0);
        
        v += 2 * c1;
        PL_TEST_VEC4(v, c1 * 2);
        
        v -= c1 * 2;
        PL_TEST_VEC4(v, c0);

        v = c3;
        v *= 4;
        v /= 8;
        PL_TEST_VEC4(v, c3 / 2);
        
        v = -1 * (c1 + c2);
        PL_TEST_VEC4(v, -c3);
        
        v = c3 - c1;
        PL_TEST_VEC4(v, c2);
    }

    // plQuat
    {
        PL_TEST_QUAT(plQuat::MakeIdentity(), plQuat::MakeFromElements(0, 0, 0, 1));

        plQuat q = plQuat::MakeFromElements(1, 2, 3, 4);
        PL_TEST_FLOAT(q.x, 1);
        PL_TEST_FLOAT(q.y, 2);
        PL_TEST_FLOAT(q.z, 3);
        PL_TEST_FLOAT(q.w, 4);

        PL_TEST_BOOL(q == q);
        PL_TEST_BOOL(q != q.GetNegated());

        plQuat q2 = q;
        PL_TEST_BOOL(q2 == q);

        q2 = plQuat::MakeFromAxisAndAngle(plVec3(1, 0, 0), plAngle::MakeFromDegree(90));
        plVec3 v = q2 * plVec3(1, 2, 3);

        PL_TEST_VEC3(v, plVec3(1, -3, 2));

        q = q2 * q2;

        plVec3 axis;
        plAngle angle;
        q.GetRotationAxisAndAngle(axis, angle);

        PL_TEST_VEC3(axis, plVec3(1, 0, 0));
        PL_TEST_FLOAT(angle.GetDegree(), 180);
    }

    // plTransform
    {
        plTransform t1 = plTransform::MakeIdentity();
        PL_TEST_VEC3(t1.m_vPosition, plVec3::MakeZero());
        PL_TEST_QUAT(t1.m_qRotation, plQuat::MakeIdentity());
        PL_TEST_VEC3(t1.m_vScale, plVec3(1));

        PL_TEST_BOOL(t1 == t1.GetInverse());
        
        plTransform t2;
        t2 = t1;

        PL_TEST_BOOL(t1 == t2);
        
        t1 += plVec3(1);
        PL_TEST_BOOL(t1 != t2);
        PL_TEST_BOOL(t1 == t2 + plVec3(1));
        PL_TEST_BOOL(t1 - plVec3(1) == t2);

        t1 -= plVec3(1);
        PL_TEST_BOOL(t1 == t2);

        PL_TEST_BOOL(plTransform::Make(plVec3(1), plQuat::MakeIdentity(), plVec3(2)) == plTransform(plVec3(1), plQuat::MakeIdentity(), plVec3(2)));

        t1 = plTransform::Make(plVec3(1, 2, 3));
        plVec3 pos(2, 3, 4);
        pos = t1 * pos;
        PL_TEST_VEC3(pos, plVec3(3, 5, 7));

        plQuat rot = plQuat::MakeFromAxisAndAngle(plVec3(1, 0, 0), plAngle::MakeFromDegree(90));
        t1 = t1 * rot;
        t1 = rot * t1;

        plTransform t3 = t1 * t2;
        plTransform t4 = plTransform::MakeGlobalTransform(t1, t2);
        PL_TEST_BOOL(t3 == t4);
    }

    // plMath
    {
        // nothing that needs testing
    }
}