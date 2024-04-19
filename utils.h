#ifndef UTILS_H
#define UTILS_H

#include "mc_scverify.h"

namespace hls {
  namespace meta {
template<int N>
struct max_s {
  template<typename T>
  static T max(T *a, T *b) {
    T m0 = max_s<N/2  >::max(a    , b    );
    T m1 = max_s<N-N/2>::max(a+N/2, b+N/4);

    T max_v = m0 > m1 ? m0 : m1;

    return max_v;
  }
};

template<> 
struct max_s<1> {
  template<typename T>
  static T max(T *a, T *b) {
    return a[0];
  }
};

template<>
struct max_s<2> {
  template<typename T>
  static T max(T *a, T *b) {
    T m0 = max_s<1>::max(a  , (T*)0);
    T m1 = max_s<1>::max(a+1, (T*)0);

    T max_v = m0 > m1 ? m0 : m1;

    b[0] = max_v;

    return max_v;
  }
};

template<int N>
struct add_s {
  template<typename T, typename I>
  static T add(I *a, T* b) {
    T m0 = add_s<N/2>::add(a, b);
    T m1 = add_s<N-N/2>::add(a+N/2, b+N/4);

    return m0 + m1;
  }
};

template<> 
struct add_s<1> {
   template<typename T, typename I>
  static T add(I *a, T* b) {
    return a[0];
  }
};

template<> 
struct add_s<2> {
   template<typename T, typename I>
  static T add(I *a, T* b) {
    T m0 = add_s<1>::add(a  , (T*)0);
    T m1 = add_s<1>::add(a+1, (T*)0);

    T sum = m0 + m1;

    b[0] = sum;

    return sum;
  }
};

template<int N, typename T>
T max(T *a, T *b) {
  return max_s<N>::max(a, b);
};

template<int N, typename I, typename T>
T add(I *a, T* b) {
  return add_s<N>::add(a, b);
};
  }
}
#endif
