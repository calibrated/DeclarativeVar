#include <thrust/device_vector.h>
#include <thrust/transform.h>
#include <thrust/functional.h>

template <typename T>
struct parallelMatrixMultiplyFunctor {
    const T* A;
    const T* B;
    T* C;
    int N;
    int ar;
    int ac;
    int br;
    int bc;
    parallelMatrixMultiplyFunctor(const T* A_, const T* B_, T* C_, int  parallelN, int arow, int acol, int bcol ):
        A(A_), B(B_), C(C_), N(parallelN), ar(arow), ac(acol), bc(bcol) 
    {
    }

    __device__ void operator()(int idx) {
        // Index of the matrix
        const float* A_mat = A + idx * ar * ac;
        const float* B_mat = B + idx * ac * bc;
        float* C_mat = C + idx * ar * bc;
        // Perform 6x6 matrix multiplication
        for (int row = 0; row < ar; ++row) {
            float sum = 0.0f;
            for (int col = 0; col < ac; ++col) 
            {
                for (int k = 0; k < ac; ++k) {
                    sum += A_mat[row * ac + k] * B_mat[k * bc + col];
                }
                C_mat[row * ac + col] = sum;
            }
        }
    }
};
template <typename T>
void parallelMatrixMultiply(int N,int ar,int ac, int bc, thrust::device_vector<T>& d_A, thrust::device_vector<T>& d_B, thrust::device_vector<float>& d_res)
{
    thrust::for_each(
        thrust::counting_iterator<int>(0),
        thrust::counting_iterator<int>(N),
        parallelMatrixMultiplyFunctor(
            thrust::raw_pointer_cast(d_A.data()),
            thrust::raw_pointer_cast(d_B.data()),
            thrust::raw_pointer_cast(d_res.data(), N,
                ar,ac,bc
            )
        )
    );
}