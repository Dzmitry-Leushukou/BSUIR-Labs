#include<iostream>
#include<vector>

#define ll long long
using namespace std;

vector<long long>v;
main()
{

    long long a,kol=0;
    while(cin>>a)
    {
        v.push_back(a);
        if(a==0)
            kol++;
    }
    ll mx=-(ll)(1e18);
    ll mn=mx*(-1),xi,ni;
    for(int i=0;i<v.size();i++)
    {
        if(v[i]>mx)
            mx=v[i],xi=i;
        if(v[i]<mn)
            mn=v[i],ni=i;
    }
    for(int i=min(xi,ni)+1;i<max(xi,ni);i++)
    {
        if(v[i]!=0)kol++;
        v[i]=0;
    }
    if(kol>v.size()/2)
    {
        for(int i=0;i<v.size();i++)
        {
            if(v[i]!=0)
                cout<<v[i]<<" ";
        }
    }
    else for(int i=0;i<v.size();i++)
        {
                cout<<v[i]<<" ";
        }
    return 0;
}
