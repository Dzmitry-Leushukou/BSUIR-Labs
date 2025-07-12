using SQLite;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab_1.Entities
{
    [Table("Routes")]
    public class Route
    {
        [PrimaryKey, AutoIncrement, Indexed]
        public int Id { get; set; }
        public string Name { get; set; }
        [Indexed]   
        public double Duration {  get; set; }
        
    }
}
