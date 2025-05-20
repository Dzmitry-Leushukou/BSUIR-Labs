using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab_6.Interfaces
{
    public interface IFileService<T> where T : class
    {
        IEnumerable<T> ReadFiles(string fileName);
        void SaveData(IEnumerable<T> data, string fileName);
    }
}
