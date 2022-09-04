using System;

public class TestLib
{
    public float PublicFloat = 1.0f;

    public void PrintFloatVar()
    {
        Console.WriteLine($"PublicFloat Value : {PublicFloat}");
    }

    public void IncrementFloatVar(float value)
    {
        PublicFloat += value;
    }

}