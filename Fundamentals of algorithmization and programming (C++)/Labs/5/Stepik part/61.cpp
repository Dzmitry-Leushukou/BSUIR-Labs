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

ll m,n;
ld a[(ll)1e6],b[(ll)1e6];
main()
{
cin>>m>>n;
for(int i=0;i<m;i++)cin>>a[i];
for(int i=0;i<n;i++)cin>>b[i];
//cout<<a[0]-a[m-2]<<endl<<b[0]-b[n-2]<<endl;
cout<<(a[0]-a[m-1]<b[0]-b[n-1])+1;
}
