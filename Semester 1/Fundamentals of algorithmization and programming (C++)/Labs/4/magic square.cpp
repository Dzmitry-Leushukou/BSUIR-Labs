#include <iostream>
#include <stdlib.h>
#define ll long long

using namespace std;


void v(ll** &a,ll n)
{
    for (int q = 0; q < n; q++)
    {
        for (int w = 0; w < n; w++)
        {

            std::cout << a[q][w] << " ";
        }
        std::cout << '\n';
    }
}

void nch(ll** &a,ll n, ll x, ll mx)
{
    ll i=0,j=n/2;
    while(x<=mx)
    {

        a[i][j]=x;
        x++;
        ll si=i,sj=j;
        j++;
        i--;
        ///up right
        if(i<0&&j<n)
        {
            i=n-1;
        }
        if(i>=0&&j>=n)
        {
            j=0;
        }
        if(i<0&&j>=n)
        {
            i+=2;
            j--;
        }
        if(a[i][j]!=0)
        {
            i=si;
            j=sj;
            i++;
        }


    }

}


void sq(ll n)
{
  ll** a = nullptr;
    a = (ll**)malloc(n * sizeof(ll*));


    for (ll i = 0; i < n; ++i)
    {
        a[i] = (ll*)malloc(n * sizeof(ll));

        for (ll j = 0; j < n; j++)
            a[i][j] = n*i+j+1;
    }

    for(ll i=0;i<n/4;i++)
    {
        for(int j=0;j<n/4;j++)
        {
            a[i][j]=n*n-(n*i+j);
            a[i+n/4*3][j]=n*n-((i+n/4*3)*n+j);
            a[i][j+n/4*3]=n*n-(n*i+j+n/4*3);
            a[i+n/4*3][j+n/4*3]=n*n-((i+n/4*3)*n+j+n/4*3);
        }

    }
    for(int i=n/4;i<n/4*3;i++)
        for(int j=n/4;j<n/4*3;j++)
        a[i][j]=n*n-(n*i+j);
       for (int q = 0; q < n; q++)
    {
        for (int w = 0; w < n; w++)
        {

            std::cout << a[q][w] << " ";
        }
        std::cout << '\n';
    }

}


void ch(ll n)
{
ll** a;
ll** b;
ll** c;
ll** d;
 a = (ll**)malloc(n * sizeof(ll*));
    for (ll i = 0; i < n; ++i)
    {
        a[i] = (ll*)malloc(n * sizeof(ll));
        for(int j=0;j<n;j++)
            a[i][j]=0;
    }
 b = (ll**)malloc(n * sizeof(ll*));
    for (ll i = 0; i < n; ++i)
    {
        b[i] = (ll*)malloc(n * sizeof(ll));
        for(int j=0;j<n;j++)
            b[i][j]=0;
    }
 c = (ll**)malloc(n * sizeof(ll*));
    for (ll i = 0; i < n; ++i)
    {
        c[i] = (ll*)malloc(n * sizeof(ll));
        for(int j=0;j<n;j++)
            c[i][j]=0;
    }
 d = (ll**)malloc(n * sizeof(ll*));
    for (ll i = 0; i < n; ++i)
    {
        d[i] = (ll*)malloc(n * sizeof(ll));
        for(int j=0;j<n;j++)
            d[i][j]=0;
    }
ll s=n*n/4;
nch(a,n/2,1,s);
nch(b,n/2,2*s+1,s*3);
nch(c,n/2,3*s+1,s*4);
nch(d,n/2,s+1,s*2);

for (int q = 0; q < n/2; q++)
for (int w = 0; w < n/4; w++)
{
    if(w<n/4-1)
    swap(b[q][n/2-w-1],d[q][n/2-w-1]);
    if(q==n/4&&w==0)
    {
        swap(a[q][n/4],c[q][n/4]);
        continue;
    }
    swap(a[q][w],c[q][w]);
}
 for (int q = 0; q < n/2; q++)
    {
        for (int w = 0; w < n/2; w++)
        {

            std::cout << a[q][w] << " ";
        }
        for (int w = 0; w < n/2; w++)
        {

            std::cout << b[q][w] << " ";
        }
        std::cout << '\n';
    }
   for (int q = 0; q < n/2; q++)
    {
        for (int w = 0; w < n/2; w++)
        {

            std::cout << c[q][w] << " ";
        }
        for (int w = 0; w < n/2; w++)
        {

            std::cout << d[q][w] << " ";
        }
        std::cout << '\n';
    }
}


void ms(ll n)
{
    if(n%2==1)
    {
        ll** a = (ll**)malloc(n * sizeof(ll*));
    for (ll i = 0; i < n; ++i)
    {
        a[i] = (ll*)malloc(n * sizeof(ll));
        for(int j=0;j<n;j++)
            a[i][j]=0;
    }
        nch(a,n,1,n*n);
        return v(a,n);
    }
    if(n%4==0)
        return sq(n);
    return ch(n);
}


int main()
{
    ll n;
    cin>>n;
    if(n==1)
    {
        return cout<<1,0;
    }
    if (n == 2)
    {
        cout << "NO";
        return 0;
    }
    ms(n);
}
