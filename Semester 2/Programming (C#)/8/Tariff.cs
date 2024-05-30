    internal class Tariff
    {
    private IPrice ip;
    string name;
    public string Name { get { return name; } set { name = value; } }
    public decimal Price { get { return ip.GetPrice(); } }

    

    public Tariff(string name, decimal price, decimal discount=0)
    {
        if(discount==0)
        {
            ip=new NoDiscount(price);
        }
        else
            ip = new Discount(price, discount);

        this.name = name;
    }
    }

