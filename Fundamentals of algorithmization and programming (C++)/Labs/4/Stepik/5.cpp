#include<iostream>

#define ll long long
using namespace std;

ll n,m;
main()
{
    cin>>n>>m;
    ll a[n+5][m+5],b[n+5][m+5];
    for(int i=0;i<n;i++)
        for(int j=0;j<m;j++)
            cin>>a[i][j];
    b[0][0]=a[0][0];
    for(int j=0;j<m;j++)
        for(int i=0;i<n;i++)
    {

        if(i==j&&j==0)
            continue;
                    b[i][j]=a[i][j];
        if(i-1>=0)
            b[i][j]=max(b[i][j],b[i-1][j]);
        if(j-1>=0)
            b[i][j]=max(b[i][j],b[i][j-1]);
    }
    for(int i=0;i<n;i++){
        for(int j=0;j<m;j++)
        cout<<b[i][j]<<" ";
        cout<<'\n';
    }
    return 0;
}
