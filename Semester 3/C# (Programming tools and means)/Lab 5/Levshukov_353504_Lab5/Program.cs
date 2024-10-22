using SerializerLib;
using Lab5.Domain.Entitites;
using Microsoft.Extensions.Configuration;
using Lab5.Domain.Interfaces;
using System.IO;
internal class Program
{
    private static void Main(string[] args)
    {

        var configuration = new ConfigurationBuilder().SetBasePath(Directory.GetCurrentDirectory()).AddJsonFile("appsettings.json", optional: false, reloadOnChange: true).Build();

        var Drivers = new List<Driver>
        {
            new("Alex","12",new Bus("AC5686","Mx",100)),
            new("John","34",new Bus("HQ8787","Mx",100)),
            new("Alan","56",new Bus("BX7476","Ni",50)),
            new("Borat","7772",new Bus("OP2612","Ni",50)),
            new("Gleb","8888888",new Bus("LK1111","Ma",75))
        };

        var filename = configuration["FileName"];
        var linq_filename = filename + "LINQ.xml";
        var xml_filename = filename + ".xml";
        var json_filename = filename + ".json";

        var serializer = new Serializer();
        serializer.SerializeByLINQ(Drivers, linq_filename);
        serializer.SerializeXML(Drivers, xml_filename);
        serializer.SerializeJSON(Drivers, json_filename);

        Console.WriteLine("Serialized");


        int index = 0;
        bool flag = false;
        Console.WriteLine("Check LINQ");
        
        var fd = serializer.DeSerializeByLINQ(linq_filename);
        foreach(var d in fd)
        {
         
            if (d.Equals(Drivers[index]))
                Console.WriteLine($"[{index}] OK");
            else
                Console.WriteLine($"[{index}] Wrong");
            index++;
        }

        index = 0;

        Console.WriteLine("Check XML");

        using var xml = serializer.DeSerializeXML(xml_filename).GetEnumerator();
        foreach (var d in Drivers)
        {
            xml.MoveNext();
            if (d.Equals(xml.Current))
                Console.WriteLine($"[{index}] OK");
            else
                Console.WriteLine($"[{index}] Wrong");
            index++;
            
        }


        index = 0;

        Console.WriteLine("Check JSON");

        using var json = serializer.DeSerializeJSON(json_filename).GetEnumerator();
        foreach (var d in Drivers)
        {

            json.MoveNext();
            if (d.Equals(json.Current))
                Console.WriteLine($"[{index}] OK");
            else
                Console.WriteLine($"[{index}] Wrong");
            index++;
            
        }





    }

}