using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace Main
{
    internal class MiddleBuilder : AbstractBuilder
    {
        public override Housing Build()
        {
            var middle = new Middle(name, type, house);
            if (house != null)
                middle.SetHouse(house);
            return middle;
        }
    }
}
