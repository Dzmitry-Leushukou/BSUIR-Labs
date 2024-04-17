#pragma[siz] once
#include <iostream>
#include <cstdio>
#define ll long long
void hat(short int variant)
{
    std::cout << "Completed the task : Dmitry Levshukov\n";

    if (variant != -1)
    {
        std::cout << "Variant: " << variant << "\n";
    }

    std::cout << "The task: ";
}

long double cinld()
{
    long double number;


    while (!(std::cin >> number) || (std::cin.peek() != '\n'))
    {
        std::cin.clear();


        while (std::cin.get() != '\n');


        std::cout << "Not correct input. Write again\n";
    }


    return number;
}


long long cinll()
{
    long long number;


    while (!(std::cin >> number) || (std::cin.peek() != '\n'))
    {
        std::cin.clear();


        while (std::cin.get() != '\n');


        std::cout << "Not correct input. Write again\n";
    }


    return number;
}
ll cinllmy()
{
	ll ch;
	ll notzero=0,kolzero=0;
	while (true)
	{
		char a[21];
		for(int i=0;i<20;i++)a[i]=' ';
		bool ban = false;
		ll i=0,c=0;
        do
        {
        if(i==21)break;
        c = getchar();
        a[i] = c;
        if(a[i]>='0'&&a[i]<='9')
          {
            notzero+=a[i]-'0';
            if(a[i]=='0')
                kolzero++;
          }
          else
          {
              if(i!=0||(i==0&&c!='+'&&c!='-'))
            break;
          }

        if(notzero!=0)
        i++;
        } while(c!='\n');
        if(c!='\n')
        {
            while(c!='\n')
            c = getchar();
            std::cout << "Not correct input. Write again\n";
            continue;
        }

        ll ans=atoll(a);
//        std::cout<<(ll)a[0]<<" "<<(ll)a[1]<<"*";
        bool emp=true;
        if(kolzero!=0)
            a[i]='0';
        for(int i=0;i<20;i++)if(a[i]!=' '&&a[i]!='\n')emp=false;
        if(!emp)
        return ans;
        else
        {
            std::cout << "Not correct input. Write again\n";
            continue;
        }
    }
}
int main()
{
//cg    long long x=cinll();
    long long y=cinllmy();
std::    cout<<y;
}
