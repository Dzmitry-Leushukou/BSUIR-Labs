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

    const __int128 m=(ll)1e10;
    bool fl=false;
    __int128 binpow(__int128 a,__int128 s)
    {
        if(s==0)
            return 1;


        if(s==1)
            return a;


        if(s%2==0)
        {
            __int128 q=binpow(a,s/2);

                q%=m;
            return (q*q)%m;
        }


        __int128 q=binpow(a,s-1);


        q=q%m;
        return (a*q)%m;
    }

    __int128 step(__int128 a, ll s)
    {
        __int128 res=1;
        while(s!=0&&res<=m)
            res*=a,s--;
        if(s==0)
            return res%m;
        fl=true;
        return (res*binpow(a,s))%m;
    }

    string viv(ll n)
    {
        string s;
        s="";
        while(n!=0)
                s+='0'+n%10,n/=10;
        if(fl)
        {
            while(s.size()<10)
                s+="0";
        }
         reverse(s.begin(),s.end());
            return s;
    }
    main()
    {
        ll t,q,w;
        cin>>t;
        while(t--)
        {
            cin>>q>>w;
            ll res=(ll)step((__int128)q,(__int128)w);
//            cout<<res<<endl;
            cout<<viv(res)<<'\n';
        }
    }
