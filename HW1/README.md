# Parallel Programming 2025 Spring @ NYCU - HW1

### :pencil2: Q1-1: Does the vector utilization increase, decrease or stay the same as VECTOR_WIDTH changes? Why?
---
VECTOR_WIDTH = 2

![image](https://hackmd.io/_uploads/HyOUy7PhJe.png)

VECTOR_WIDTH = 4

![image](https://hackmd.io/_uploads/H1VVJXwhke.png)

VECTOR_WIDTH = 8

![image](https://hackmd.io/_uploads/ry4fyQw3Jl.png)

VECTOR_WIDTH = 16

![image](https://hackmd.io/_uploads/Skbe1Xwnkl.png)

From the experimental results, we can observe that as `VECTOR_WIDTH` increases, vector utilization `decreases`. This is due to partial computations within the vector being wasted, such as those caused by masked operations (masking).

For example, if `VECTOR_WIDTH` = 4:
```c=
[A0, A1, A2, A3]
```

Assuming `A1 > 0`, but `A0`, `A2`, and `A3` ≤ 0, only `A1` will be actively computed, yet the entire vector will still be processed.

If `VECTOR_WIDTH` = 16:
```c=
[A0, A1, A2, ..., A15]
```

If only `A5`, `A7`, and `A12` > 0, then **only 3 valid computations will be performed, while 13 vector slots are wasted**, leading to lower utilization.

In the example from **Part1**, I think the most significant factor contributing to the decrease in vector utilization is **partial computation**. However, in a more general sense, there are multiple reasons why vector utilization decreases, not just partial computation.

This becomes more clear in **Part2** of the assignment. As vector width increases, it may seem that more data is processed simultaneously. However, **additional overhead must be considered**, such as handling alignment issues or checking for memory overlap. These factors introduce extra **computational costs** that impact overall efficiency.

### :pencil2: Q2-1: Fix the code to make sure it uses aligned moves for the best performance.
---
When enabling AVX2 instructions in `Assignment I Part 2`, we can observe that the generated assembly file, `assembly/test1.vec.restr.align.avx2.s` contains instructions prefixed with `v*`, indicating the use of AVX/AVX2 vectorized operations. However, we can notice the following:
```assembly=
.LBB0_2:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	vmovups	(%rdi,%rcx,4), %ymm0
	vmovups	32(%rdi,%rcx,4), %ymm1
	vmovups	64(%rdi,%rcx,4), %ymm2
	vmovups	96(%rdi,%rcx,4), %ymm3
	vaddps	(%rsi,%rcx,4), %ymm0, %ymm0
	vaddps	32(%rsi,%rcx,4), %ymm1, %ymm1
	vaddps	64(%rsi,%rcx,4), %ymm2, %ymm2
	vaddps	96(%rsi,%rcx,4), %ymm3, %ymm3
	vmovups	%ymm0, (%rdx,%rcx,4)
	vmovups	%ymm1, 32(%rdx,%rcx,4)
	vmovups	%ymm2, 64(%rdx,%rcx,4)
	vmovups	%ymm3, 96(%rdx,%rcx,4)
	addq	$32, %rcx
	cmpq	$1024, %rcx             # imm = 0x400
```

The presence of `vmovups` suggests that the compiler is generating <span class="orange">**unaligned**</span> memory accesses. This is primarily due to the way memory alignment is defined in the source code.

In the previous code section, the alignment intrinsic used is:
```c=
  a = (float *)__builtin_assume_aligned(a, 16);
  b = (float *)__builtin_assume_aligned(b, 16);
  c = (float *)__builtin_assume_aligned(c, 16);
```

This instructs the compiler to assume the pointer `a`, `b` and `c` are aligned to **16 bytes**, which is not sufficient for AVX2 vector operations, as `YMM` registers require **32-byte** alignment for optimal performance.

When modifying the alignment intrinsic to **32 bytes**:
```c=
  a = (float *)__builtin_assume_aligned(a, 32);
  b = (float *)__builtin_assume_aligned(b, 32);
  c = (float *)__builtin_assume_aligned(c, 32);
```
and recompiling, we observe that the generated assembly code changes to:

```assembly=
.LBB0_2:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	vmovaps	(%rdi,%rcx,4), %ymm0
	vmovaps	32(%rdi,%rcx,4), %ymm1
	vmovaps	64(%rdi,%rcx,4), %ymm2
	vmovaps	96(%rdi,%rcx,4), %ymm3
	vaddps	(%rsi,%rcx,4), %ymm0, %ymm0
	vaddps	32(%rsi,%rcx,4), %ymm1, %ymm1
	vaddps	64(%rsi,%rcx,4), %ymm2, %ymm2
	vaddps	96(%rsi,%rcx,4), %ymm3, %ymm3
	vmovaps	%ymm0, (%rdx,%rcx,4)
	vmovaps	%ymm1, 32(%rdx,%rcx,4)

```
The key difference is the transition from `vmovups` to `vmovaps`. The aligned move instruction (`vmovaps`) is more efficient as it assumes memory addresses are properly aligned, avoiding potential performance penalties associated with unaligned memory accesses.

By ensuring correct memory alignment, we allow the compiler to generate more efficient vectorized code, reducing unnecessary penalties from unaligned memory operations.


### :pencil2: Q2-2.1: What speedup does the vectorized code achieve over the unvectorized code? 
---
We executed a program under three different scenarios: 
- without vectorization
    - ![image](https://hackmd.io/_uploads/H1XPYSunJe.png)
    - Median execution time: **6.934** secs
- with vectorization
    - ![image](https://hackmd.io/_uploads/HJXgqBOhkg.png)
    - Median execution time: **1.725** secs
- with vectorization enabled along with the `AVX2` instruction set
    - ![image](https://hackmd.io/_uploads/B1w16Hu2Jx.png)
    - Median execution time: **0.865** secs

From our observations, the performance difference between the vectorized and non-vectorized versions is approximately **4×**. Furthermore, enabling both `AVX2` and vectorization improves performance to about `8×` that of the non-vectorized version, with AVX2 alone contributing roughly a `2×` speedup over standard vectorization.

Note: Each experiment was run `10` times, and we report the median execution time.

### :pencil2: Q2-2.2: What additional speedup does using -mavx2 give (AVX2=1 in the Makefile)? 
---
Enabling `-mavx2` during compilation allows the compiler to generate optimized code leveraging `AVX2` instructions. The potential speedup depends on several factors:

1. Increased SIMD Vector Width (**256-bit**)
    - `AVX2` doubles the width of SIMD registers compared to SSE (128-bit).
    - This means a single instruction processes twice as many elements in parallel (e.g., processing 8 float values instead of 4).
    This is particularly beneficial for applications.
2. Optimized Integer SIMD Operations
    - `AVX2` introduces new 256-bit integer instructions, which improving the performance. For examples:
        - vmovaps (Vector Move Aligned Packed Single-Precision)
            - `vmovaps` moves aligned **256-bit** packed values between registers or between memory and registers.
It requires the memory operand to be aligned a 32-byte (for YMM registers) boundary.
        - vaddps (Vector Add Packed Single-Precision)
            - `vaddps` performs element-wise addition on packed numbers in YMM registers.


### :pencil2: Q2-2.3: What can you infer about the bit width of the default vector registers on the PP machines? What about the bit width of the AVX2 vector registers?
---

vectorization without `AVX2`

![image](https://hackmd.io/_uploads/BJU3WI_2yg.png)

vectorization with `AVX2` enabled

![image](https://hackmd.io/_uploads/H1JRkUdnyg.png)

By looking at the assembly code, we can observe that `AVX2` utilizes `YMM` registers, which are **256-bit wide** registers. In contrast, the program that does not use the `AVX2` instruction set operates with `XMM` registers, which are the standard **128-bit wide** registers used by the `SSE` instruction set and our PP machine.

Reference: [Intel® 64 and IA-32 Architectures Software Developer's Manual](https://www.intel.com/content/www/us/en/content-details/843836/intel-64-and-ia-32-architectures-software-developer-s-manual-combined-volumes-3a-3b-3c-and-3d-system-programming-guide.html)
![image](https://hackmd.io/_uploads/rJmgNId3Jx.png)

![image](https://hackmd.io/_uploads/BJlEV8Oh1x.png)


### :pencil2: Q2-3: Provide a theory for why the compiler is generating dramatically different assemblies.
---

I couldn't come up with a clear explanation for this until I looked through [A Guide to Vectorization with Intel® C++ Compilers](https://www.intel.com/content/dam/develop/external/us/en/documents/31848-compilerautovectorizationguide.pdf) documentation.

There was a section that explained this very clearly and resolved my question. The key takeaway is that vectorization actually **alters the instruction execution order**. Therefore, vectorization can only be performed if it is ensured that these changes do not affect the correctness of the results.

![image](https://hackmd.io/_uploads/B1BmuIuh1g.png)


So the main reason is that the code might attempt to **access the same memory location twice within the `for loop`**, preventing the compiler from vectorizing the loop.


Aside from the approach provided in the assignment, another method that allows the compiler to generate **vectorized assembly** using `movaps` and `maxps` instructions could be like:

```c=
float temp = a[j];
if (b[j] > a[j])
    temp = b[j];
c[j] = temp;
```

the assembly of the above `C` code:
```assembly=
.LBB0_2:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movaps	(%rsi,%rcx,4), %xmm0
	movaps	16(%rsi,%rcx,4), %xmm1
	maxps	(%rdi,%rcx,4), %xmm0
	maxps	16(%rdi,%rcx,4), %xmm1
	movaps	%xmm0, (%rdx,%rcx,4)
	movaps	%xmm1, 16(%rdx,%rcx,4)
	movaps	32(%rsi,%rcx,4), %xmm0
	movaps	48(%rsi,%rcx,4), %xmm1
	maxps	32(%rdi,%rcx,4), %xmm0
	maxps	48(%rdi,%rcx,4), %xmm1
	movaps	%xmm0, 32(%rdx,%rcx,4)
	movaps	%xmm1, 48(%rdx,%rcx,4)
	addq	$16, %rcx
	cmpq	$1024, %rcx             # imm = 0x400
	jne	.LBB0_2
```

This method involves using a temporary variable to store the computation result. Since all computations involving the temporary variable **occur within registers**, the compiler no longer perceives multiple accesses to the same memory location. As a result, vectorization becomes possible.

