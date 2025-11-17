using Lab1.Domain.Entities;

namespace Lab1.Domain.Models
{
    /// <summary>
    /// Класс для работы с корзиной заказов
  /// </summary>
    public class Cart
    {
     /// <summary>
    /// Список объектов в корзине
        /// key - идентификатор автомобиля
        /// </summary>
        public Dictionary<int, CartItem> CartItems { get; set; } = new();

        /// <summary>
        /// Добавить объект в корзину
 /// </summary>
        /// <param name="car">Добавляемый автомобиль</param>
        public virtual void AddToCart(Car car)
        {
  if (car == null)
          throw new ArgumentNullException(nameof(car));

            if (CartItems.ContainsKey(car.Id))
            {
                CartItems[car.Id].Count++;
     }
        else
        {
                CartItems[car.Id] = new CartItem(car);
      }
        }

        /// <summary>
        /// Удалить объект из корзины
        /// </summary>
        /// <param name="id">id удаляемого объекта</param>
        public virtual void RemoveItems(int id)
        {
       CartItems.Remove(id);
        }

  /// <summary>
        /// Очистить корзину
      /// </summary>
        public virtual void ClearAll()
      {
    CartItems.Clear();
        }

/// <summary>
        /// Количество объектов в корзине
    /// </summary>
   public int Count => CartItems.Sum(item => item.Value.Count);

/// <summary>
        /// Общая стоимость товаров в корзине
        /// </summary>
        public double TotalPrice => CartItems.Sum(item => item.Value.Car.Price * item.Value.Count);
    }
}
