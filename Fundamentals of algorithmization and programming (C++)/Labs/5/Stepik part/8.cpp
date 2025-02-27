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
    if(x%2==1)return x;
    if(((x/2)%2)==1)return x/2LL;
    ll mx=1;
    for(ll i=1;i*i<=x;i++)
    {
        if(x%i==0)
        {
          if((x/i)%2!=0)
                mx=max(mx,x/i);
          if(i%2!=0)
                mx=max(i,mx);
        }
    }
    return mx;
}
ll f1(ll x)
{
  if (x==0) return 0;
  ll k=(x+1)/2;
  return k*k+f1(x/2);
}
main()
{
    ll n;
//    ios_base::sync_with_stdio(0);
//    cin.tie(0);

    cin>>n;

    for(ll i=0;i<n;i++)
    {
        ll x;
        cin>>x;
        cout<<f1(x)<<'\n';
        //x=i+1;
//        ll ans=0;
//        for(ll j=1;j<=x;j++)
//            ans+=f(j);
//        cout<<ans<<'\n';
    }
}
