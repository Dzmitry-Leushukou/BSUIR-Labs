using _1;
internal class Program
{
    public static List<Planet> Planets;
    public static List<Star> Stars;
    private static void Info()
    {
        
        Console.WriteLine("===Planets===");
        foreach (Planet planet in Planets)
        {
            Console.WriteLine(planet.MainInfo());
        }
        Console.WriteLine("\n***Stars***");
        foreach (Star star in Stars)
        {
            Console.WriteLine(star.MainInfo());
        }
        Console.WriteLine("----------------------------------\n\n");
    }
    private static int GetStarID()
    {
        int i = 0;
        foreach (Star Star in Stars)
        {
            Console.WriteLine($"[{i}] {Star.MainInfo()}");
        }

        Console.WriteLine("\nWrite id of planet");
        if (!int.TryParse(Console.ReadLine(), out i) || i < 0 || i >= Stars.Count())
            return GetStarID();
        return i;

    }
    private static void Relative()
    {
        if(Stars.Count() == 0) 
        {
            Console.WriteLine("Add a star first");
            return;
        }
        int i = GetStarID();
        Console.WriteLine(Stars[i].GoToEarth() + "\n");
    }
    private static void Visible()
    {
        if (Stars.Count() == 0)
        {
            Console.WriteLine("Add a star first");
            return;
        }
        int i = GetStarID();
        Console.WriteLine(Stars[i].Visible()+"\n");
    }
    private static int GetPlanetID()
    {
        int i = 0;
        foreach (Planet planet in Planets)
        {
            Console.WriteLine($"[{i}] {planet.MainInfo()}");
        }
        
        Console.WriteLine("\nWrite id of planet");
        if(!int.TryParse(Console.ReadLine(), out i)||i<0||i>=Planets.Count())
            return GetPlanetID();
        return i;

    }
    private static void Move_Planet()
    {
        if (Planets.Count() == 0)
        {
            Console.WriteLine("Add a planet first");
            return;
        }
        Console.WriteLine("Choose id of planet");
        int id = GetPlanetID();
        double time = 0;
        Console.WriteLine("Write how many seconds(>0) should move choosed planet");
        while(!double.TryParse(Console.ReadLine(),out time))
        {
            Console.WriteLine("Wrong input");
        }
        Planets[id].move(time);

    }
    private static Star ReadStar()
    {
        //(string name, double size, double distance, double direction, double speed, double Brightness)
        Console.WriteLine("Write correct data: size>=0, distance(km)>=0,speed(km/s)>=0, direction in range [0;359], Brightness >= 0");
        string name;
        double size, dist, dir, speed, br;
        Console.WriteLine("Write name of the star:");
        name=Console.ReadLine();
        if (name == "")
            return ReadStar();
        Console.WriteLine("Write size of the star:");
       
        if (!double.TryParse(Console.ReadLine(), out size)||size<0)
            return ReadStar();

        Console.WriteLine("Write distance of the star:");

        if (!double.TryParse(Console.ReadLine(), out dist)||dist<0)
            return ReadStar();
        Console.WriteLine("Write direction of the star:");

        if (!double.TryParse(Console.ReadLine(), out dir) || dir < 0||dir>359)
            return ReadStar();
        Console.WriteLine("Write speed of the star:");

        if (!double.TryParse(Console.ReadLine(), out speed) || speed < 0)
            return ReadStar();
        Console.WriteLine("Write brightness of the star:");

        if (!double.TryParse(Console.ReadLine(), out br) || br < 0)
            return ReadStar();
        return new Star(name, size, dist, dir, speed, br);
    }
    private static Planet ReadPlanet()
    {
        //double R, double s, double size, double distance, string name
        Console.WriteLine("Write correct data: size>=0, distance(km)>=0,speed(km/s)>=0, radius >= 0");
        string name;
        double size, dist, R, speed;
        Console.WriteLine("Write name of the planet:");
        name = Console.ReadLine();
        if (name == "")
            return ReadPlanet();
        Console.WriteLine("Write size of the planet:");

        if (!double.TryParse(Console.ReadLine(), out size) || size < 0)
            return ReadPlanet();

        Console.WriteLine("Write distance of the planet:");

        if (!double.TryParse(Console.ReadLine(), out dist) || dist < 0)
            return ReadPlanet();
        Console.WriteLine("Write speed of change of speed along one axis of the planet:");

        if (!double.TryParse(Console.ReadLine(), out speed) || speed < 0)
            return ReadPlanet();
        Console.WriteLine("Write radius of the star:");

        if (!double.TryParse(Console.ReadLine(), out R) || R < 0)
            return ReadPlanet();
        return new Planet(R,speed, size, dist, name);
    }
    private static void Menu()
    {
        Info();
        int p;
        Console.WriteLine("Choose operation:\n1. Add star\n2. Add planet\n3. Move planet\n4. Check direction relative to Earth\n5. Check visibility");
        if (!int.TryParse(Console.ReadLine(), out p) || p > 5 || p < 1)
            Menu();
        switch (p)
        {
            case 1:
                Stars.Add(ReadStar());
                break;
            case 2:
                Planets.Add(ReadPlanet());
                break;
            case 3:
                Move_Planet();
                break;
            case 4:
                Relative();
                break;
            default:
                Visible();
                break;
        }
    }
    private static void Main(string[] args)
    {
        Planets =new List<Planet>();
        Stars=new List<Star>();
        while(true)Menu();
    }
}