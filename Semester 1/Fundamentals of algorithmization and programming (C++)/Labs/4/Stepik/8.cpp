#include<iostream>
#include<vector>

#define ll long long
using namespace std;

ll n,m;

main()
{
    cin>>n;
    ll a[n+1][n+1];
    ll c[n+1][n+1];
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            cin>>a[i][j],c[i][j]=0;
    cin>>m;
    ll b[m+1][m+1];
    for(int i=0;i<m;i++)
        for(int j=0;j<m;j++)
            cin>>b[i][j];

    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            {
            ll q=0;
            ll w=0;
            while(q<n&&w<n)
            {
                c[i][j]+=a[i][q]*b[w][j];
                q++;
                w++;
            }
            }

    for(int i=0;i<m;i++){
        for(int j=0;j<m;j++)
        {
            if(j!=m-1)
           cout<<c[i][j]<<" ";
           else
            cout<<c[i][j];
        }
        cout<<'\n';
    }
        return 0;
}
