using Main;
internal class Luxe : Housing, IInside, IOutside
{
    public Luxe(string name, Type type, IHousing house) : base(name, type, house) { }
    public override void GetInfo()
    {
        Console.WriteLine("Name: " + name + "\nClass: Luxe\nType: " + type);
        AddInside();
        AddOutside();
    }

    public void AddInside()
    {
        Console.Write("Additional functions: ");
        if (type == Type.Hostel)
            Console.WriteLine("Internet, Minibar");
        else if (type == Type.Hotel)
            Console.WriteLine("Internet, Minibar, Swimming pool");
        else if (type == Type.Farmstead)
            Console.WriteLine("Minibar, Swimming pool");
        else
            Console.WriteLine("Internet");
    }
    public void AddOutside()
    {
        Console.Write("Transfer: ");
            Console.WriteLine("Private transfer");
    }

}

