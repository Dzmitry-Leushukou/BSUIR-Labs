using Lab5.Domain.Entitites;
using Lab5.Domain.Interfaces;
using System.Text.Json;
using System.Xml.Linq;
using System.Xml.Serialization;

namespace SerializerLib
{
    public class Serializer:ISerializer
    {
        public IEnumerable<Driver> DeSerializeByLINQ(string fileName)
        {
            var doc = XDocument.Load(fileName);
            var element = doc.Element("Drivers");

            foreach(var el in element.Elements("Driver"))
            {
                var name = el.Attribute("name").Value;
                var id = el.Attribute("id").Value;
                var bus = el.Element("bus");
                var bid = bus.Attribute("bus_id").Value;
                var bcapacity = bus.Attribute("bus_capacity").Value;
                var bmodel = bus.Attribute("bus_model").Value;
                yield return new Driver(name, id, new Bus(bid,bmodel,int.Parse(bcapacity)));
            }
        }
        public IEnumerable<Driver> DeSerializeXML(string fileName)
        {
            var Serializer=new XmlSerializer(typeof(List<Driver>));
            using var stream = new FileStream(fileName, FileMode.OpenOrCreate);
            var drivers=Serializer.Deserialize(stream) as IEnumerable<Driver>;
            return drivers;
        }
        public IEnumerable<Driver> DeSerializeJSON(string fileName)
        {
            using var stream = new FileStream(fileName, FileMode.OpenOrCreate);
            var drivers = JsonSerializer.Deserialize<IEnumerable<Driver>>(stream);
            return drivers;
        }
        public void SerializeByLINQ(IEnumerable<Driver> drivers, string fileName)
        {
            var doc = new XDocument();
            var element = new XElement("Drivers");
            
            foreach(var d in drivers)
            {
                var driver = new XElement("Driver");
                driver.Add(new XAttribute("name", d.Name));
                driver.Add(new XAttribute("id", d.Id));
                driver.Add(new XElement("bus", 
                new XAttribute("bus_id",d.Bus.Id),
                new XAttribute("bus_capacity", d.Bus.Capacity),
                new XAttribute("bus_model", d.Bus.Model)
                ));

                element.Add(driver);    
            }

            doc.Add(element);
            doc.Save(fileName);
        }
        public void SerializeXML(IEnumerable<Driver> drivers, string fileName)
        {
            var serializer=new XmlSerializer(drivers.GetType());
            using var stream = new FileStream(fileName, FileMode.OpenOrCreate);
            serializer.Serialize(stream, drivers);
        }
        public void SerializeJSON(IEnumerable<Driver> drivers, string fileName)
        {
            using var stream = new FileStream(fileName, FileMode.OpenOrCreate);
            JsonSerializer.Serialize(stream, drivers);
        }
    }
}
