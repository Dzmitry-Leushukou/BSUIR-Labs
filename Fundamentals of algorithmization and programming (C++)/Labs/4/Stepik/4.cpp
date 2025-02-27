#include<iostream>

using namespace std;

long long n,m,ans=0;
main()
{
    cin>>n>>m;
    long double a[n+7][m+7];
    for(long long i=0;i<n;i++)
        for(long long j=0;j<m;j++)
            cin>>a[i][j];

    for(long long i=0;i<n;i++)
        for(long long j=0;j<m;j++)
    {
        bool fl=false,fl2=false;
        for(long long q=-1;q<=1;q++)
            for(long long w=-1;w<=1;w++)
        {
        if(q==w&&q==0)
        {
            continue;
        }
        if(i+q>=0&&j+w>=0&&i+q<n&&j+w<m&&a[i][j]>=a[i+q][j+w])
        {
            fl=true;
            break;
        }
        if(i+q>=0&&j+w>=0&&i+q<n&&j+w<m)
        fl2=true;
        }
        if(fl!=true&&fl2==true)
        {
          ans++;
        }
    }
    cout<<ans;
    return 0;
}
