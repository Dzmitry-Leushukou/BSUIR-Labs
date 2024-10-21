using Lab.Interfaces;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab
{
    public class FileService : IFileService<Listener>
    {
        public IEnumerable<Listener> ReadFile(string fileName)
        {
            using var f = File.OpenRead(fileName);
            using var binReader = new BinaryReader(f);
            string name = "";
            int age = 0;
            bool online = false;
            while (binReader.PeekChar() != -1)
            {
                try
                {
                    name = binReader.ReadString();
                    age = binReader.ReadInt32();
                    online = binReader.ReadBoolean();
                }
                catch
                {
                    Console.WriteLine("Couldn't read file");
                    yield break;
                }
                yield return new Listener(name, age, online);
            }
            
        }

        public void SaveData(IEnumerable<Listener> data, string fileName)
        {
            using var f = File.OpenWrite(fileName);
            using var binWriter = new BinaryWriter(f);
            foreach (var i in data)
            {
                try
                {
                    binWriter.Write(i.Name);
                    binWriter.Write(i.Age);
                    binWriter.Write(i.Online);
                }
                catch
                {
                    Console.WriteLine("Couldn't write to file");
                }
            }
            binWriter.Close();
            f.Close();
        }
    }
}
