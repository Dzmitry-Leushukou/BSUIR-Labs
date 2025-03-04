#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;

unordered_map<long long, long long>stored;

long long f(long long i)
{
    int kol=1;
    while(i!=1)
    {
        if(stored[i]!=0)
            return kol+stored[i]-1;

        if(i%2==0)
            i=i/2;
        else {
            i=(3*i+1)/2;
        }
        kol++;

    }
    return kol;

}
main()
{
    ios_base::sync_with_stdio(0);
    cin.tie(0);

    ofstream fout("simple.dat");
    long long n=1e5;

    for( long long i=1;i<=n;i++)
    {
        stored[i]=f(i);
        fout<<stored[i]<<'\n';
        if(i%50000==0)
        {
            cout<<"+"<<i<<'\n';
        }
    }
    return 0;
}
