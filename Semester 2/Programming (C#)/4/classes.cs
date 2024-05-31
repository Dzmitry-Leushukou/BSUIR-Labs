using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace main
{
    public class Cargo
    {
        private static Cargo? _instance;
        public static Cargo GetInstance()
        {
            if(_instance == null)
             _instance = new Cargo();
            return _instance;
        }
        private double w;
        public double weight
        {
            set { this.w = value; }
            get { return this.w; }
        }
        private string name;
        public string Name
        {
            set { this.name = value; }
            get { return this.name; }
        }
        private Tariff tariff;
        public Tariff Tariff 
        { get
            {
             return this.tariff;
            } 
        }
        public Cargo()
        {
            tariff = new Tariff();
            w = 0;
            name = "";
        }
        public void change_tariff(double delta)
        {
            tariff.change_cost(delta);
        }

        public void total()
        {
            Console.WriteLine(w * tariff.Cost); //lox
        }
    }
    public class Tariff
    {
        private double cost;
        public double Cost { get { return cost; } set { this.cost = value; } }
        public void change_cost(double delta)
        {
            cost += delta;
            if(cost<0) cost = 0; 
        }
        public Tariff()
        {
            cost = 0;
        }
    }
    
}
