using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab1.Domain.Models
{
    public class ResponseData<T>
    {
        // запрашиваемые данные
        public T? Data { get; set; }
        // признак успешного завершения запроса
        public bool Successfull { get; set; } = true;
        // сообщение в случае неуспешного завершения
        public string? ErrorMessage { get; set; }
        /// <summary>
        /// Получить объект успешного ответа
        /// </summary>
        /// <param name="data">передаваемые данные</param>
        /// <returns></returns>
        public static ResponseData<T> Success(T data)
        {
            return new ResponseData<T> { Data = data };
        }
        /// <summary>
        /// Получение объекта ответа с ошибкой
        /// </summary>
        /// <param name="message">Сообщение об ошибке</param>
        /// <param name="data">Передаваемые данные</param>
        /// <returns></returns>
        public static ResponseData<T> Error(string message,
        T? data = default)
        {
            return new ResponseData<T>
            {
                ErrorMessage = message,
                Successfull = false,
                Data = data
            };
        }

    }
}
