#include "test.h"

// void test2(float *__restrict a, float *__restrict b, float *__restrict c, int N)
// {
//   __builtin_assume(N == 1024);
//   a = (float *)__builtin_assume_aligned(a, 16);
//   b = (float *)__builtin_assume_aligned(b, 16);
//   c = (float *)__builtin_assume_aligned(c, 16);

//   for (int i = 0; i < I; i++)
//   {
//     for (int j = 0; j < N; j++)
//     {
//       /* max() */
//       float temp = a[j];
//       // c[j] = a[j];
//       if (b[j] > a[j])
//         temp = b[j];
//       c[j] = temp;
//     }
//   }
// }

void test2(float *__restrict a, float *__restrict b, float *__restrict c, int N)
{
  __builtin_assume(N == 1024);
  a = (float *)__builtin_assume_aligned(a, 16);
  b = (float *)__builtin_assume_aligned(b, 16);
  c = (float *)__builtin_assume_aligned(c, 16);


  for (int i = 0; i < I; i++)
  {
    for (int j = 0; j < N; j++)
    {
      /* max() */
      c[j] = a[j];
      if (b[j] > a[j])
        c[j] = b[j];

      // float temp = a[j];
      // if (b[j] > a[j]){
      //   temp = b[j];
      //   // break;
      // }
      // c[j] = temp;        
    }
  }
}

