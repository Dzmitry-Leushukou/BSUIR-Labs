public class Discount : IPrice
{
    private decimal price, discount;
    public decimal Price { get { return price; } set { price = value; } }
    public decimal Discount_{get{ return discount; } set { discount = value; }}
    
    decimal IPrice.GetPrice()
    {
        return price * (100 - discount) / 100;
    }

    public Discount(decimal price, decimal discount)
    {
        this.price = price;
        this.discount = discount;
    }
}