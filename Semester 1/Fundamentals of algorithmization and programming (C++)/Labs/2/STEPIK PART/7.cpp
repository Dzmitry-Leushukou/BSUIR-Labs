#include <iostream>

int main()
{
    long long w, h, x1, y1, x2, y2, a, b, ans = (long long)1e18;// a - width, b - height
    std::cin >> w >> h >> x1 >> y1 >> x2 >> y2 >> a >> b;
    long long fsw, fsh, c, d; //Free spaces
    c = x2 - x1;
    d = y2 - y1;
    if (x1 > a || y1 > b)
    {
        std::cout << 0;
        return 0;
    }
    fsw = w - a;
    fsh = h - b;
    //1 case: in (0;0)
    if (d <= fsh)
    {
        ans = std::min(ans, b - y1);

    }
    if (c <= fsw)
    {
        ans = std::min(ans, a - x1);
    }
   

    //2 case: in (W;0)
    if (d <= fsh)
    {
        ans = std::min(ans, b - y1);

    }
    if (c <= fsw)
    {
        ans = std::min(ans, w-x1);
    }
   



    //3 case: in (0;H)
    
    if (d <= fsh)
    {
        ans = std::min(ans, h - y1);

    }
    if (c <= fsw)
    {
        ans = std::min(ans, w - x2);
    }


    //4 case: in (W;H)
    if (d <= fsh)
    {
        ans = std::min(ans, h - y2);

    }
    if (c <= fsw)
    {
        ans = std::min(ans, w - x2);
    }

    if (ans == (long long)1e18)
        std::cout << -1;
    else
        std::cout << ans;
}