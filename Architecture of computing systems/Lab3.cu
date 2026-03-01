#include <iostream>
#include <fstream>
#include <iomanip>
#include <cuda_runtime.h>
#include <cmath>

__device__ double compute_optimized(double x, int k) {
    double fraction = (x - 1.0) / (x + 1.0);
    double exponent = 2.0 * k + 1.0;
    
    // Более точное вычисление через логарифм и экспоненту
    double log_fraction = log(fabs(fraction));
    double powered = exp(exponent * log_fraction);
    
    return powered / exponent;
}

__global__ void calculate(double x, int k, double* result) {
    *result = compute_optimized(x, k);
}

int main() {
    double x;
    int n;
    std::cout << "Enter x: ";
    std::cin >> x;
    std::cout << "Enter upper bound of calculations (n): ";
    std::cin >> n;

    double* d_result;
    double* h_results = new double[n + 1];
    cudaMalloc(&d_result, sizeof(double));

    std::ofstream fa("arguments.txt");
    std::ofstream ft("times.txt");
    fa << std::setprecision(3) << std::fixed;
    ft << std::setprecision(6) << std::fixed;

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    double ms=0,res=0;
    for (int k = 0; k <= n; ++k) {
        cudaEventRecord(start);
        calculate<<<1, 1>>>(x, k, d_result);
        cudaDeviceSynchronize();
        cudaEventRecord(stop);
        cudaEventSynchronize(stop);

        float milliseconds = 0;
        cudaEventElapsedTime(&milliseconds, start, stop);

        double h_result;
        cudaMemcpy(&h_result, d_result, sizeof(double), cudaMemcpyDeviceToHost);

        res += h_result;
        ms+=milliseconds;
        if(k%1000==0)
        {
        fa << k << '\n';
        ft << std::setprecision(10)<<ms << '\n';
        std::cout << "k: " << k << " f(x) = " << std::setprecision(10) << res << " Time: " << ms << " ms\n";
        }

        
    }

    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(d_result);
    delete[] h_results;

    fa.close();
    ft.close();

    return 0;
}
