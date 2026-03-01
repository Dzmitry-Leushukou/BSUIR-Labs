#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <iostream>

double func(double x, int k) {
    double result;
    
    __asm__(
        "fildl %2\n"      // Загрузить k в st(0)
        "fldl %1\n"       // Загрузить x в st(0), k в st(1)
        "fadd %%st(0)\n"  // 2x = x + x, в st(0), k в st(1)
        "fmulp %%st(1)\n" // 2kx = 2x * k, в st(0)
        "fcos\n"          // cos(2kx) в st(0)
        "fildl %2\n"      // Загрузить k в st(0), cos(2kx) в st(1)
        "fmul %%st(0)\n"  // k² в st(0)
        "fadd %%st(0)\n"  // 2k² в st(0)
        "fadd %%st(0)\n"  // 4k² в st(0)
        "fld1\n"          // 1 в st(0), 4k² в st(1)
        "fsubp %%st(1)\n" // 4k²-1 в st(0)
        "fdivrp %%st(1)\n"// cos(2kx)/(4k²-1) в st(0)
        "fchs\n"          // Смена знака
        "fstpl %0\n"      // Результат
        : "=m"(result)
        : "m"(x), "m"(k)
        : "st", "st(1)"
    );
    
    return result;
}

int main() {

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set); // Привязка к ядру 0
    sched_setaffinity(0, sizeof(set), &set);


    double x;
    int n;
    std::cout<<"Write x: ";
    std::cin>>x;
    std::cout<<"Write upper bound of calculations: ";
    std::cin>>n;
    double res;
    double all_time=0;
    for(int i=0;i<=n;i++)
    {
        clock_t start=clock();
        double tmp=func(x,i);
        res+=tmp;
        clock_t exec_time=clock();
        double time = ((double)exec_time-(double)start)/CLOCKS_PER_SEC*(double)1000.0; //ms
        all_time += time;
        std::cout<<"x: "<<x<<" k: "<<i<<" f(x) = "<<tmp<<" "<<"time exec= "<<all_time<<" ms"<<'\n';
    }


    std::cout<<"All time to exec operations: "<<all_time<<" ms";
    return 0;
}
