#include <iostream>
#include <cmath>
#include <iomanip>


int main()
{
    long double pi = 3.14159265359;
    long double x1, y1, x2, y2, x3, y3;
    std::cin >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;
    std::cout << std::setprecision(10);

    long double a = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    long double b = sqrt((x1 - x3) * (x1 - x3) + (y1 - y3) * (y1 - y3));
    long double c = sqrt((x3 - x2) * (x3 - x2) + (y3 - y2) * (y3 - y2));
    long double p = (a + b + c) / 2.0;

    ///First option for finding S:
    long double s = sqrt(p * (p - a) * (p - b) * (p - c));

    ///Find the height lenght of the triangle and use the second S formula: S = h * a /2;
    long double ha = s / (a / 2.0);
    long double hb = s / (b / 2.0);
    long double hc = s / (c / 2.0);
    
    ///For the next steps, it will be convenient to find the angles
    long double da, db, dc, ra, rb, rc, cosa, cosb, cosc;
    cosa = (b * b + c * c - a * a) / (2.0 * b * c);
    cosb = (a * a + c * c - b * b) / (2.0 * a * c);
    cosc = (a * a + b * b - c * c) / (2.0 * a * b);
    ra = acos(cosa);
    rb = acos(cosb);
    rc = acos(cosc);

    ///Convert from radians to degrees
    da = ra * (180.0 / pi);
    db = rb * (180.0 / pi);
    dc = rc * (180.0 / pi);

    ///Find the medians lenght
    long double ma, mb, mc;
    ma = 0.5 * sqrt(2.0 * b * b + 2.0 * c * c - a * a);
    mb = 0.5 * sqrt(2.0 * a * a + 2.0 * c * c - b * b);
    mc = 0.5 * sqrt(2.0 * b * b + 2.0 * a * a - c * c);

    ///Find the bisector lenght
    long double ba, bb, bc;
    ba = sqrt(2 * b * c * cos(ra / 2.0) / (c + b));
    bb = sqrt(2 * a * c * cos(rc / 2.0) / (a + c));
    bc = sqrt(2 * a * b * cos(rc / 2.0) / (a + b));

    ///Find R and r and use 2 formulas of S
    long double R = a * b * c / (4.0 * s);
    long double r = s / p;

    ///Fins S and length of circle with radius: R and r
    long double sR, sr, lR, lr;
    sR = pi * R * R;
    sr = pi * r * r;
    lR = 2.0 * pi * R;
    lr = 2.0 * pi * r;
    
    ///Find S and P of triangle
    long double P = p * 2.0, s1, s2;
    s1 = ha * a / 2.0;
    s2 = p * r;


    std::cout << "Sides of a triangle:\n";//1
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';
    std::cout << "c = " << c << '\n';
    std::cout << '\n';

    std::cout << "Hieght lenghts of a triangle:\n"; ///2
    std::cout << "Height length to side A: " << ha << '\n';
    std::cout << "Height length to side B: " << hb << '\n';
    std::cout << "Height length to side C: " << hc << '\n';
    std::cout << '\n';

    std::cout << "Medians lenghts of a triangle:\n"; ///3
    std::cout << "Мedian length to side A: " << ma << '\n';
    std::cout << "Median length to side B: " << mb << '\n';
    std::cout << "Median length to side C: " << mc << '\n';
    std::cout << '\n';

    std::cout << "Bisectors lenghts of a triangle:\n"; ///4
    std::cout << "Bisector length to side A: " << ba << '\n';
    std::cout << "Bisector length to side B: " << bb << '\n';
    std::cout << "Bisector length to side C: " << bc << '\n';
    std::cout << '\n';

    std::cout << "Angles of a triangle:\n"; ///5
    std::cout << "Angle opposite side A: " << ra << " radians | " << da << " degrees\n";
    std::cout << "Angle opposite side b: " << rb << " radians | " << db << " degrees\n";
    std::cout << "Angle opposite side C: " << rc << " radians | " << dc << " degrees\n";
    std::cout << '\n';

    std::cout << "Radii:\n"; ///6
    std::cout << "Radius of the inscribed circle: " << r << '\n';
    std::cout << "Radius of the circumscribed circle bed circle: " << R << '\n';
    std::cout << '\n';

    std::cout << "S and length of circles with radius 1 and 2:\n"; ///7
    std::cout << "[Inscribed circle] S = " << sr << " Leght = " << lr << "\n";
    std::cout << "[Circumscribed circle bed circle] S = " << sR << " Leght = " << lR << "\n";
    std::cout << '\n';

    std::cout << "S and P of triangle:\n"; ///8
    std::cout << "S (Heron's formula) = " << s << '\n';
    std::cout << "S (S = a * ha / 2) = " << s1 << '\n';
    std::cout << "S (S = p * r) = " << s2 << '\n';
    std::cout<< " P = " << P << "\n";
    
}