using Main;

internal class Middle : Housing, IInside,IOutside
{
    public Middle(string name, Type type, IHousing house) : base(name, type, house) { }
    public override void GetInfo()
    {
        Console.WriteLine("Name: "+name+"\nClass: Middle\nType: " + type);
        AddInside();
        AddOutside();
    }

    public void AddInside()
    {
        Console.Write("Additional functions: ");
        if (type == Type.Hotel)
            Console.WriteLine("Internet, Minibar");
        else if (type == Type.Farmstead)
            Console.WriteLine("Minibar");
        else
             Console.WriteLine("Internet");
    }
    public void AddOutside()
    {
        Console.Write("Transfer: ");
        if (type == Type.Hostel)
            Console.WriteLine("Public transfer");
        else
            Console.WriteLine("No");
    }
}

