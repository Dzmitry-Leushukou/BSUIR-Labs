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

ll f(ll x)
{
    if(x%10>0)
        return x%10;
    if(x==0)
        return 0;
    return f(x/10);
}
main()
{
ll p,q;
while(cin>>p>>q)
{
    if(p<0&&q<0)return 0;
    ll ans=0;
    for(;p<=q;p++)
    {
        ans+=f(p);
    }
    cout<<ans<<'\n';
}
}
