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

string s;
bool pal(ll i, ll j)
{
    while(i<j)
    {
        if(s[i]!=s[j])return false;
        i++;
        j--;
    }
    return true;
}
ll kol=0;
main()
{
    cin>>s;
    for(int i=0;i<s.size();i++)
        if(s[i]==s[0])kol++;
    if(kol==s.size())
        return cout<<-1,0;
    ll i=0,j=s.size()-1;
    if(!pal(i,j))
        return cout<<j+1,0;

    cout<<j;
}
