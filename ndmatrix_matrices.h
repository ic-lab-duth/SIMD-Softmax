/* Synthesizable struct for addressing a multi-dimensional
* array using a single wide Vector.
*
* Created By Dionisis Filippas 30/11/2022 */
#ifndef __NDMATRIX_MATRICES__
#define __NDMATRIX_MATRICES__



template<typename T, int N , int M, int K, int L>
struct Mat4d {
  #ifdef __SYNTHESIS__
    T ar[N*M*K*L];
    Mat4d(){};
    ~Mat4d(){};
  #else
    T* ar;
    Mat4d()  { ar = new T[N*M*K*L]; };
    ~Mat4d() { delete[] ar; };
  #endif

  struct Dim1 {
    Mat4d& _ar;
    int cur_n;
    Dim1(Mat4d& A, const int n) : _ar(A), cur_n(n) {};

    struct Dim2 {
      Dim1& d1;
      int cur_m;
      Dim2(Dim1& A, const int m) : d1(A), cur_m(m) {};

      struct Dim3 {
        Dim2& d2;
        int cur_k;
        Dim3(Dim2& A, const int k) : d2(A), cur_k(k) {};

        T& operator[] (const int l) { 
          return d2.d1._ar.ar[d2.d1.cur_n*M*K*L + d2.cur_m*K*L + cur_k*L + l];
        };
      };
      Dim3 operator[] (const int k) { return Dim3(*this, k); };
    };
    Dim2 operator[] (const int m) { return Dim2(*this, m); };
  };
  Dim1 operator[] (const int n) { return Dim1(*this, n); };
};// struct Mat4d


template<typename T, int N , int M, int K>
struct Mat3d {
  #ifdef __SYNTHESIS__
    T ar[N*M*K];
    Mat3d(){};
    ~Mat3d(){};
  #else
    T* ar;
    Mat3d()  { ar = new T[N*M*K]; };
    ~Mat3d() { delete[] ar; };
  #endif

  struct Dim1 {
    Mat3d& _ar;
    int cur_n;
    Dim1(Mat3d& A, const int n) : _ar(A), cur_n(n) {};

    struct Dim2 {
      Dim1& __ar;
      int cur_m;
      Dim2(Dim1& A, const int m) : __ar(A), cur_m(m) {};

      T& operator[] (const int k) { 
        return __ar._ar.ar[__ar.cur_n*M*K +cur_m*K + k]; 
      };
    };
    Dim2 operator[] (const int m) { return Dim2(*this, m); };
  };
  Dim1 operator[] (const int n) { return Dim1(*this, n); };
}; // struct Mat3d


template<typename T, int N , int M>
struct Mat2d {
  #ifdef __SYNTHESIS__
    T ar[N*M];
    Mat2d(){};
    ~Mat2d(){};
  #else
    T* ar;
    Mat2d()  { ar = new T[N*M]; };
    ~Mat2d() { delete[] ar; };
  #endif

  struct Dim1 {
    Mat2d& _ar;
    int cur_n;
    Dim1(Mat2d& A, const int n) : _ar(A), cur_n(n) {};

    T& operator[] (const int m) {
      return _ar.ar[cur_n*M + m];
    };
  };
  Dim1 operator[] (const int n) { return Dim1(*this, n); };
}; // struct Mat2d


template<typename T, int N>
struct Mat1d {
  #ifdef __SYNTHESIS__
    T ar[N];
    Mat1d(){};
    ~Mat1d(){};
  #else
    T* ar;
    Mat1d()  { ar = new T[N]{0}; };
    Mat1d(const Mat1d<T, N> &old)  {
      ar = new T[N]{0};

      for (int i = 0; i < N; i++) {
        ar[i] = old.ar[i];
      }
    }
    ~Mat1d() { delete[] ar; };
  #endif

  T& operator[] (const int n) {
    return ar[n];
  };
}; // struct Mat2d




#endif
