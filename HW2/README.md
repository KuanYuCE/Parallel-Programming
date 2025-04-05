# Parallel Programming 2025 Spring @ NYCU - HW2

### :pencil2: Q1: Plot a graph of speedup as a function of the number of threads used for VIEW 1. Is the speedup linear to the number of threads used? Hypothesize why this is (or is not) the case.
---


```
$ ./mandelbrot -t 2
[mandelbrot serial]:            [524.892] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [263.953] ms
Wrote image file mandelbrot-thread.ppm
                                (1.99x speedup from 2 threads)
$ ./mandelbrot -t 3
[mandelbrot serial]:            [520.659] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [327.120] ms
Wrote image file mandelbrot-thread.ppm
                                (1.59x speedup from 3 threads)
$ ./mandelbrot -t 4
[mandelbrot serial]:            [530.048] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [221.920] ms
Wrote image file mandelbrot-thread.ppm
                                (2.39x speedup from 4 threads)
$ ./mandelbrot -t 5
[mandelbrot serial]:            [520.166] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [217.842] ms
Wrote image file mandelbrot-thread.ppm
                                (2.39x speedup from 5 threads)
$ ./mandelbrot -t 6
[mandelbrot serial]:            [525.668] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [161.488] ms
Wrote image file mandelbrot-thread.ppm
                                (3.26x speedup from 6 threads)
```

![image](https://hackmd.io/_uploads/ryPbP6sTke.png)


It is obvious from the graph above that the relationship between performance gain and the number of threads is **not linear growth**. As the thread count increases, we observe a decline in efficiency—most noticeably when using three threads. This behavior typically suggests the presence of **false sharing** or **an imbalance in task distribution**, where one or more threads are assigned a disproportionately heavy workload, ultimately extending the overall execution time of the task.




### :pencil2: Q2: How do the measurements explain the speedup graph you previously plotted?
---
We begin by validating the hypothesis using a profiling tool called `perf`. According to the results from perf report, the **cache miss rates** between the two cases are roughly comparable, allowing us to reasonably rule out cache performance as the primary bottleneck.
![image](https://hackmd.io/_uploads/ry53tTsaJg.png)
![image](https://hackmd.io/_uploads/rJHatasayx.png)


Next, we examine whether a similar trend occurs in view2. As shown in the chart below, the decline in efficiency for view2 is less pronounced compared to view1. This suggests that the issue may be more related to task allocation, indicating that optimization efforts should be directed in that area.


```
$ ./mandelbrot -t 2 -v 2
[mandelbrot serial]:            [311.714] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [160.595] ms
Wrote image file mandelbrot-thread.ppm
                                (1.94x speedup from 2 threads)
$ ./mandelbrot -t 3 -v 2
[mandelbrot serial]:            [305.883] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [109.877] ms
Wrote image file mandelbrot-thread.ppm
                                (2.78x speedup from 3 threads)
$ ./mandelbrot -t 4 -v 2
[mandelbrot serial]:            [304.339] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [85.996] ms
Wrote image file mandelbrot-thread.ppm
                                (3.54x speedup from 4 threads)
$ ./mandelbrot -t 5 -v 2
[mandelbrot serial]:            [301.669] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [61.907] ms
Wrote image file mandelbrot-thread.ppm
                                (4.87x speedup from 5 threads)
$ ./mandelbrot -t 6 -v 2
[mandelbrot serial]:            [303.936] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [61.378] ms
Wrote image file mandelbrot-thread.ppm
                                (4.95x speedup from 6 threads)
```
![image](https://hackmd.io/_uploads/BJbXPpjpkl.png)



### :pencil2: Q3: Describe your parallelization approach and report the final speedup achieved with 4 threads.
---
Based on the profiling results, we can rule out false sharing as the primary performance issue. We then turn our attention to identifying the actual bottleneck in the execution of the Mandelbrot program. As shown in the chart below, the `mandel()` function accounts for approximately **97.4%** of the total execution time.

![image](https://hackmd.io/_uploads/HyCq2Ts6Jx.png)

Given the computational nature of the Mandelbrot algorithm given in `mandelSerial.cpp`, `mandel()` exits its inner loop when the sum of the squares of the x and y coordinates exceeds `4.0f`. The output is a grayscale image, where pixel values range **from 0 to 255**. By analyzing the image, we observe that most of the central region is white.

Under the original task distribution strategy (dividing the image into N equal subregions by thread count and assigning each thread a contiguous block), we find that the threads responsible for computing the central region of the image perform up to 256 iterations (constant variable `maxIterations` in `main.cpp`) of floating-point operations per pixel (as per the logic of `mandelbrotSerial()`).

This imbalance indicates that task distribution could be optimized. We first experiment with **cyclic partitioning** as an alternative to the original **block partitioning**. Unlike block partitioning, where each thread computes a continuous chunk of the image (e.g., thread 1 processes the first half of the pixels, thread 2 the second half), cyclic partitioning distributes the workload in a round-robin fashion. For instance, with two threads, thread 1 handles all odd-indexed pixels, while thread 2 processes the even-indexed ones.

This method yields a notable improvement in speedup as shown in below chart. 
```
$ ./mandelbrot -t 2
[mandelbrot serial]:            [523.640] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [271.442] ms
Wrote image file mandelbrot-thread.ppm
                                (1.93x speedup from 2 threads)
$ ./mandelbrot -t 3
[mandelbrot serial]:            [522.083] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [184.269] ms
Wrote image file mandelbrot-thread.ppm
                                (2.83x speedup from 3 threads)
$ ./mandelbrot -t 4
[mandelbrot serial]:            [527.500] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [140.773] ms
Wrote image file mandelbrot-thread.ppm
                                (3.75x speedup from 4 threads)
$ ./mandelbrot -t 5
[mandelbrot serial]:            [522.494] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [115.341] ms
Wrote image file mandelbrot-thread.ppm
                                (4.53x speedup from 5 threads)
$ ./mandelbrot -t 6
[mandelbrot serial]:            [524.420] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [98.303] ms
Wrote image file mandelbrot-thread.ppm
                                (5.33x speedup from 6 threads)
```
![image](https://hackmd.io/_uploads/BkGjECo6Je.png)


However, we observe an increase in cache miss rate.
![image](https://hackmd.io/_uploads/SyE7URjTJe.png)

This prompted a refinement of our approach: instead of pure cyclic partition, we transitioned to a cyclic chunked partitioning scheme where each thread processes scattered, yet more cache-friendly, pixel chunks. This helps reduce false sharing caused by frequent cache line invalidation.

With this adjusted strategy, we observe a marked reduction in cache miss rate, validating the effectiveness of the refined partitioning approach.

![image](https://hackmd.io/_uploads/B19JU0oaJg.png)


### :pencil2: Q4: Is the performance noticeably better than with 6 threads? Why or why not? (Notice that the workstation server provides 6 threads on 6 cores.)
---

```
$ run -c 6 -- ./mandelbrot -t 6
[mandelbrot serial]:            [384.143] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [70.822] ms
Wrote image file mandelbrot-thread.ppm
                                (5.42x speedup from 6 threads)
$ run -c 6 -- ./mandelbrot -t 12
[mandelbrot serial]:            [384.634] ms
Wrote image file mandelbrot-serial.ppm
[mandelbrot thread]:            [72.434] ms
Wrote image file mandelbrot-thread.ppm
                                (5.31x speedup from 12 threads)
```
The execution results shown above indicate that there is no significant performance improvement, in fact, in some cases, the performance remains flat or even degrades. This is mainly due to the overhead introduced when the program attempts to acquire more threads than the hardware can manage. As the number of threads increases, **the processor is forced to perform more frequent context switches**, which in turn diminishes the potential speedup.

These results show an important consideration in parallel computing: optimization efforts must account not only for software programming improvements, but also for the underlying hardware constraints.
