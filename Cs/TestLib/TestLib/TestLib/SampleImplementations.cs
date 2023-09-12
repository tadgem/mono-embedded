using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class OnUpdate1 : IAspects.IOnUpdate
{
    public void Update(float delta)
    {
        Console.WriteLine($"OnUpdate1 : delta time : {delta}");
    }
}

public class OnUpdate2 : IAspects.IOnUpdate
{
    public void Update(float delta)
    {
        Console.WriteLine($"OnUpdate2 : delta time : {delta}");
    }
}

public class OnUpdate3 : IAspects.IOnUpdate
{
    public void Update(float delta)
    {
        Console.WriteLine($"OnUpdate3 : delta time : {delta}");
    }
}

public class OnInit1 : IAspects.IOnInit
{
    public void Init()
    {
        Console.WriteLine($"OnInit1 : Called!");
    }
}

public class OnInit2 : IAspects.IOnInit
{
    public void Init()
    {
        Console.WriteLine($"OnInit2 : Called!");
    }
}

public class OnInit3 : IAspects.IOnInit
{
    public void Init()
    {
        Console.WriteLine($"OnInit3 : Called!");
    }
}

public class OnStart1 : IAspects.IOnStart
{
    public void Start()
    {
        Console.WriteLine($"OnStart1 : Called!");
    }
}

public class OnStart2 : IAspects.IOnStart
{
    public void Start()
    {
        Console.WriteLine($"OnStart2 : Called!");
    }
}

public class OnStart3 : IAspects.IOnStart
{
    public void Start()
    {
        Console.WriteLine($"OnStart3 : Called!");
    }
}

public class OnCleanup1 : IAspects.IOnCleanup
{
    public void Cleanup()
    {
        Console.WriteLine($"OnCleanup1 : Called!");
    }
}

public class OnCleanup2 : IAspects.IOnCleanup
{
    public void Cleanup()
    {
        Console.WriteLine($"OnCleanup2 : Called!");
    }
}

public class OnCleanup3 : IAspects.IOnCleanup
{
    public void Cleanup()
    {
        Console.WriteLine($"OnCleanup3 : Called!");
    }
}
