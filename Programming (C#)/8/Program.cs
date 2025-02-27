internal class Program
{
    private static void Main(string[] args)
    {
        Airport airport = new Airport();

        Tariff tariff1 = new Tariff("First", 12131);
        Tariff tariff2 = new Tariff("Second", 100000,50);

        airport.AddTariff(tariff1);
        airport.AddTariff(tariff2);

        airport.GetMaxPrice();

        Tariff tariff3 = new Tariff("Third", 23);
        Tariff tariff4 = new Tariff("sadd", 10, 50);

        airport.AddTariff(tariff3);
        airport.AddTariff(tariff4);

        airport.GetMaxPrice();

        Tariff tariff5 = new Tariff("gfg", 1000000);
        Tariff tariff6 = new Tariff("qwe", 1000000000, 50);

        airport.AddTariff(tariff5);
        airport.AddTariff(tariff6);

        airport.GetMaxPrice();

        Tariff tariff7 = new Tariff("adsd", 1);
        Tariff tariff8 = new Tariff("bvbcv", 89, 50);

        airport.AddTariff(tariff7);
        airport.AddTariff(tariff8);

        airport.GetMaxPrice();
    }
}