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

ld a,b,x[(ll)1e4],y[(ll)1e4],t;
ll n;
main()
{
    cin>>t;
    while(t--){
    a=b=1.0;
    cin>>n;
    for(int i=0;i<n;i++)
        cin>>x[i];
        for(int i=0;i<n;i++){
        cin>>y[i];
        if(x[i]>y[i])
            a*=x[i]/y[i];
        else b*=y[i]/x[i];
        }
    if(a>b)
        cout<<">";
    if(a<b)
        cout<<"<";
        if(a==b)
        cout<<"=";
    cout<<'\n';
    }
}
