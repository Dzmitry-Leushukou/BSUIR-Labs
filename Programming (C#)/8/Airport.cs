internal class Airport
{
    private List<Tariff> tariffList;
    public void AddTariff(Tariff tariff)
    {
        tariffList.Add(tariff);
    }
    public void GetMaxPrice()
    {
        decimal maxPrice=-1;
        string name="";
        for(int i=0; i<tariffList.Count;i++)
        {
            if (maxPrice < tariffList[i].Price)
            {
                name = tariffList[i].Name;
                maxPrice = tariffList[i].Price;
            }
        }

        if (maxPrice == -1)
            Console.WriteLine("Firstly add tariff");
        else
        Console.WriteLine($"===The most expensive tariff===\nName:{name} Price:{maxPrice}\n");
    }
    public Airport()
    {
        tariffList=new List<Tariff>();
    }
    }
