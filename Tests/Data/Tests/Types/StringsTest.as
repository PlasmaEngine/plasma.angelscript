#include "../TestFramework.as"

void ExecuteTests()
{
    plString m_sString = "String";
    plHashedString m_sHashedString = "HashedString";
    plTempHashedString m_sTempHashedString = "TempHashedString";
    plStringView svLong = "Nett hier. Aber waren sie schon mal in Baden-Württemberg?";

    // plStringView
    {
        plStringView sv0;
        PL_TEST_BOOL(sv0.IsEmpty());

        plStringView sv1("Test1");
        PL_TEST_BOOL(!sv1.IsEmpty());
        PL_TEST_STRING(sv1, "Test1");

        plStringView sv2(sv1);
        PL_TEST_BOOL(sv1 == sv2);
        PL_TEST_BOOL(sv1 == "Test1");
        PL_TEST_STRING(sv1, sv2);

        plStringView sv3(m_sString);
        PL_TEST_STRING(sv3, m_sString);

        plStringBuilder sb1;
        sb1 = "Test1";
        PL_TEST_STRING(sv1, sb1);
        PL_TEST_BOOL(sv1 == sb1);

        plStringBuilder sb2("Test2");

        plStringView sv4(sb1);
        PL_TEST_STRING(sv4, sb1);
        sv4 = sb2;
        PL_TEST_STRING(sv4, sb2);
        PL_TEST_BOOL(sv4 == sb2);
        PL_TEST_BOOL(sv4 != sb1);

        sv1 = m_sHashedString;
        PL_TEST_STRING(sv1, m_sHashedString.GetView());
        PL_TEST_STRING(sv1, m_sHashedString);

        sv1 = m_sString;
        PL_TEST_STRING(sv1, m_sString);

        sv1 = sb2;
        PL_TEST_STRING(sv1, sb2);

        PL_TEST_BOOL("Abcd" < "aBCD");

        PL_TEST_BOOL("abc" > "ABC");
    }

    // plString
    {
        plString s0;
        PL_TEST_BOOL(s0.IsEmpty());

        plString s1("abc");
        PL_TEST_STRING(s1, "abc");

        s1 = svLong;
        PL_TEST_BOOL(s1 == svLong);
        PL_TEST_STRING(s1, svLong);
        PL_TEST_INT(s1.GetElementCount(), 58);

        plString s2(s1);
        PL_TEST_STRING(s2, svLong);

        plStringBuilder sb1(svLong);
        PL_TEST_INT(sb1.GetCharacterCount(), 57);

        PL_TEST_BOOL(!s2.IsEmpty());
        s2.Clear();
        PL_TEST_BOOL(s2.IsEmpty());

        s2 = sb1;
        PL_TEST_STRING(s2, svLong);

        plString s3(sb1);
        PL_TEST_STRING(s3, svLong);
        s3 = m_sHashedString;
        PL_TEST_STRING(s3, m_sHashedString);

        plString s4(m_sHashedString);
        PL_TEST_STRING(s4, m_sHashedString);

        s4 = "TEST";
        PL_TEST_STRING(s4, "TEST");

        s4 = s3;
        PL_TEST_STRING(s4, m_sHashedString);

        s3 = "ABC";
        s4 = "abc";
        PL_TEST_BOOL(s3 < s4);
    }

    // plStringBuilder
    {
        plStringBuilder sb1;
        plStringBuilder sb2b("T", "e");
        plStringBuilder sb2("T", "e", "s", "t");
        plStringBuilder sb3(m_sString);
        plStringBuilder sb4(m_sHashedString);
        plStringBuilder sb5(sb2);

        PL_TEST_BOOL(sb1.IsEmpty());
        PL_TEST_STRING(sb2b, "Te");
        PL_TEST_STRING(sb2, "Test");
        PL_TEST_STRING(sb3, m_sString);
        PL_TEST_STRING(sb4, m_sHashedString);
        PL_TEST_STRING(sb5, sb2);

        sb1.SetFormat("a {} {} {} {}", 1, "b", 2.3f, plTime::Seconds(7));
        PL_TEST_STRING(sb1, "a 1 b 2.3 7sec");
    }

    // plHashedString
    {
        plHashedString hs1();
        PL_TEST_BOOL(hs1.IsEmpty());

        plHashedString hs2("Hallo");
        PL_TEST_STRING(hs2, "Hallo");
        PL_TEST_BOOL(!hs2.IsEmpty());

        plHashedString hs3(hs2);
        PL_TEST_STRING(hs2, hs3);
        PL_TEST_BOOL(hs2 == hs3);

        hs1.Assign("Text");
        PL_TEST_STRING(hs1, "Text");
        PL_TEST_BOOL(hs1 != hs3);

        hs1 = "Bla";
        PL_TEST_STRING(hs1, "Bla");
        PL_TEST_BOOL(hs1 == "Bla");

        hs1 = m_sString;
        PL_TEST_STRING(hs1,  m_sString);
        PL_TEST_BOOL(!hs1.IsEmpty());
        hs1.Clear();
        PL_TEST_BOOL(hs1.IsEmpty());

        plTempHashedString ths = "Hallo";
        PL_TEST_BOOL(hs2 == ths);
    }

    // plTempHashedString
    {
        plTempHashedString ths1;
        PL_TEST_BOOL(ths1.IsEmpty());

        plTempHashedString ths2(svLong);
        plHashedString hs1(svLong);
        PL_TEST_BOOL(hs1 == ths2);

        plTempHashedString ths3(ths2);
        PL_TEST_BOOL(ths2 == ths3);

        plTempHashedString ths4(hs1);
        PL_TEST_BOOL(ths4 == hs1);

        ths4 = "Guck guck";
        PL_TEST_BOOL(ths4 != hs1);
        PL_TEST_BOOL(!ths4.IsEmpty());

        ths4 = hs1;
        PL_TEST_BOOL(ths4 == hs1);
        ths4.Clear();
        PL_TEST_BOOL(ths4.IsEmpty());
    }
}