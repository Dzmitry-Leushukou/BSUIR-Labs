using Lab_6.Interfaces;
using System.Text.Json;
namespace ClassLibrary
{
    public class FileService <T> : IFileService<T> where T : class
    {
        public IEnumerable<T> ReadFiles(string fileName)
        {
            using var fs = new FileStream(fileName, FileMode.Open);
            return JsonSerializer.Deserialize<IEnumerable<T>>(fs);
        }
        public void SaveData(IEnumerable<T> data, string fileName)
        {
            using var fs = new FileStream(fileName, FileMode.Open);
            JsonSerializer.Serialize(fs, data);
        }

    }
}
