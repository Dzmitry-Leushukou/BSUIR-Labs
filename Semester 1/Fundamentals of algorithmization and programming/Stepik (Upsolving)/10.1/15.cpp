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

ll n=1,n1=1;
ll check(string s)
{
    ll ans=0;
    ll step=1;
    for(int i=s.size()-1;i>=0;i--,step*=3)
    {
        ans+=step*(s[i]-'0');
        if(s[i]=='0')return -1;
    }
    return ans;
}
main()
{
//while(true){
string s;
s="";

//n=n1;
cin>>n;
while(n!=0)
{
    if(n%3==0)
        s+='3',n--;
    else
        s+='0'+n%3;
        n/=3;
}
reverse(s.begin(),s.end());
//if(check(s)!=n1)
//{
//    cout<<n1<<" != "<<s<<"\n";
//}
//n1++;
//}
cout<<s;
}
