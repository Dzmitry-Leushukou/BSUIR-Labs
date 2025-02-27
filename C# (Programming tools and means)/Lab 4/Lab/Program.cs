using Lab;

internal class Program
{
    private static void Main(string[] args)
    {
        var directoryInfo = new DirectoryInfo("../../");
        Console.WriteLine(directoryInfo.Name);
        try
        {
            directoryInfo.CreateSubdirectory("Leushukou_LAB4");
        }
        catch
        {
            Console.WriteLine("Already exists");
        }

        foreach (var f in directoryInfo.GetFiles("./Leushukou_LAB4/"))
        {
            Console.WriteLine($"Delete file:{f.Name}");
            f.Delete();
        }
        foreach (var d in directoryInfo.GetDirectories("./Leushukou_LAB4/"))
        {
            Console.WriteLine($"Delete directory:{d.Name}");
            d.Delete();
        }


        var extensions = new List<string> { ".txt", ".rtf", ".dat", ".inf" };
        for (var i = 0; i < 10; i++)
        {
            var filename = "../../Leushukou_LAB4/" + Path.GetRandomFileName() + extensions[i % 4];
            File.Create(filename);
        }

        Console.WriteLine("All files:");
        foreach (var f in directoryInfo.GetFiles("./Leushukou_LAB4/"))
        {
            Console.WriteLine($"Имя файла: {f.Name} , расширение файла: {f.Extension}");
        }

        var listeners = new List<Listener>
        {   
             new ("Alex", 48, true),
             new ("Egor", 21, true),
             new ("Danil", 25, true),
             new ("Dima", 30, false),
             new ("Misha", 40, false),
        };

        var fileService = new FileService();

        File.Create("../../Leushukou_LAB4/listeners.txt").Dispose();
        fileService.SaveData(listeners, "../../Leushukou_LAB4/listeners.txt");

        File.Move("../../Leushukou_LAB4/listeners.txt", "../../Leushukou_LAB4/old_listeners.txt");
        File.Delete("../../Leushukou_LAB4/listeners.txt");

        var newListeners = new List<Listener>();

        foreach (var l in fileService.ReadFile("../../Leushukou_LAB4/old_listeners.txt"))
        {
            Console.WriteLine($"Listener: {l.Name}, age: {l.Age}, online?: {l.Online}");
            newListeners.Add(l);
        }

        var orderedByName = newListeners.OrderBy(l => l, new MyCustomComparer());

        Console.WriteLine("After sorting by Name:");
        foreach (var w in orderedByName)
        {
            Console.WriteLine($"Listener: {w.Name}, age: {w.Age}, online?: {w.Online}");
        }

        var orderedByAge = newListeners.OrderBy(w1 => w1.Age);
        Console.WriteLine("After sorting by age:");
        foreach (var w in orderedByAge)
        {
            Console.WriteLine($"Listener: {w.Name}, age: {w.Age}, online?: {w.Online}");
        }
    }
}