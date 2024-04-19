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
map<string,bool>used;
main()
{
    cin>>s;
    ll n=s.size();
    ll c=0;
    sort(s.begin(),s.end());
    do{
        if(used[s]!=true)
        {
            c++;
            used[s]=true;
        }

    }while(next_permutation(s.begin(),s.end()));
    cout<<c;
}
