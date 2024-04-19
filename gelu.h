#ifndef GELU_H
#define GELU_H

#include "ac_fixed.h"
#include "ndmatrix_matrices.h"
#include <fstream>

#include "utils.h"
#include "mc_scverify.h"

#define clog2(x) ac::log2_ceil<x>::val
  
template< int W, int I >
void split(ac_int<I, 0>          &_int,
           ac_fixed<W-I+1, 1, 1> &_frac,
     const ac_fixed<W, I, 1>     &in
    ){
      ac_fixed<W, I, 1> in_eff;
                        in_eff = in;

      // Compute the 2's complement. Invert and add
      // a 1 to the LSB. Because its fixed-point rep,
      // the LSB's weight is (1 >> (W-I)).
      in_eff = ac_fixed<W, I, 0> (
        ~in_eff + ( ac_fixed<W-I+1, 1, 0>(1) >> (W-I) )
      );

      _int.set_slc(0, in_eff.template slc<I>( (unsigned)(W-I) ));
      
      _frac = ac_fixed<W-I+1, 1, 1>(in + _int);
    }
  

template< int W, int I >
void pow2_pwl(
        ac_fixed<W-I+1, 1, 0> &a,
        ac_fixed<W-I+1, 1, 0> &b,
  const ac_fixed<W-I+1, 1, 1> &x
){  
  /**
   * @note If x is zero, just output 1 no need to pwl.
  */

  const ac_fixed<W-I+1, 1, 0>
  alpha_lut[8] = {
    0.663764778033316,
    0.608423508606172,
    0.557994093565377,
    0.511664863403061,
    0.469203088566355,
    0.430261502403925,
    0.394545500518956,
    0.361823424728051
  };

  const ac_fixed<W-I+1, 1, 0> 
  beta_lut[8] = {
    0.999395521280818,
    0.992477862602425,
    0.979870508842226,
    0.962497047531358,
    0.941266160113005,
    0.916927668761487,
    0.89014066734776,
    0.861508851030718
  };

  // Take absolute before indexing.
  ac_fixed<W-I+1, 1, 0> x_abs = x[W-I] ? (ac_fixed<W-I+1, 1, 0>)(~x + ac_fixed<W-I+1, 1, 0>().template set_val<AC_VAL_QUANTUM>())
                                       : (ac_fixed<W-I+1, 1, 0>)x;

  /**
    * @param index is the 3 MSBs of the fraction.
    * Here x is <1>.<W-I> --> W-I+1 bits in total
  */
  ac_int<3, false> index;
                    index.set_slc(0, x_abs.template slc<3>(W-I+1 - 4));
  
  ac_int<1, false> is_zero = (x == 0);

  a = is_zero ? ac_fixed<W-I+1, 1, 0>(0.0f) : alpha_lut[index];
  b = is_zero ? ac_fixed<W-I+1, 1, 0>(1.0f) : beta_lut[index] ;
}
  
template< int W, int I >
void exp_pwl(
        ac_fixed<W, I, 0> &exp,
  const ac_fixed<W, I, 1> &x
){
  ac_fixed<W, I, 1> x_eff;
                    x_eff = x + (x >> 1) 
                              - (x >> 4) 
                              + (x >> 6) - (x >> 7);

  ac_int<I, 0>            _int; 
  ac_fixed<W-I + 1, 1, 1> _frac;
  split<W, I>(_int, _frac, x_eff);
  
  ac_fixed<W-I+1, 1, 0> _pow2;
  ac_fixed<W-I+1, 1, 0> a;
  ac_fixed<W-I+1, 1, 0> b;
  pow2_pwl<W, I>(a, b, _frac);
  
  _pow2 = a * _frac + b;
  
  exp = _pow2 >> _int;
}

template< int W, int I >
void log2_pwl(
        ac_fixed<W-I+2, 2, 0> &a,
        ac_fixed<W-I+1, 1, 0> &b,
  const ac_fixed<W-I+1, 1, 0> &f
){
  const ac_fixed<W-I+2, 2, 0>
  alpha_lut[8] = {
    175.0f/128.0f,
    155.0f/128.0f,
    142.0f/128.0f,
    129.0f/128.0f,
    119.0f/128.0f,
    110.0f/128.0f,
    102.0f/128.0f,
    95.0f/128.0f
  };

  const ac_fixed<W-I+1, 1, 0> 
  beta_lut [8] = {
    0.0f,
    20.0f/1024.0f,
    46.0f/1024.0f,
    84.0f/1024.0f,
    123.0f/1024.0f,
    167.0f/1024.0f,
    215.0f/1024.0f,
    264.0f/1024.0f
  };
  
  /**
    * @param index is the 3 MSBs of the fraction.
    * Here x is <1>.<W-I> --> W-I+1 bits in total
  */
  ac_int<3, false> index;
                    index.set_slc(0, f.template slc<3>(W-I+1 - 4));
  a = alpha_lut[index];
  b = beta_lut[index];
}
  
template< int I >
void arbiter(
        ac_int<I, 0> &o,
  const ac_int<I, 0>  x[I],
  const ac_int<I, 0> &one_hot
){
  ac_int<1, false> flag;
  for (int i = I; i >= 0; i--) {
    if (i == I) {
      flag = 0;
    } else {
      if (!one_hot[i] && !flag){
        o = x[i];
        flag = 1;
      }
    }
  }
}
  
template< int I >
void lzcount(
        ac_int<I, 0> &lzc,
  const ac_int<I, 0> &x
){
  ac_int<I, 0> x_inv = ac_int<I, 0>(~x);
  
  ac_int<I, 0> acc[I];

  for(int i = I-1; i >= 0; i--) {
    if (i == I-1) {
      acc[I-1] = x_inv[(unsigned)(I-1)];
    } else {
      acc[i] = x_inv[i] + acc[i + 1];
    }

  }

  arbiter<I>(lzc, acc, x_inv);
}
  
template< int N, int W, int I >
void ln_pwl(
        ac_fixed<W, I, 0>                       &ln,
  const ac_fixed<clog2(N) + (W-I), clog2(N), 0> &x
){
  // Extract integer part
  ac_int<clog2(N), 0> _int;
                      _int.set_slc(0, x.template slc<clog2(N)>( (unsigned)(W-I) ));


  // Count leading zeros
  ac_int<clog2(N), 0> lzc;
  lzcount<clog2(N)>(lzc, _int);

  // Leading 1's position
  ac_int<clog2(N), 0> k = clog2(N) - lzc - 1;

  // Get the fractional part
  ac_fixed<W-I+1, 1, 0> _frac = 0;
                        _frac.set_slc(0, x.template slc<W-I>(0 + k));

  
  ac_fixed<(W-I+2), 2, 0> a;
  ac_fixed<(W-I+1), 1, 0> b;
  ac_fixed<(W-I+1), 1, 0> _log2;

  log2_pwl<W, I>(a, b, _frac);
  
  _log2 = a * _frac + b;
  

  ac_fixed<clog2(N) + (W-I), clog2(N), 0> _log2_complete(0);
                           _log2_complete.set_slc(0, _log2.template slc<W-I>(0));
                           _log2_complete.set_slc((unsigned)(W-I), k.template slc<clog2(N)>(0));

  
  ln = _log2_complete * ac_fixed<W, I, 0>(0.69314718056); 

}

#pragma hls_design top
template<int N, int W, int I>
void softmax(
        Mat1d<ac_fixed<W, I, true>, N> &X,
        Mat1d<ac_fixed<W, I, true>, N> &Y,
  const ac_int<1, false>               &mode_sel
){
  static ac_fixed<W, I, true> inp_buffer[N];

  // Read input and buffer it.
  #pragma hls_unroll yes
  INP_READ: for(int i = 0; i < N; i++) inp_buffer[i] = X[i];

  // Find the maxima of interest
  ac_fixed<W, I, 1> max2[N/2];
  ac_fixed<W, I, 1> top_max;

  top_max = hls::meta::max<N, ac_fixed<W, I, 1> >(inp_buffer, max2);


  // Transform
  ac_fixed<W, I, true> diff[N];
  
  #pragma hls_unroll yes
  DIFF: for (int i = 0; i < N; i++) {
    ac_fixed<W, I, 1> max_val = (mode_sel) ? max2[i/2] : top_max;
    diff[i] = inp_buffer[i] - max_val; 
  }

  // Calculate exp
  ac_fixed<W, I, 0> exps[N];
  ac_fixed<W, I, 0> exps_test[N];
  
  #pragma hls_unroll yes
  EXP_PWL: for (int i = 0; i < N; i++){
    exp_pwl<W, I>(exps[i], diff[i]);
  }


  // Sum of exponents using a tree
  ac_fixed<clog2(N) + (W-I), clog2(N), 0> exp_sum2[N/2];
  ac_fixed<clog2(N) + (W-I), clog2(N), 0> tot_exp_sum;
  
  tot_exp_sum = hls::meta::add<N, ac_fixed<W, I, 0>, ac_fixed<clog2(N) + (W-I), clog2(N), 0> >(exps, exp_sum2);
  

  // Calculate ln
  ac_fixed<W, I, 0> lns[N/2];
  
  #pragma hls_unroll yes
  LN_PWL: for (int i = 0; i < N/2; i++) {
    if (i == 0) {
      ac_fixed<clog2(N) + (W-I), clog2(N), 0> exp_sum;
                                              exp_sum = mode_sel ? exp_sum2[0] : tot_exp_sum;
  
      ln_pwl<N, W, I>(lns[0], exp_sum);
    } else {
      ln_pwl<N, W, I>(lns[i], exp_sum2[i]);
    }
  }


  // Calculate: xi-max - ln(sum(exp(xi-max)))
  ac_fixed<W+1, I+1, 1> diff_ln[N];
  
  #pragma hls_unroll yes
  DIFF_LN: for (int i = 0; i < N; i++) {
    ac_fixed<W, I, 1> ln = mode_sel ? lns[i/2] : lns[0];
    diff_ln[i] = diff[i] - ln;
  }

  // Calculate final softmax output
  ac_fixed<W, I, 0> output[N];
  
  #pragma hls_unroll yes
  COMP_OUTPUT: for (int i = 0; i < N; i++) {
    exp_pwl<W, I>(output[i], diff_ln[i]);
  }

  // Write output
  #pragma hls_unroll yes
  WRITE_OUTPUT: for (int i = 0; i < N; i++) {
    Y[i].set_slc(0, output[i].template slc<W>(0));
  }
}
  
template <int N, int W, int I>
void poly_q(
  Mat1d<ac_fixed<2*W, 3*I, true>, N*2> &P,
  Mat1d<ac_fixed<  W,   I, true>, N*2>  Xq,
  const ac_fixed<W, I, false>   S,
  const ac_fixed<W, I, false>   qc
){
  
  #pragma hls_unroll yes
  for (int i = 0; i < N*2; i+=2) {
    ac_fixed<W, I, true> xq_accent = Xq[i];

    ac_fixed<2*W  , 3*I, true>                  xq_accent_cube_eff;
    
    xq_accent_cube_eff = xq_accent * xq_accent * xq_accent;
    
    ac_fixed<2*W, 3*I, true> poly_res = (xq_accent_cube_eff + qc * xq_accent) * S;
    
    P[i]   = poly_res;
    P[i+1] = ac_fixed<2*W, 3*I, true>(~poly_res + ac_fixed<2*W, 3*I, true>().template set_val<AC_VAL_QUANTUM>());
  }
  
}


#pragma hls_design top
template <int N, int W, int I>
void CCS_BLOCK(gelu)(
         Mat1d<ac_fixed<W, I, true>, N*2>   &X,
         Mat1d<ac_fixed<2*W, 3*I, true>, N> &G,
         const ac_fixed<W, I, false> S,
         const ac_fixed<W, I, false> qc
         ){
  
  Mat1d<ac_fixed<2*W, 3*I, true>, N*2> X_eff;
  Mat1d<ac_fixed<2*W, 3*I, true>, N*2> Y;
  
  poly_q<N, W, I>(X_eff, X, S, qc);
  
  softmax<N*2, 2*W, 3*I>(X_eff, Y, 1);
  
  #pragma hls_unroll yes
  for (int i = 0; i < N; i++) {
    ac_fixed<W, I, true> y;
    y.set_slc(0, Y[2*i].template slc<W>(6));
    G[i] = y * X[2*i];
  }
}


#endif
