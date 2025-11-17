using Lab1.Domain.Entities;

namespace Lab1.Domain.Models
{
    /// <summary>
 /// Элемент корзины заказов
    /// </summary>
    public class CartItem
    {
        /// <summary>
        /// Автомобиль в корзине
 /// </summary>
        public Car Car { get; set; } = null!;

        /// <summary>
  /// Количество экземпляров в корзине
      /// </summary>
        public int Count { get; set; }

        public CartItem() { }

        public CartItem(Car car)
        {
      Car = car;
       Count = 1;
 }
    }
}
