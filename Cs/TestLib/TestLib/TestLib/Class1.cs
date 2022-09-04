using System;

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

}