using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace _1
{
    sealed class Star : Object
    {
        public Star(string name, double size, double distance, double direction, double speed, double Brightness) : base(size,distance, name)
        {
            
            Direction = direction;
            Speed = speed;
            Brightness = brightness;
        }
        private double direction, speed, brightness;
        public double Direction { get { return direction; } set {  direction = value; } }   
        public double Speed { get { return speed; } set {  speed = value; } }
        
        public double Brightness { get { return brightness; } set { brightness = value; } } 

        public string GoToEarth()
        {
            if(direction == 0) return $"Will fly to Earth through {Distance / speed} seconds"; 
            if (direction <180||direction>270) return "Will near the Earth";
            return $"The star is moving away from the Earth at a speed : {speed} km/s";
        }
        public string Visible()
        {
            double time = 0;
             if(direction == 0)
            {
                time = Distance / speed;
            }
            if (direction < 180 || direction > 270)
            {
                time += Distance / speed +  Math.Sqrt(brightness * 1e18)/speed;
            }

            if (direction >= 180 && direction <= 270)
            {
                time +=  (Math.Sqrt(brightness * 1e18) - Distance) / speed;
            }
            if (brightness * 1e18 / (Distance*Distance) >= 1)
            {
                return $"Star will be visible by Earth by next {time} seconds";
            }
            else return "Star is ivisible :(";
        }
        public override string MainInfo()
        {
            return $"Name: {Name} \tSize: {Size} \tBrightness: {brightness} \tDirection: {direction} azimuth \tDistance: {Distance} km \tSpeed: {speed} km/s \tLight signal go in {LightSignalTime()} seconds";
        }
    }
}
