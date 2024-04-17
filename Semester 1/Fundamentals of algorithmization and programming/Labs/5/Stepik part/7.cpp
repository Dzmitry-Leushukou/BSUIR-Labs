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
ll k,n,t,m;
ll binpow(ll o, ll step)
{
    if(step==1)return o%m;
    if(step==0)return 1LL;
    if(step%2!=0)return ((o%m)*(binpow(o,step-1)%m))%m;
    if(step%2==0)
    {
        ll a=binpow(o,step/2)%m;
        return (a*a)%m;
    }
}
main()
{
ll i=1;
while(cin>>k>>n>>t)
{
    if(k==0&&t==0&&n==0)return 0;

    m=1;
    while(t--)
        m*=10;
    cout<<"Case #"<<i<<": "<<binpow(k,n)<<'\n';
    i++;
}
}
