using Lab;
using System.Net;
internal class Program
{
    private static void write_cargos(List<Client> c)
    {
        foreach (Client client in c) 
        { 
            Console.WriteLine("Name: "+client.Name);
            foreach (Cargo cargo in client.Cargos) {
                Console.WriteLine($"  Weight: {cargo.wei} Tariff: {cargo.tariff.Cost}");
            }
            Console.WriteLine($" = {client.get_total()}");
        }
    }

    private static void write_total(double total)
    {
        Console.WriteLine("======================");
        Console.WriteLine($"Total cost: {total}");
    }
    private static void Main(string[] args)
    {
        Company cmp= new Company();
        
        cmp.add_tarrif(1.1);
        cmp.add_tarrif(3.0);
        cmp.add_tarrif(10.1);
        cmp.add_tarrif(100);
       
        cmp.reg_user("Das");
        cmp.reg_user("Qas");
        cmp.reg_user("Ras");
        cmp.reg_user("Has");
        cmp.reg_user("Uas");

        cmp.add_cargo(0, 0, 1);
        cmp.add_cargo(0, 3, 2);
        cmp.add_cargo(0, 1, 3);

        cmp.add_cargo(1, 0, 3);
        cmp.add_cargo(1, 3, 0);

        cmp.add_cargo(2, 2, 3);
        cmp.add_cargo(2, 3, 3);
        cmp.add_cargo(2, 1, 3);

        cmp.add_cargo(3, 0, 1);
        cmp.add_cargo(3, 1, 2);
        cmp.add_cargo(3, 2, 3);

        cmp.add_cargo(4, 2, 1);
        cmp.add_cargo(4, 3, 0);
        cmp.add_cargo(4, 1, 0);

        write_cargos(cmp.Clients);
        write_total(cmp.profit());
    }
}