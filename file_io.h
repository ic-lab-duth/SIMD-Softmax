#ifndef __IO_HH__
#define __IO_HH__

#include "npy.hpp"

struct Shape {
  unsigned int B;
  unsigned int H;
  unsigned int W;

  /// For asseriton check in main
  bool
  operator==(const Shape other){
    return (this->B == other.B) && (this->H == other.H) && (this->W == other.W);
  }
};


/// @brief Read 1-d numpy array
/// @tparam npy_t the type of the numpy array to read
/// @tparam hls_t the type of the destination hls array,
/// i.e. an array with ac_<dtype> as the data type.
template<class npy_t, class hls_t, int N>
void
read_npy_1d(hls_t arr[N], const char *filename, Shape &shape) {
  npy::npy_data<float> np_arr = npy::read_npy<npy_t>(filename);

  shape.B = 1;
  shape.H = 1;
  shape.W = np_arr.shape[0];

  std::cout << filename << ": " << shape.H << ", " << shape.W << "\n";

  for (int i = 0; i < shape.W; i++) {
    arr[i] = np_arr.data[i];
  }
}

/// @brief Read 2-d numpy array
/// and return a pointer array
template<class npy_t, class hls_t, int N, int M>
void
read_npy_2d(hls_t arr[N][M], const char *filename, Shape &shape) {
  npy::npy_data<float> np_arr = npy::read_npy<float>(filename);

  shape.B = 1;
  shape.H = np_arr.shape[0];
  shape.W = np_arr.shape[1];

  std::cout << filename << ": " << shape.H << ", " << shape.W << "\n";

  for (int i = 0; i < shape.H; i++) {
    for (int j = 0; j < shape.W; j++) {
      arr[i][j] = np_arr.data[i * shape.W + j];
    }
  }
}

/// @brief Read 3-d numpy array
template<class npy_t, class hls_t, int B, int N, int M>
void
read_npy_3d(hls_t arr[B][N][M], const char *filename, Shape &shape) {
  npy::npy_data<float> np_arr = npy::read_npy<float>(filename);

  shape.B = np_arr.shape[0];
  shape.H = np_arr.shape[1];
  shape.W = np_arr.shape[2];

  std::cout << filename << ": " << shape.B << ", " << shape.H << ", " << shape.W << "\n";

  for (int i = 0; i < shape.B; i++) {
    for (int j = 0; j < shape.H; j++) {
      for (int k = 0; k < shape.W; k++) {
        arr[i][j][k] = np_arr.data[i * shape.H*shape.W + j * shape.W + k];
      }
    }
  }
}

/// @brief Convert an ac_<dtype> 1-d array into
/// C++ array
template<class cpp_t, class hls_t, int N>
void
convert_1d(cpp_t dst[N], hls_t src[N]) {
  for (int i = 0; i < N; i++) {
    dst[i] = (cpp_t)src[i].to_double();
  }
}

/// @brief Convert an ac_<dtype> 1-d array into
/// C++ array
template<class cpp_t, class hls_t, int N, int M>
void
convert_2d(cpp_t dst[N][M], hls_t src[N][M]) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      dst[i][j] = (cpp_t)src[i][j].to_double();
    }
  }
}

/// @brief Convert an ac_<dtype> 1-d array into
/// C++ array
template<class cpp_t, class hls_t, int B, int N, int M>
void
convert_3d(cpp_t dst[B][N][M], hls_t src[B][N][M]) {
  for (int i = 0; i < B; i++) {
    for (int j = 0; j < N; j++) {
      for (int k = 0; k < M; k++) {
        dst[i][j][k] = (cpp_t)src[i][j][k].to_double();
      }
    }
  }
}

/// @brief Write 1-d array to .npy file
template<class npy_t, class hls_t, int N>
void
write_npy_1d(const char *filename, hls_t arr[N], const Shape shape) {
  npy::npy_data_ptr<npy_t> np_arr;

  npy_t npy_arr[N];
  convert_1d<npy_t, hls_t, N>(npy_arr, arr);

  np_arr.data_ptr = npy_arr;
  np_arr.shape    = {shape.W};

  npy::write_npy(filename, np_arr);
}

/// @brief Write 2-d array to .npy file
template<class npy_t, class hls_t, int N, int M>
void
write_npy_2d(const char *filename, hls_t arr[N][M], const Shape shape) {
  npy::npy_data_ptr<npy_t> np_arr;

  npy_t npy_arr[N][M];
  convert_2d<npy_t, hls_t, N>(npy_arr, arr);

  np_arr.data_ptr = &npy_arr[0][0];
  np_arr.shape    = {shape.H, shape.W};

  npy::write_npy(filename, np_arr);
}

/// @brief Write 3-d array to .npy file
template<class npy_t, class hls_t, int B, int N, int M>
void
write_npy_3d(const char *filename, hls_t arr[B][N][M], const Shape shape) {
  npy::npy_data_ptr<npy_t> np_arr;

  npy_t npy_arr[B][N][M];
  convert_3d<npy_t, hls_t, N>(npy_arr, arr);

  np_arr.data_ptr = &npy_arr[0][0][0];
  np_arr.shape    = {shape.B, shape.H, shape.W};

  npy::write_npy(filename, np_arr);
}

#endif