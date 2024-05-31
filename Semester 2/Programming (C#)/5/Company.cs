using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab
{
    internal class Company
    {
        private List<Client> ClientList = new List<Client>();
        private List<Tariff> TariffList = new List<Tariff>();
        public List<Client> Clients 
        {
        get { return ClientList;} 
        }
        
        
        public double profit()
        {
            double sum = 0;
            foreach (Client client in ClientList)
            {
                sum += client.get_total();
            }
            return sum;
        }

        public void reg_user(string username)
        {
            Client client = new Client(username);
            ClientList.Add(client);
        }

        
        public void add_cargo(int index,  int w_type, int tariff_id)
        {            
            Cargo cargo = new Cargo(w_type, TariffList[tariff_id]);
            ClientList[index].Cargos.Add(cargo);
        }
        
        public void add_tarrif(double t)
        {
            TariffList.Add(new Tariff(t));
        }
    }
}
