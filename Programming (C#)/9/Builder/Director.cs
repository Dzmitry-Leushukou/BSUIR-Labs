
internal static class Director
{
    public static Housing GetMotel(string name, AbstractBuilder builder)
    {
        return builder.SetName(name).SetType(Type.Motel).Build();
    }
    public static Housing GetHostel(string name, AbstractBuilder builder)
    {
        return builder.SetName(name).SetType(Type.Hostel).Build();
    }
    public static Housing GetFarmstead(string name, AbstractBuilder builder)
    {
        return builder.SetName(name).SetType(Type.Farmstead).Build();
    }
    public static Housing GetHotel(string name, AbstractBuilder builder)
    {
        return builder.SetName(name).SetType(Type.Hotel).Build();
    }
    public static Housing GetHotelCard(string name, AbstractBuilder builder)
    {
        return builder.SetName(name).SetType(Type.Hotel).SetHouse(new Card()).Build();
    }
    public static Housing GetHostelCard(string name, AbstractBuilder builder)
    {
        return builder.SetName(name).SetType(Type.Hostel).SetHouse(new Card()).Build();
    }
    public static Housing GetFarmsteadBank(string name, AbstractBuilder builder)
    {
        return builder.SetName(name).SetType(Type.Farmstead).SetHouse(new Bank_Transfer()).Build();
    }
}