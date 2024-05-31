using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab
{
    internal class Client
    {
        private string? name;
        public string Name
        {
            get
            {
                return name;
            }
            set { name = value; }
        }

        private List<Cargo> cargoList = new List<Cargo>();
        public List<Cargo> Cargos
        {
            get
            {
                return cargoList;
            }
        }
        public Client(string name)
        {
            this.name = name;
        }
        public double get_total()
        {
            double sum = 0;
            for(int  i = 0;i<cargoList.Count;i++)
            {
                sum += cargoList[i].wei * cargoList[i].tariff.Cost;
            }
            return sum;
        }

    }
}
