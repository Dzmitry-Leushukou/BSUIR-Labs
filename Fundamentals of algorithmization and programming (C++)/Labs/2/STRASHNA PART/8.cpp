#include <iostream>
#include <cmath>
#include <iomanip>

int main()
{

 
    const long double pi = 3.14159265359;
    bool NoSolutions = true;
    long double a, b, c, p, q, d;
    std::cin >> a >> b >> c >> p >> q;

    //1
    std::cout << "\n=1=\n";
    std::cout << "ax^4 + bx^2 + c = 0\nSolution(s): ";
    if (a == b && b == c && c == 0)
    {
        std::cout << "any number";
    }
    else {
        long double t1 = -1, t2 = -1;
        d = b * b - 4.0 * a * c;
        if (d > 0) {
            t1 = (-b - sqrtl(d)) / (2.0 * a);
            t2 = (-b + sqrtl(d)) / (2.0 * a);
        }
        if (d == 0)
        {
            t1 = -b / (2.0 * a);
        }
        
        if (t1 == t2)
        {
            t2 = -1;
        }
        if (t1 > 0)
        {
            NoSolutions = false;
            std::cout << sqrtl(t1) << " " << -sqrtl(t1) << " ";
        }
        if (t2 > 0)
        {
            NoSolutions = false;
            std::cout << sqrtl(t2) << " " << -sqrtl(t2) << " ";
        }
        if (NoSolutions)
        {
            std::cout << "no solutions :(";
        }
    }
    //2
    std::cout << "\n\n=2=\n";
    std::cout << "ax^4 + bx^3 + cx^2 + bx + a = 0\nSolution(s): ";
    if (a == b && b == c && c == 0)
    {
        std::cout << "any number";
    }
    else {
        NoSolutions = true;
        long double t1 = 0, t2 = 0;
        if (a == 0)
        {
            NoSolutions = false;
            if (b == 0)
            {
                if (c == 0)
                {
                    
                    std::cout << "any number";
                }
                else std::cout << "0";
            }
            else {
                //bx^3+cx^2+bx=0
                //x*(bx^2+cx+b)=0
                d = c * c - 4.0 * b * b;
                if (d > 0) {
                    t1 = (-c - sqrtl(d)) / (2.0 * b);
                    t2 = (-c + sqrtl(d)) / (2.0 * b);
                }
                if (d == 0)
                {
                    t1 = -c / (2.0 * b);
                }

                if (t1 == t2)
                {
                    t2 = -1;
                }
                if (t1 > 0)
                {
                    NoSolutions = false;
                    std::cout << sqrtl(t1) << " " << -sqrtl(t1) << " ";
                }
                if (t2 > 0)
                {
                    NoSolutions = false;
                    std::cout << sqrtl(t2) << " " << -sqrtl(t2) << " ";
                }
                std::cout << "0";
            }
        }
        else {


            d = b * b - 4.0 * a * (c - 2.0 * a);
            //std::cout << "!" << d << "\n";
            if (d > 0) {
                t1 = (-b - sqrtl(d)) / (2.0 * a);
                t2 = (-b + sqrtl(d)) / (2.0 * a);
            }
            if (d == 0)
            {
                t1 = -b / (2.0 * a);
            }

            //t=(x^2+1)/x;
            if (d >= 0)
            {
                d = t1 * t1 - 4.0;
                //std::cout << t1 << " " << d << "\n";

                //Just t because eq = x*x -ax + 1
                if (d > 0)
                {
                    NoSolutions = false;
                    std::cout << (t1 - sqrtl(d)) / 2.0 << " " << (t1 + sqrtl(d)) / 2.0 << " ";
                }
                if (d == 0)
                {
                    NoSolutions = false;
                    std::cout << t1 / 2.0 << " ";
                }

                if (t1 != t2) {
                    d = t2 * t2 - 4.0;
                    if (d > 0)
                    {
                        NoSolutions = false;
                        std::cout << (t2 - sqrtl(d)) / 2.0 << " " << (t2 + sqrtl(d)) / 2.0 << " ";
                    }
                    if (d == 0)
                    {
                        NoSolutions = false;
                        std::cout << t2 / 2.0;
                    }
                }
            }
        }
        if (NoSolutions)
        {
            std::cout << "no solutions :(";
        }
    }
    //3
    //p,q
    std::cout << "\n\n=3=\n";
    std::cout << "x^3 + px + q = 0\nReal solution(s): ";

    NoSolutions = true;
    long double r, s, fi;
    a = 0;
    b = p;
    c = q;

    q = (a*a - 3.0 * b) / 9.0;
    r = (2.0 * a * a * a - 9.0 * a * b + 27.0 * c) / 54.0;
    s = q * q * q - r * r;
    if (s > 0)
    {
        fi = acosl(r / (sqrtl(q * q * q))) / 3.0;
        std::cout << -2.0 * sqrtl(q) * cosl(fi) - a / 3.0 << " ";
        std::cout << -2.0 * sqrtl(q) * cosl(fi + (2.0 / 3.0) * pi) - a / 3.0 << " ";
        std::cout << -2.0 * sqrtl(q) * cosl(fi - (2.0 / 3.0) * pi) - a / 3.0 << " ";
    }
    if (s < 0)
    {
        short int sgn = (r > 0) ? 1 : -1;
        if (q > 0) {
            fi = acoshl(fabsl(r) / (sqrtl(q * q * q)))/3.0;
            std::cout << -2.0 * sgn * sqrtl(q) * coshl(fi) - a / 3.0 << "1 ";
            //std::cout << sgn(r) * sqrtl(q) * coshl(pi) - a / 3 << " ";
            //std::cout << sgn(r) * sqrtl(q) * coshl(pi) - a / 3 << " ";
        }
        if (q < 0)
        {
            fi = asinhl(fabsl(r) / (sqrtl(fabsl(q) * q * q)))/3.0;
            std::cout << -2.0 * sgn * sqrtl(fabsl(q)) * sinhl(fi) - a / 3 << "2 ";
        }
        if (q == 0)
        {
            std::cout << -powl(c - (a * a * a) / 27.0, 1.0 / 3.0) - a / 3.0;
        }
    }
    if (s == 0)
    {
        
        std::cout << ( - 2.0) * 0 << std::endl;
        std::cout << -2.0 * powl(r, 1.0 / 3.0) - a / 3.0 << " ";
        std::cout << powl(r, 1.0 / 3.0) - a / 3.0;
    }
}
/*
0 0 0 9 0.89
*/