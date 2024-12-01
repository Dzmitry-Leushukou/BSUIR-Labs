#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <fstream>

#define ll long long
#define si short int
#define ld long double

#define clear_screen system("cls")

short k[] = { 0,0,0,0 };
short a = -1, b = -1,l=0,r=320;
char field[205][325];

extern "C"
{
    void buildGraphType1(short a, short b, short c, short d, short l, short r);
    void buildGraphType2(short a, short b, short l, short r);
    void findSquare();
    void put(unsigned short x, unsigned short y);
}


void put(unsigned short x, unsigned short y)
{
    //std::cout<<"\n*" << x << " " << y << "*\n/" << 205 - x << " " << y << "/\n";
    if(205-y>=0&&x<=320)
    field[205-y][x] = '@';
}

short input(short l, short r)
{
    printf("Write number (%hd, %hd): ", l, r);
    short ch = 0, kol = 0;
    char fl = 0;
    while (1)
    {
        char c;
        scanf_s("%c", &c);

        if (c == '\n')
            if (kol > 0 && kol <= 5 && ch >= l && ch <= r && fl == 0)
                return ch;
            else
            {
                printf("Wrong input. Try again\n");
                return input(l, r);
            }
        if (c >= '0' && c <= '9')
        {
            if (ch >= 1000 && ch / 1000 > 3)
                fl = 1;
            ch = ch * 10 + (c - '0'), kol++;
            if (ch < 0)
                fl = 1;
        }
        else
        {
            fl = 1;
        }
    }

}

void ShowGraph()
{
    if (a == -1)
        buildGraphType1(k[0],k[1],k[2],k[3],l,r);
    else
        buildGraphType2(a,b,l,r);
}

void print_field()
{
    std::ofstream fout("function.graph");
    for (int i = 0; i <= 205; i++) {
        for (int j = 0; j <= 320; j++) {
            fout << field[i][j];
        }
        fout << "\n";
    }
}
//Menu
char main_menu()
{
    printf("========= Choose function =========\n1. f (x) = ax^3 + bx^2 + cx + d\n2. f (x) = bx^2 + cx + d\n3. f (x) = cx + d\n4. f (x) = d\n5. f (x) = ab^x\n");
    return input(1, 5);
}

void func_prc()
{
    printf("Graphic view of funciton:\n");
    ShowGraph();
    print_field();
    //Call notepad.exe to open file with graph
    const char* filePath = "D:\\Programming\\BSUIR\\BSUIR-Labs\\Semester 3\\Assembler\\Lab 7\\lab\\function.graph";
    HINSTANCE result = ShellExecuteA(NULL, "open", "notepad.exe", filePath, NULL, SW_SHOWMAXIMIZED);
    //printf("Square: ");
    findSquare();
    printf("\n");
}

void func_input(short numb)
{
    clear_screen;
    if (numb < 5)
    {
        for (int i = numb - 1; i < 4; i++)
        {
            clear_screen;
            printf("%hdx^3 + %hdx^2 + %hdx + %hd\n", k[0], k[1], k[2], k[3]);
            printf("[%hd] ", i);
            k[i] = input(0, 32767);
        }
        clear_screen;
        printf("f (x) = %hdx^3 + %hdx^2 + %hdx + %hd\n", k[0], k[1], k[2], k[3]);
    }
    else
    {
        printf("ab^x\n");
        printf("[a] ");
        a = input(0, 32767);
        clear_screen;

        printf("%hdb^x\n", a);
        printf("[b] ");
        b = input(0, 32767);
        clear_screen;

        printf("f (x) = %hd*%hd^x\n", a, b);
    }

}

void clear_field()
{
    for (int i = 0; i < 205; i++)
        for (int j = 0; j < 325; j++)
            field[i][j] = ' ';
}



int main()
{
    
    
    while (1)
    {
        k[0] = k[1] = k[2] = k[3] = 0;
        a = b = -1;
        clear_field();
        func_input(main_menu());
        func_prc();
        system("pause");
        clear_screen;
    }
    return 0;
}
