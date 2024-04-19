#include <iostream>
#include <stdio.h>
#include <string>
#include "gelu.h"
#include <cmath>
#include "file_io.h"

#include "mc_scverify.h"


template <int N, int W, int I>
class gelu_tb {
  private:

    std::string name;

    size_t nRuns;
  public:
    gelu_tb(const char *name)
      : name(name), nRuns(0)
    {}

    void run(float *init_arr) {
      Mat1d<ac_fixed<  W,   I, true>, N*2> input;
      Mat1d<ac_fixed<  W,   I, true>, N*2> original_input;
      Mat1d<ac_fixed<2*W, 3*I, true>, N> output;
      
      
      printf("-------- Input --------\n");
      for(size_t i = 0; i < N*2; i++) {
        std::cout << init_arr[i] << ", ";
        original_input[i] = init_arr[i];
      }
      
      printf("\n-----------------------\n");

      ac_fixed<W, I, 0> Sout = ac_fixed<W, I, true>(0.03567740814);
      ac_fixed<W, I, 0> qc   = ac_fixed<W, I, true>(1.0f) / (ac_fixed<W, I, 0>(0.044715f));
      
      for (size_t i = 0; i < N*2; i++) {
        input[i] = init_arr[i];
      }

      gelu<N, W, I>(input, output, Sout,  qc);

      printf("-------- Input TB --------\n");
      for(size_t i = 0; i < N*2; i++) {
        std::cout << input[i].to_double() << ", ";
      }

      printf("\n-----------------------\n");
      printf("-------- GELU --------\n");
      for(size_t j = 0; j < N; j++) {
        std::cout << output[j].to_double() << ", ";
      }
      printf("\n-----------------------\n");
      printf("\n");

      this->nRuns++;
    }
};

const int B = 1;
const int M = 1;
const int N = 8;
const int Ng = N/2;
const int total_bits = 16;
const int Accuracy = 4;
const int nIntBits = 5;

const float max_v = 16.0;
const float min_v = -16.0;

int main() {

  float pytorch_input[B][M][N/2];
  Shape s;
  read_npy_3d<float, float, B, M, N/2>(pytorch_input, "./test.npy", s);

  for (int i = 0; i< s.H; i++) {
    for (int j = 0; j< s.W; j++) {
      std::cout << pytorch_input[0][i][j] << ", ";
    }
    std::cout << "\n";
  }
  
  gelu_tb<Ng, total_bits, nIntBits> gelu_tb ("GELU");

  float input[M][N];
  for (size_t m = 0; m < M; m++) {
    for(int n = 0; n < N; n+=2) {
      input[m][n  ] = pytorch_input[0][m][n/2];
      input[m][n+1] = 0;
    }
    
    gelu_tb.run(input[m]);
  }

  return 0;
}
