using Lab1.Domain.Entities;
using Lab1.Domain.Models;
using Lab1.Extensions;

namespace Lab1.Services
{
    /// <summary>
    /// Корзина, которая сохраняет данные в сессию
    /// </summary>
    public class SessionCart : Cart
    {
        private readonly ISession _session;
  private const string CartKey = "cart";

        public SessionCart(ISession session)
        {
 _session = session ?? throw new ArgumentNullException(nameof(session));
     
            // Загружаем корзину из сессии при инициализации
            var cart = GetCartFromSession();
            CartItems = cart?.CartItems ?? new Dictionary<int, CartItem>();
        }

   public override void AddToCart(Car car)
        {
    base.AddToCart(car);
     SaveCartToSession();
        }

        public override void RemoveItems(int id)
   {
            base.RemoveItems(id);
     SaveCartToSession();
        }

     public override void ClearAll()
        {
    base.ClearAll();
       SaveCartToSession();
        }

        private void SaveCartToSession()
        {
            _session.Set<Cart>(CartKey, this);
        }

        private Cart? GetCartFromSession()
        {
    return _session.Get<Cart>(CartKey);
        }
    }
}
