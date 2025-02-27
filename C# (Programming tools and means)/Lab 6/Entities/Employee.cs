using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab_6.Entities
{
    internal class Employee
    {
        public Employee() { }
        public Employee(int salary, bool work, string name) 
        {
            Salary = salary;
            AtWork = work;
            Name = name;
        }
        public int Salary {  get;  set; }
        public bool AtWork {  get;  set; }
        public string Name { get;  set; }
    }
}
