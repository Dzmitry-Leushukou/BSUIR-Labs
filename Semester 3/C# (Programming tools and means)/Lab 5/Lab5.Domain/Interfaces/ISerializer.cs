using Lab5.Domain.Entitites;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab5.Domain.Interfaces
{
    public interface ISerializer
    {
        IEnumerable<Driver> DeSerializeByLINQ(string fileName);
        IEnumerable<Driver> DeSerializeXML(string fileName);
        IEnumerable<Driver> DeSerializeJSON(string fileName);
        void SerializeByLINQ(IEnumerable<Driver> xxx, string fileName);
        void SerializeXML(IEnumerable<Driver> xxx, string fileName);
        void SerializeJSON(IEnumerable<Driver> xxx, string fileName);
    }
}
