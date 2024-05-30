using LAB;

internal class Program
{
    static Matrix a, b;
    private static void showMatrix(string matrix_name, Matrix m)
    {
        Console.WriteLine($"=====Matrix {matrix_name}=====");
        Console.WriteLine(m.ToString());
        Console.WriteLine("=============================\n");
    }

    private static void showCmp(string operation, bool verdict)
    {
        if(verdict)
        Console.WriteLine($"{operation} ? Yes");
        else Console.WriteLine($"{operation} ? No");

    }

    private static void getD()
    {
        Console.WriteLine($"Determinate A: {(int)a}");
        Console.WriteLine($"Determinate B: {(int)b}\n");
    }

    private static void Main(string[] args)
    {
         a = new Matrix();
         b = new Matrix();

        a[0, 0] = 1;
        a[0, 1] = 2;
        a[1, 0] = 3;
        a[1, 1] = 4;

        b[0, 0] = 5;
        b[0, 1] = 6;
        b[1,1]= 8;
        b[1, 0] = 7;

        Matrix c;
        showMatrix("A",a);
        showMatrix("B", b);

        showMatrix("A + B",a+b);
        showMatrix("A - B", a - b);
        showMatrix("A x B", a * b);
        showMatrix("A x 6", a * 6);
        showMatrix("A / 6", a / 6);

        showMatrix("A++", a++);
        showMatrix("B++", b++);

        showCmp("A = B", a==b);
        showCmp("A != B", a != b);

        getD();

        showMatrix("Matrix(int)",(Matrix)5);

        showCmp("Is zero A determinant?",a.IsZeroDeterminant());
        showCmp("Is zero B determinant?", a.IsZeroDeterminant());
        showCmp("Is zero C determinant?", a.IsZeroDeterminant());
    }
}