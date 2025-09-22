using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab1.Domain.Entities
{
    public class Car
    {
        public int Id { get; set; }
        public string Name { get; set; }
        
        public string? Description {  get; set; }
        public Category? Category { get; set; }
        public double Price {  get; set; }
        public string? ImagePath {get; set; }
        public string? Mime {  get; set; }
    }
}
