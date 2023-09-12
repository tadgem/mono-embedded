using System;
using System.Runtime.CompilerServices;

public class TestLib
{
    public float PublicFloat = 1.0f;

    private string _name = "Johnny B. Goode";
    public string Name { 
        get 
        { 
            return _name; 
        }
        set
        {
            _name = value;
            IncrementFloatVar(5.0f);
        }
    }

    public void PrintFloatVar()
    {
        Console.WriteLine($"PublicFloat Value : {PublicFloat}");
    }

    public void IncrementFloatVar(float value)
    {
        PublicFloat += value;
    }

    public void TestInternalCalls()
    {
        PrintString();
        IntPtr instance1 = CreateTestClassDefault();
        IntPtr instance2 = CreateTestClass(2305);
        Console.WriteLine($"Instance 1 Number : {GetTestClassNumber(instance1)}");
        Console.WriteLine($"Instance 2 Number : {GetTestClassNumber(instance2)}");
    }

    public void CreateInterfaceImpls()
    {
        var u1 = new OnUpdate1();
        var u2 = new OnUpdate2();
        var u3 = new OnUpdate3();

        var i1 = new OnInit1();
        var i2 = new OnInit2();
        var i3 = new OnInit3();

        var s1 = new OnStart1();
        var s2 = new OnStart2();
        var s3 = new OnStart3();

        var c1 = new OnCleanup1();
        var c2 = new OnCleanup2();
        var c3 = new OnCleanup3();
    }

    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    extern static void PrintString();

    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    extern static IntPtr CreateTestClassDefault();

    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    extern static IntPtr CreateTestClass(int arg);

    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    extern static int GetTestClassNumber(IntPtr instance);

}