#include<iostream>
#include<algorithm>
#include<string>

#define ll long long

ll n = 1, n1 = 1;
ll check(std::string s)
{
    ll ans = 0;
    ll step = 1;
    for (int i = s.size() - 1; i >= 0; i--, step *= 3)
    {
        ans += step * (s[i] - '0');
        if (s[i] == '0')return -1;
    }
    return ans;
}
int main()
{
    //while(true){
    std::string s;
    s = "";

    //n=n1;
    std::cin >> n;
    while (n != 0)
    {
        if (n % 3 == 0)
            s += '3', n--;
        else
            s += '0' + n % 3;
        n /= 3;
    }
    reverse(s.begin(), s.end());
    //if(check(s)!=n1)
    //{
    //    cout<<n1<<" != "<<s<<"\n";
    //}
    //n1++;
    //}
    std::cout << s;
}