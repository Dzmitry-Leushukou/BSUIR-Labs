#include <iostream>
#include <cstdint>

int8_t A[8] = { 1, -2, 3, -4, 5, -6, 7, -8 };
int8_t B[8] = { -1, 2, -3, 4, -5, 6, -7, 8 };
int8_t C[8] = { 9, -8, 7, -6, 5, -4, 3, -2 };
int16_t D[8] = { 2, 4, 6, 8, 10, 12, 14, 16 };
int16_t F[8] = { 0 };

int main() {
    asm volatile (
        "movq %[A], %%mm0\n\t"        
        "movq %[B], %%mm1\n\t"        
        
        "pxor %%mm7, %%mm7\n\t"       
        "pcmpgtb %%mm0, %%mm7\n\t"    // Sign mask for A
        "movq %%mm0, %%mm2\n\t"       // A to mm2
        "punpcklbw %%mm7, %%mm2\n\t"  
        
        "pxor %%mm6, %%mm6\n\t"       
        "pcmpgtb %%mm1, %%mm6\n\t"    // Sign mask for B
        "movq %%mm1, %%mm3\n\t"       // B to mm3
        "punpcklbw %%mm6, %%mm3\n\t"  
        
        "pmullw %%mm3, %%mm2\n\t"     // A[0-3]*B[0-3]
        
        "punpckhbw %%mm7, %%mm0\n\t"  
        "punpckhbw %%mm6, %%mm1\n\t"  
        "pmullw %%mm1, %%mm0\n\t"     // A[4-7]*B[4-7]
        
        "movq %[C], %%mm4\n\t"        // C to mm4
        
        "pxor %%mm7, %%mm7\n\t"       
        "pcmpgtb %%mm4, %%mm7\n\t"    // Sign mask for C
        "movq %%mm4, %%mm5\n\t"       // C to mm5
        "punpcklbw %%mm7, %%mm5\n\t"  
        "punpckhbw %%mm7, %%mm4\n\t"  
        
        "movq %[D], %%mm6\n\t"        // D[0-3] to mm6
        "movq %[D8], %%mm7\n\t"       // D[4-7] to mm7
        
        "psubw %%mm6, %%mm5\n\t"      // C[0-3] - D[0-3]
        "psubw %%mm7, %%mm4\n\t"      // C[4-7] - D[4-7]
        
        "paddw %%mm5, %%mm2\n\t"      
        "paddw %%mm4, %%mm0\n\t"      
        
        "movq %%mm2, %[F]\n\t"        // mm2 to F[0-3]
        "movq %%mm0, %[F8]\n\t"       // mm0 F[4-7]
        
        "emms\n\t"                   
        
        : [F] "=m" (F), [F8] "=m" (F[4])
        : [A] "m" (A), [B] "m" (B), [C] "m" (C), [D] "m" (D), [D8] "m" (D[4])
        : "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7", "memory"
    );

    // Verification code
    for (int i = 0; i < 8; i++) {
        int16_t expected = (A[i] * B[i]) + (C[i] - D[i]);
        std::cout << "F[" << i << "] = " << F[i] 
                  << " (expected: " << expected << ")" 
                  << (F[i] == expected ? " ✓" : " ✗") << "\n";
    }
    
    return 0;
}