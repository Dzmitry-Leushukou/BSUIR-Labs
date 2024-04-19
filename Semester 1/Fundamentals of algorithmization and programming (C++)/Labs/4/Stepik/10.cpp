#include<iostream>
#include<vector>
#include<algorithm>

#define ll long long
using namespace std;

ll n,m;
vector<ll>a,res;
void add(vector<ll>a)
{
//    cout<<"a: ";
//    for(int i=0;i<40;i++)
//        cout<<a[i]<<" ";
//        cout<<endl;
    ll mem=0;
    ll n=res.size();
    for(int i=n-1;i>=0;i--)
    {
        ll nm=(a[i]+res[i])/10;
        res[i]=(res[i]+a[i])%10+mem;
        mem=nm;

    }
}
vector<ll> to_vect(ll x,ll kolz)
{
    vector<ll>b;
    while(x!=0)
    {
        b.push_back(x%10);
        x/=10;
    }

    while(b.size()!=res.size()-kolz)
        b.push_back(0);
    reverse(b.begin(),b.end());
while(kolz--)
        b.push_back(0);
    return b;
}
void sum(ll n,ll x,ll kolz)
{
    if(n==0)
        return;
    add(to_vect(x,kolz));
    sum(n-1,x,kolz);
}


main()
{

    res.resize(40);
    for(int i=0;i<40;i++)
        res[i]=0;
    ll n,m;

    cin>>n;
    if(n==0)return cout<<"1",0;
    if(n==1)
        return cout<<"2",0;
    m=n/2;
    if(n%2==0)
       n--;
    a=to_vect(n,0);
    for(auto& i:a)
    {
//        cout<<i<<" ";
    }
//    cout<<endl<<n<<" "<<m;
//    cout<<endl;
    reverse(a.begin(),a.end());
    for(int i=0;i<a.size();i++)
    {
//        cout<<"Before: ";
//        for(int i=0;i<40;i++)
//        cout<<res[i]<<" ";
//        cout<<endl;
        sum(a[i],m,i);
//        cout<<a[i]<<" "<<m<<" "<<i<<endl<<"After: ";
//        for(int i=0;i<40;i++)
//        cout<<res[i]<<" ";
//        cout<<endl;

    }
    sum(1,1,0);
    ll i=0;
    while(res[i]==0)
        i++;
     for(i;i<40;i++)
        cout<<res[i];
    return 0;
}
