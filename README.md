# SIMD-Softmax
In this work we implement in C++ a SIMD-Softmax unit, that operates on either a single N-element vector or N/2 two-element vectors. This SIMD-Softmax unit used to implement a GELU activation function, that is used on BERT model for various benchmarks of the GLUE dataset. For evaluation, we run the pytorch model from [huggingface](https://huggingface.co/) and save the input matrices of GELU for the different tests included in [GLUE dataset](https://huggingface.co/datasets/nyu-mll/glue) to use the as GELU inputs in ```main.cpp```.

SIMD-Softmax depends only on ac_fixed library that is available in [HLSLibs](https://github.com/hlslibs/ac_types).

Also the post-synthesis RTL co-simultion of the given examples require the sc_verify flow of Catapult HLS. The necessary header (mc_scverify) is publicly available in [ac_simutils](https://github.com/hlslibs/ac_simutils/tree/master/include).



## Repository Hierarchy

This repository is organized as follows:

* ```main.cpp``` includes the testbench that reads the input matrices and feed them to the GELU activation function
* ```gelu.h``` includes the GELU and SIMD-Softmax implementations
* ```utils.h``` includes the max and adder trees used for the SIMD-Softmax
* ```file_io.h``` provide utility functions to read and write python (.npy) arrays
* ```ndmatrix_matrices.h``` utility header, used to define dynamically allocated matrices



## Pending Features

* Integrate accuracy and mean absolute error metrics
* Automate the tests, to run for all the BERT layers and benchmarks
* Implement other activation functions by reusing SIMD-Softmax



## Reference

The architecture and performance of the SIMD-Softmax used to implement GELU activation will be presented in IEEE International Conference on Artificial Intelligence Circuits and Systems, April 2024. You can find the paper [here](https://arxiv.org/abs/2402.10118). To cite this work, please use:

```
@inproceedings{fused-fp-dot,
author = {Peltekis, Christodoulos and Alexandridis, Kosmas and Dimitrakopoulos, Giorgos},
title = {Reusing Softmax Hardware Unit for GELU Computation in Transformers},
booktitle = {6th IEEE International Conference on Artificial Intelligence Circuits and Systems (AICAS)},
year = {2024},}
```



## Contributors

Currently active: [Christodoulos Peltekis](https://github.com/chrispelt), [Kosmas Alexandridis](https://github.com/kosmalex) and [Giorgos Dimitrakopoulos](https://github.com/gdimitrak)



## License

SIMD-Softmax is licensed with the MIT License. You are completely free to re-distribute your work derived from SIMD-Softmax