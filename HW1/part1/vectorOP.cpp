#include "PPintrin.h"

// implementation of absSerial(), but it is vectorized using PP intrinsics
void absVector(float *values, float *output, int N)
{
  __pp_vec_float x;
  __pp_vec_float result;
  __pp_vec_float zero = _pp_vset_float(0.f);
  __pp_mask maskAll, maskIsNegative, maskIsNotNegative;

  //  Note: Take a careful look at this loop indexing.  This example
  //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
  //  Why is that the case?
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {

    // All ones
    maskAll = _pp_init_ones();

    // All zeros
    maskIsNegative = _pp_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _pp_vload_float(x, values + i, maskAll); // x = values[i];

    // Set mask according to predicate
    _pp_vlt_float(maskIsNegative, x, zero, maskAll); // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _pp_vsub_float(result, zero, x, maskIsNegative); //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _pp_mask_not(maskIsNegative); // } else {

    // Execute instruction ("else" clause)
    _pp_vload_float(result, values + i, maskIsNotNegative); //   output[i] = x; }

    // Write results back to memory
    _pp_vstore_float(output + i, result, maskAll);
  }
}

void clampedExpVector(float *values, int *exponents, float *output, int N)
{
  //
  // PP STUDENTS TODO: Implement your vectorized version of
  // clampedExpSerial() here.
  //
  // Your solution should work for any value of
  // N and VECTOR_WIDTH, not just when VECTOR_WIDTH divides N
  //
  __pp_vec_float valueVector;
  __pp_vec_int exponentVector;
  __pp_vec_int count;
  __pp_vec_float result;
  __pp_vec_int zero = _pp_vset_int(0);
  __pp_vec_int one = _pp_vset_int(1);
  __pp_vec_float ceil = _pp_vset_float(9.999999f);

  __pp_mask paddedMask, maskIfZero, maskIfNotZero, maskGreaterThanLimit, maskGreaterThanZero;

  int padding_count = N % VECTOR_WIDTH;
  int total_iterations = 0;
  int full_iterations = N / VECTOR_WIDTH;
  
  // TODO: 改成使用memset去初始化
  float *vectorOnes = new float[N];
  float *upperBoundVec = new float[N];
  for (int i = 0; i < N; ++i) {
    vectorOnes[i] = 1.f;
    upperBoundVec[i] = 9.999999f;
  }

  for (int i = 0; i < N; i += VECTOR_WIDTH)
    { 
          // get padded mask 
          if (total_iterations == full_iterations) {
            paddedMask = _pp_init_ones(padding_count);
          } else {
            paddedMask = _pp_init_ones();
          }
          
          // create mask
          maskIfZero = _pp_init_ones(0);
          maskGreaterThanLimit = _pp_init_ones(0);
  
          _pp_vload_float(valueVector, values + i, paddedMask);
          _pp_vload_int(exponentVector, exponents + i, paddedMask);
          
          // if
          _pp_veq_int(maskIfZero, exponentVector, zero, paddedMask);  
          _pp_vload_float(result, vectorOnes, maskIfZero);

          // else
          maskIfNotZero = _pp_mask_not(maskIfZero); 
          _pp_vload_float(result, values + i, maskIfNotZero); 
          _pp_vsub_int(count, exponentVector, one, maskIfNotZero); 

          // while
          _pp_vgt_int(maskGreaterThanZero, count, zero, maskIfNotZero);
          while (_pp_cntbits(maskGreaterThanZero)) {
            _pp_vmult_float(result, result, valueVector, maskGreaterThanZero);
            _pp_vsub_int(count, count, one, maskIfNotZero);
            _pp_vgt_int(maskGreaterThanZero, count, zero, maskIfNotZero);
          }

          // if
          _pp_vgt_float(maskGreaterThanLimit, result, ceil, paddedMask); 
          _pp_vload_float(result, upperBoundVec, maskGreaterThanLimit);
          
          _pp_vstore_float(output + i, result, paddedMask);

          total_iterations++;
    }
}

// returns the sum of all elements in values
// You can assume N is a multiple of VECTOR_WIDTH
// You can assume VECTOR_WIDTH is a power of 2
float arraySumVector(float *values, int N)
{
  float sum = 0;
  //
  // PP STUDENTS TODO: Implement your vectorized version of arraySumSerial here
  //
  __pp_vec_float valueVector;
  __pp_vec_float result;
  
  __pp_mask maskAll;
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
    maskAll = _pp_init_ones();
     _pp_vload_float(valueVector, values + i, maskAll); 
     _pp_hadd_float(result, valueVector);
     _pp_interleave_float(result, result);

     for (int j=0; j < VECTOR_WIDTH; j++) {
        sum += result.value[j];
     }
  }
  return sum/2;
}