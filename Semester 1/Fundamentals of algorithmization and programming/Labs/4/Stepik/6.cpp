#include<iostream>
#include<vector>
#include<algorithm>

#define ll long long
using namespace std;

ll n,ind,a,ans;
bool used[10];
vector<ll>v;
std::string s,t;
main()
{
    cin>>s;
    t=s;
    reverse(t.begin(),t.end());
    while(s.size()!=0)
    {
        a=s.back()-'0';
        s.pop_back();
        v.push_back(a);
        used[a]=true;
    }
    for(int i=0;i<10;i++)
    {
        ans+=used[i];
    }
    cout<<ans<<'\n';
    cin>>ind;
    ll kol=0;
    for(int i=0;i<v.size();i++)
        if(v[i]==t[ind-1]-'0')
        cout<<i<<" ";
    return 0;
}
