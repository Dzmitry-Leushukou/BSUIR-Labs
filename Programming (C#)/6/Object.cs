using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _1
{
    abstract class Object
    {
        public Object()
        {
            size = distance = 0;
            name = "";
        }
        public Object(double size,double distance, string name)
        {
            this.name = name;
            this.size = size;
            this.distance = distance;
        }
        private double size;
        public double Size
        {
            get { return size; }
            set { size = value; }   
        }
        private double distance;
        public double Distance
        {
            get { return distance; } 
            set { distance = value; }
        }
        private string name;
        public string Name
        {
            get { return name; }
            set { name = value; }
        }
        public double LightSignalTime()
        {
            return distance/(299792.458);
        }
        public virtual string MainInfo()
        {
            return $"Name: {name} \tSize: {size} \tDistance : {distance} km";
        }

    }
}
