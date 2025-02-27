using LoremNET;

internal class Program
{
    private static async Task Main(string[] args)
    {
        Console.WriteLine($"Primary thread: {Thread.CurrentThread.ManagedThreadId}");
        List<Listener> listeners = new();
        Progress<string> progress = new(Console.WriteLine);
        var stream = new MemoryStream();
        var ss = new StreamService<Listener>();

        for (int i = 0; i < 1000; i++)
        {
            listeners.Add(new Listener()
            {
                Id = listeners.Count(),
                Name = Lorem.Words(1),
                CourseCount = Convert.ToInt32(Lorem.Number(1, 256))
            });
        }

        var op1 = ss.WriteToStreamAsync(stream, listeners, progress);
        await Task.Delay(200);
        var op2 = Task.Run(()=>ss.CopyFromStreamAsync(stream, "data.json", progress));
        Task.WaitAll(op1, op2);
        var ans = ss.GetStatisticsAsync("data.json", (listener) => listener.CourseCount > 10);

    }
}