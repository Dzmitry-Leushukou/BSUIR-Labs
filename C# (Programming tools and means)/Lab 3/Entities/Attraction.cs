using SQLite;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab_1.Entities
{
    
    [Table("Attractions")]
    public class Attraction
    {
        [PrimaryKey, AutoIncrement, Indexed]
         public int Id { get; set; }
         public string Name { get; set; }
         [Indexed]
         public double Rating { get; set; }
         [Indexed]
         public int RouteID {  get; set; }
    }
    
}
