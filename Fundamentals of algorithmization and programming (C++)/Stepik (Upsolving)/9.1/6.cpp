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

string s;
ll used[300];
ll ans=0;
ll fact(ll x)
{
    ll ans=1;
    if(x==1||x==0)return 1;
    for(int i=2;i<=x;i++)
        ans*=i;
    return ans;
}
main()
{
    cin>>s;
    ans=fact(s.size());
    for(int i=0;i<300;i++)
        used[i]=0;
    for(int i=0;i<s.size();i++)
        used[s[i]]++;
    for(int i=0;i<300;i++)
        ans/=(fact(used[i]));
        cout<<ans;
}
