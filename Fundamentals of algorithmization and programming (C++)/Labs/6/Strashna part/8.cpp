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
#define all(x) x.begin(), x.end()

using namespace std;

ll n;
main()
{
    cin >> n;
    while (n--)
    {
        string a, b;
        cin >> a >> b;
        bool fl = false;
        for (int i = 0; i < a.size(); i++)
            for (int j = 0; j < a.size() - i; j++)
            {
                if (fl)break;
                ll ch = b.size() - 1 - j;
                if (ch > i + j)
                    continue;
                ll l1 = i, r = i + j;
                ll l2 = r - ch;
                ll r1 = r - l1 + 1;
                ll r2 = r - l2;
                string s = "", u = "";
                for (l1; l1 < min(r + 1, (ll)a.size()); l1++)
                    s += a[l1];
                for (l2; l2 < min(r, (ll)a.size()); l2++)
                    u += a[l2];
                //        string u=a.substr(l2, r2);
                reverse(all(u));
                s += u;

                //        cout<<"="<<s<<endl;
                if (s == b)
                {
                    fl = true;
                }
            }
        if (fl)
            cout << "YES";
        else
            cout << "NO";
        cout << '\n';
    }
}
