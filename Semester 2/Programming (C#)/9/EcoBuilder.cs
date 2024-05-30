using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace Main
{
    internal class EcoBuilder:AbstractBuilder
    {
        public override Housing Build()
        {
            var eco = new Eco(name, type, house);
            if (house != null)
                eco.SetHouse(house);
            return eco;
        }
    }
}
