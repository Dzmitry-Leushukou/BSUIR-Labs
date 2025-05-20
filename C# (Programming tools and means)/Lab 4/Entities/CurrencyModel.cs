using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab_1.Entities
{
    internal class CurrencyModel
    {
        public string Name { get; set; }
        public decimal Scale { get; set; }
        public decimal? Currency { get; set; }
        public string FullName {  get; set; }
        public string Alignment 
        { 
            get;
            set; 
        }
        
        public void set_alig()
        {
            string s = new string("             ");
            int count = Name.Length + Scale.ToString().Length;
            Alignment = s.Substring(0, s.Length - count);
        }
        
    }
}
