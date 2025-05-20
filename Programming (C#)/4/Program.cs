using main;
namespace main
{
    internal class Program
    {
        public static Cargo cargo;
        public static double get_amount()
        {
            Console.WriteLine("Enter how much you want to change the tariff\n(If you reduce the tariff by a number greater than the tariff itself, then the latter will be equal to 0):");
            double amount;
            if(double.TryParse(Console.ReadLine(), out amount)&&amount!=double.NaN&&amount>=double.MinValue&&amount<=double.MaxValue)
            return amount;
            else
            {
                Console.WriteLine("Wrong input. Try again");
                return get_amount();
            }
        }
        public static void choose(ref int type)
        {
            cargo = Cargo.GetInstance();
            switch (type)
            {
                case 1:
                    cargo.total();
                    break;

                case 2:
                    cargo.change_tariff(get_amount());
                    break;

                case 3:
                    cargo.change_tariff(get_amount() * (-1));
                    break;

                default:
                    Console.WriteLine("Wrong input. Try again");
                    MainMenu();
                    break;
            }
            MainMenu();
        }
        public static void MainMenu()
        {
            Console.WriteLine($"Choose type of operation\n1. Total\n2. Increase tariff\n3. Decrease tariff\nTariff:{cargo.Tariff.Cost}\nWeight:{cargo.weight}");
            
            int type;
            if(int.TryParse(Console.ReadLine(), out type))
            choose(ref type);
            else
            {
                Console.WriteLine("Wrong input. Try again");
                MainMenu();
            }
        }
        public static void Main(string[] args)
        {
            cargo = Cargo.GetInstance();
            cargo.Name = "BlaBlaBla";
            cargo.weight = 100;
            MainMenu();
        }
    }
}