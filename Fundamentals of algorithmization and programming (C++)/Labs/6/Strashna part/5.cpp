#include<iostream>
#include<cmath>
#include<vector>
#include<algorithm>
#include<iomanip>
#include<string>
#include<map>
#include<unordered_map>
#include<queue>

#define ll long long
#define si short int
#define ld long double


using namespace std;

ll n, m, t;



ll binpow(ll a, ll x)
{
    if (x == 0)return 1 % m;
    if (x == 1)return a % m;
    if (x % 2 == 0)
    {
        ll t = binpow(a, x / 2) % m;
        return (t * t) % m;
    }
    if (x % 2 == 1)
    {
        return (a * binpow(a, x - 1)) % m;
    }
}


bool candel(string a, string b, ll kol)
{
    int j = 0;
    for (int i = a.size() - kol; i < a.size(); i++, j++)
        if (a[i] != b[j])
            return false;
    return true;
}



ll get_kol(string a, string b)
{
    if (a.size() + b.size() <= n)
    {
        return binpow(26, n - a.size() - b.size());
    }
    ll bad = a.size() + b.size() - n;
    if (bad <= a.size() && bad <= b.size() && candel(a, b, bad))
    {
        //        cout<<"X";
        return 1;
    }
    return 0;
}
bool check(string s1, string s2)
{
    for (int i = 0; i < s2.size(); i++)
        if (s1[i] != s2[i])
            return false;
    int j = s1.size() - 1;
    for (int i = s2.size() - 1; i >= 0; i--, j--)
        if (s1[j] != s2[i])
            return false;
    return true;
}
main()
{
    string a, b;
    cin >> t;
    while (t--)
    {
        ll ans = 0;
        cin >> n >> m >> a >> b;

        ans = (get_kol(a, b) + get_kol(b, a));
        //        cout<<ans<<endl;
        if (a.size() > b.size())
        {
            if (check(a, b))
            {
                //                cout<<"w";
                ans -= get_kol(a, a);
                ans += m;
            }
        }
        else
        {
            if (check(b, a))
            {
                //                cout<<"q";
                ans -= get_kol(b, b);
                ans += m;
            }
        }
        cout << ans % m << '\n';
    }
}
/**
1
7 1000000
aaa
aa

-*-*-*-**--*-*-*-*
aaa??aa
aa??aaa
*-*-*-*-*-*-*-*-*-
**/
