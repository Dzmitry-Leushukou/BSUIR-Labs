public class NoDiscount : IPrice
{
    private decimal price;
    public decimal Price { get { return price; } set { price = value; } }

    decimal IPrice.GetPrice() => price;

    public NoDiscount(decimal price)
    {
        Price = price;
    }
}