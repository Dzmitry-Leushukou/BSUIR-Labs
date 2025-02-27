using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json;
public class StreamService<T>
    {
    private static Semaphore sem = new Semaphore(1,1);
        public async Task WriteToStreamAsync(Stream stream, IEnumerable<T> data, IProgress<string> progress)
        {
            sem.WaitOne();
            await JsonSerializer.SerializeAsync(stream, data);
            progress.Report($"Thread:{Thread.CurrentThread.ManagedThreadId} start writting...");
            for (int i = 0; i < 100; i++)
            {
                Thread.Sleep(30);
                if(i%5==0)
                progress.Report($"Thread:{Thread.CurrentThread.ManagedThreadId} writting... [{i}/100]");
            }
            progress.Report($"Thread:{Thread.CurrentThread.ManagedThreadId} stoped writting");

            sem.Release();
        }
        public async Task CopyFromStreamAsync(Stream stream, string filename, IProgress<string> progress)
        {
            sem.WaitOne();
            progress.Report($"Thread:{Thread.CurrentThread.ManagedThreadId} start copy...");
            await using FileStream fstream = new FileStream(filename, FileMode.Create, FileAccess.Write);
            await stream.CopyToAsync(fstream);
            progress.Report($"Thread:{Thread.CurrentThread.ManagedThreadId} ended copy");
            sem.Release();
    }
        public async Task<int> GetStatisticsAsync(string fileName, Func<T, bool> filter)
        {
            await using var stream = new FileStream(fileName, FileMode.Open);
            var data = await JsonSerializer.DeserializeAsync<List<T>>(stream);
            return data!.Where(filter).Count();
        }

}