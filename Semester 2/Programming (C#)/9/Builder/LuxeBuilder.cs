﻿using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace Main
{
    internal class LuxeBuilder : AbstractBuilder
    {
        public override Housing Build()
        {
            var luxe = new Luxe(name, type, house);
            if (house != null)
                luxe.SetHouse(house);
            return luxe;
        }
    }
}
