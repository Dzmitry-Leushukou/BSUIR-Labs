using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace _3
{
    internal class DateService
    {
        int day, month, year;
        string date;
        public DateService(int d, int m ,int y)
        {
            day = d; month=m; year=y;   
        }
        public DateService(string s)
        {
            string d=s.Substring(0, 2);
            string m=s.Substring(3,2);
            string y=s.Substring(6);
            //Console.WriteLine(day+" "+month+" "+year);
            day=int.Parse(d);
            month=int.Parse(m);
            year=int.Parse(y);
        }
        public void GetDay()
        {
            DateTime dateTime1= new DateTime(year, month, day);
            Console.WriteLine(dateTime1.DayOfWeek);
        }
        public void GetDaysSpan() 
        {
        DateTime dateTime = DateTime.Now, dateTime1=new DateTime(year, month,day);
        Console.WriteLine(Math.Ceiling(Math.Abs(dateTime.Subtract(dateTime1).TotalDays)) + " Day(s)");
        }
    }
}
