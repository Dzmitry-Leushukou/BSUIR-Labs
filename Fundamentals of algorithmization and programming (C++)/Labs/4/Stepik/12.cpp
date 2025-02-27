#include<iostream>

#define ll long long
using namespace std;

long long n,i,j;
main()
{
    cin>>n;
    ll x=1;
    while(x*x<n)
        x++;
    ll f=x*x;
    if(f-x>=n)
    {
        f=f-(x-1);
        cout<<x-(f-n+1)<<" "<<x-1;
    } else
    cout<<x-1<<" "<<f-n;
    return 0;
}
