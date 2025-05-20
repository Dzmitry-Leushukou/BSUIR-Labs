
internal class Eco:Housing,IInside
    {
        public Eco(string name, Type type, IHousing house):base(name,type, house) { }
        public override void GetInfo()
        {
        Console.WriteLine("Name: " + name + "\nClass: Economy\nType: " + type);
        AddInside();
    }

    public void AddInside()
    {
        Console.Write("Additional functions: ");
        if (type == Type.Hostel)
            Console.WriteLine("Internet");
        else if (type == Type.Hotel)
            Console.WriteLine("Internet");
        else
            Console.WriteLine("Nothing");
    }

}

