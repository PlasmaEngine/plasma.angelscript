#include "../TestFramework.as"

void ExecuteTests()
{
    // typedef to basic type
    {
        plUInt32 test = 5;
        array<plUInt32> elements;

        PL_TEST_BOOL(elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 0);
        PL_TEST_BOOL(!elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), -1);
        elements.PushBack(test);
        PL_TEST_BOOL(!elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 1);
        PL_TEST_BOOL(elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), 0);
        PL_TEST_BOOL(elements[0] == test);
    }

    // pod value type
    {
        plGameObjectHandle test;
        array<plGameObjectHandle> elements;

        PL_TEST_BOOL(elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 0);
        PL_TEST_BOOL(!elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), -1);
        elements.PushBack(test);
        PL_TEST_BOOL(!elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 1);
        PL_TEST_BOOL(elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), 0);
        PL_TEST_BOOL(elements[0] == test);
    }

    // Bigger pod value type
    {
        plVec4 test(1.0f, 2.0f, 3.0f, 4.0f);
        plVec4 test2(5.0f, 6.0f, 7.0f, 8.0f);
        array<plVec4> elements;

        PL_TEST_BOOL(elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 0);
        PL_TEST_BOOL(!elements.Contains(test));
        PL_TEST_BOOL(!elements.Contains(test2));
        PL_TEST_INT(elements.IndexOf(test), -1);
        PL_TEST_INT(elements.IndexOf(test2), -1);

        elements.PushBack(test);
        PL_TEST_BOOL(!elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 1);
        PL_TEST_BOOL(elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), 0);
        PL_TEST_BOOL(elements[0] == test);
        PL_TEST_BOOL(elements[0] != test2);

        elements.PushBack(test2);
        PL_TEST_BOOL(!elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 2);
        PL_TEST_BOOL(elements.Contains(test));
        PL_TEST_BOOL(elements.Contains(test2));
        PL_TEST_INT(elements.IndexOf(test), 0);
        PL_TEST_INT(elements.IndexOf(test2), 1);
        PL_TEST_BOOL(elements[0] == test);
        PL_TEST_BOOL(elements[1] == test2);
        PL_TEST_BOOL(elements[0] != test2);
        PL_TEST_BOOL(elements[1] != test);

        elements.PushBack(plVec4(9.0f, 10.0f, 11.0f, 12.0f));
        PL_TEST_BOOL(elements.Contains(plVec4(9.0f, 10.0f, 11.0f, 12.0f)));
        PL_TEST_BOOL(elements[2] == plVec4(9.0f, 10.0f, 11.0f, 12.0f));
    }

    // non-pod value type
    {
        plString test = "Test";
        array<plString> elements;

        PL_TEST_BOOL(elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 0);
        PL_TEST_BOOL(!elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), -1);
        elements.PushBack(test);
        PL_TEST_BOOL(!elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 1);
        PL_TEST_BOOL(elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), 0);
        PL_TEST_BOOL(elements[0] == test);
    }

    // plStringView
    {
        plStringView test = "Test";
        array<plStringView> elements;

        PL_TEST_BOOL(elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 0);
        PL_TEST_BOOL(!elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), -1);
        elements.PushBack(test);
        PL_TEST_BOOL(!elements.IsEmpty());
        PL_TEST_BOOL(elements.GetCount() == 1);
        PL_TEST_BOOL(elements.Contains(test));
        PL_TEST_INT(elements.IndexOf(test), 0);
        PL_TEST_BOOL(elements[0] == test);
    }
}