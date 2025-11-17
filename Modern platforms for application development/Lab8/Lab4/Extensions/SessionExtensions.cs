using System.Text.Json;

namespace Lab1.Extensions
{
    /// <summary>
    /// Расширяющие методы для работы с сессией
    /// </summary>
    public static class SessionExtensions
    {
 /// <summary>
        /// Получить объект из сессии
        /// </summary>
   public static T? Get<T>(this ISession session, string key) where T : class
        {
         var data = session.TryGetValue(key, out byte[]? value);
         if (!data || value == null) return null;

        try
    {
                var json = System.Text.Encoding.UTF8.GetString(value);
   return JsonSerializer.Deserialize<T>(json);
     }
            catch
     {
         return null;
}
       }

      /// <summary>
   /// Установить объект в сессию
       /// </summary>
       public static void Set<T>(this ISession session, string key, T value) where T : class
        {
        if (value == null)
      {
            session.Remove(key);
          return;
         }

    try
       {
    var json = JsonSerializer.Serialize(value);
        var data = System.Text.Encoding.UTF8.GetBytes(json);
  session.Set(key, data);
      }
       catch
 {
  // Логирование ошибки сериализации
    }
     }
    }
}
