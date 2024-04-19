using _3;
internal class Program
{
    private static bool  check(string s)
    {
        if (s.Length != 2 + 2 + 2 + 4) return false;
        int day = 0;
        int month = 0;
        int year = 0;
        string d=s.Substring(0,2);
        string m = s.Substring(3, 2);
        string y = s.Substring(6);
        
        if (s[2] != '.' || s[5] != '.'||!int.TryParse(d, out day) || !int.TryParse(m, out month) || !int.TryParse(y, out year)) return false;
        if (year <= 0 || month > 12 || month < 1 || day > DateTime.DaysInMonth(year,month)||year>9999) return false;

        for(int i=0;i<s.Length;i++)
        {
            if (i == 2 || i == 5) continue;
            if (s[i]>='0' && s[i]<='9')
            {
                continue;
            }
            return false;
        }
        return true;    
    }
    private static void Menu()
    {
        Console.WriteLine("Change type of operation\n1. GetDay(string date)\n2. GetDaysSpan(int day, int month, int year)");
        int numb = 0;
        if (!int.TryParse(Console.ReadLine(), out numb)||(numb!=1&&numb!=2))
        {
            Console.WriteLine("Wrong input");
            Menu();
            return;
        }
        if(numb==1)
        {
            Console.WriteLine("Write date in string by format day.month.year\nFormat DD.MM.YYYY");
            string s=Console.ReadLine();
            if(!check(s))
            {
                Console.WriteLine("Wrong input");
                Menu();
                return;
            }
            DateService date = new DateService(s);
            date.GetDay();
        }
        else
        {
            Console.WriteLine("Write date(every parameter by enter) in format day month year\nFormat DD{Enter}MM{Enter}YYYY");
            int d, m, y;
            if (!(int.TryParse(Console.ReadLine(), out d)) || !(int.TryParse(Console.ReadLine(), out m)) 
                || !(int.TryParse(Console.ReadLine(), out y))
                || y < 1 || y > 10000 || m > 12 || m < 1 || d<0||d>DateTime.DaysInMonth(y, m))
            {
                Console.WriteLine("Wrong input");
                Menu();
                return;
            }
            DateService date=new DateService(d,m,y);
            date.GetDaysSpan();
        }
    }
    private static void Main(string[] args)
    {
        while (true) Menu();
    }
}