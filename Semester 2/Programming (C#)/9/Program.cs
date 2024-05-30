using Main;

internal class Program
{
    private static void Main(string[] args)
    {
        var houses = new List<Housing>();
        var hostel = new MiddleBuilder();
        var motel = new EcoBuilder();
        var farmstead = new LuxeBuilder();
        var hotel = new LuxeBuilder();

        houses.AddRange(new Housing[] {
                Director.GetHostelCard("Hostel #1",hostel),
                Director.GetHotel("Hotel #1",hostel),
                Director.GetHotelCard("Hotel #2",hotel),
                Director.GetMotel("Motel #1",motel),
                Director.GetFarmsteadBank("Farmstead #1",farmstead),
                Director.GetFarmstead("Farmstead #2",motel),
            });
        foreach (var house in houses)
        {
            house.GetInfo();
            Console.WriteLine('\n');
        }
    }
}