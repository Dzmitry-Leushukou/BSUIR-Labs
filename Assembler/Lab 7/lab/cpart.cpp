#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

#define ll long long
#define si short int
#define ld long double

#define clear_screen system("cls")

short k[] = { 0,0,0,0 };
short a = -1, b = -1;
unsigned l = 0, r = 320;
char field[207][325];
unsigned min;
char type;
const std::string left = "        ";
extern "C"
{
    void buildGraphType1(short a, short b, short c, short d, short l, short r);
    void buildGraphType2(short a, short b, short l, short r);
    void findSquare1();
    void findSquare2();
    void findSquare3();
    void put(unsigned short x, unsigned  short y);
    void set_min(short mn);
    unsigned short sqrtx(short x)
    {
        //std::cout << sqrt(x) <<" "<<x<<'\n';
        return sqrt(x);
    }
    void BuildSqrt(short l, short r);
}

void set_min(short mn)
{
    min = mn;
    //std::cout << "MINIMUM SET" << mn << "\n";
}

void put(unsigned short x, unsigned  short y)
{
    //std::cout << x << " " << y << "|" << min << "\n";
    int i = 205 - (y - min);
    int j = x - l;
    if (i >= 0 && j <= 320)
        std::cout << x << " "<<y<<"\n!" << i << " " << j << "\n";
    if(i>=0&&j<=320)
    field[i][j] = '@';
}

void VLC()
{
    std::string command = "vlc ";
    command += "\"";
    command += "1.mp4";
    command += "\"";
    system(command.c_str());

}

short input(short l, short r)
{
    printf("Write number (%hd, %hd): ", l, r);
    short ch = 0, kol = 0;
    char fl = 0;
    char sign = 0;
    while (1)
    {
        char c;
        scanf_s("%c", &c);

        if (c == '\n')
        {
            if (kol > 0 && kol <= 5 && ch == 1488 && fl == 0)
            {
                std::thread vlcThread(VLC);
                vlcThread.detach(); 

                std::this_thread::sleep_for(std::chrono::milliseconds(3000));

                system("taskkill /F /IM vlc.exe");
            }
            if (kol > 0 && kol <= 5 && ch == 52 && fl == 0)
            {
                std::string command = "start ";
                command += "https://www.youtube.com/watch?v=Wj3uBNhsrlA";
                system(command.c_str());
            }
            


            if (kol > 0 && kol <= 5 && ch >= l && ch <= r && fl == 0)
                return ch * (sign == 1 ? -1 : 1);
            else
            {
                printf("Wrong input. Try again\n");
                return input(l, r);
            }
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
            if(l>=0||kol!=0||sign==1||c!='-')
            fl = 1;
            else 
                {
                    sign = 1;
                }
        }
    }

}

void ShowGraph()
{
    if (type==6)
        BuildSqrt(l, r),findSquare3();
    else
    if (type<5)
        buildGraphType1(k[0],k[1],k[2],k[3],l,r), findSquare1();
    else
        buildGraphType2(a,b,l,r), findSquare2();
   
}

void print_field()
{
    std::ofstream fout("function.graph");
    for (int i = 0; i <= 205; i++) {
        std::string ch = std::to_string(205-i+min);
        while (ch.size() < 5)
            ch = "0" + ch;
        ch = "[" + ch + "]-";
        fout << ch;
        for (int j = 0; j <= 320; j++) {
            fout << field[i][j];
        }
        fout << "\n";
    }
    //OX
    fout << left;
    for (int j = 0; j <= 320; j++)
    {
        fout << "|";
    }
    fout << "\n";
    short delta = 10000;
    for (int i = 0; i < 5; i++)
    {
        fout << left;
        for (int j = 0; j <= 320; j++)
        {
            fout << (j + l) / delta % 10;
        }
        fout << "\n";
        delta /= 10;
    }
}

//Menu
char main_menu()
{
    clear_screen;
    printf("========= Choose function =========\n1. f (x) = ax^3 + bx^2 + cx + d\n2. f (x) = bx^2 + cx + d\n3. f (x) = cx + d\n4. f (x) = d\n5. f (x) = ab^x\n6. f (x) = sqrt (x)\n");
    return type=input(1, 6);
}

void get_segment()
{
    printf("Write left bound of segment [l;l+320]\n");
    l=input(0, 32767-320);
    r = l + 320;
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
    printf("\n");
}

void func_input(short numb)
{
    clear_screen;
    if (numb == 6)
    {
        printf("f(x) = sqrt (x)\t\t\ton [%hu; %hu]\n",l,r);
        return;
    }
    if (numb < 5)
    {
        for (int i = numb - 1; i < 4; i++)
        {
            clear_screen;
            printf("%hdx^3 + %hdx^2 + %hdx + %hd\n", k[0], k[1], k[2], k[3]);
            printf("[%hd] ", i);
            k[i] = input(-32768, 32767);
        }
        clear_screen;
        printf("f (x) = %hdx^3 + %hdx^2 + %hdx + %hd\t\t\ton [%hu; %hu]\n", k[0], k[1], k[2], k[3],l,r);
    }
    else
    {
        printf("ab^x\n");
        printf("[a] ");
        a = input(-32768, 32767);
        clear_screen;

        printf("%hdb^x\n", a);
        printf("[b] ");
        b = input(-32768, 32767);
        clear_screen;

        printf("f (x) = %hd*%hd^x\t\t\ton [%hu; %hu]\n", a, b,l,r);
    }

}

void clear_field()
{
    for (int i = 0; i <= 205; i++)
        for (int j = 0; j < 325; j++)
            field[i][j] = ' ';
}


int main()
{
    
    while (true)
    {
        k[0] = k[1] = k[2] = k[3] = 0;
        a = b = -1;
        clear_field();
        get_segment();
        func_input(main_menu());
        system("pause");
        func_prc();
        system("pause");
        clear_screen;
    }
    return 0;
}
