using ClassLibrary;
using Lab_6.Entities;
using Lab_6.Interfaces;
using System.IO;
using System.Reflection;

internal class Program
{
    private static void Main(string[] args)
    {
        List<Employee> employees =
        [
            new Employee(100,true, "Ivan"),
            new Employee(150, false, "Vasya"),
            new Employee(130, true, "Dima"),
            new Employee(11111, false, "Ilya"),
            new Employee(20456, false, "Egor"),
            new Employee(32567, true, "Igor"),
        ];

        var assmebled= Assembly.LoadFrom("D:\\Programming\\BSUIR\\BSUIR-Labs\\Semester 3\\C# (Programming tools and means)\\Lab 6\\bin\\Debug\\net8.0\\Lab 6.dll");
       
        var classType=assmebled.GetType("ClassLibrary.FileService`1")
            .MakeGenericType(typeof(Employee));
        
        var fs=Activator.CreateInstance(classType) as IFileService<Employee>;

        fs.SaveData(employees, "Employees.json");
        var Employees = fs.ReadFiles("Employees.json");
        Console.WriteLine("Employees was read:");
        foreach (var e in Employees!)
        {
            Console.WriteLine($"Employee: Name: {e.Name}, Age: {e.Salary}, AtWork: {e.AtWork}");
        }
    }
}