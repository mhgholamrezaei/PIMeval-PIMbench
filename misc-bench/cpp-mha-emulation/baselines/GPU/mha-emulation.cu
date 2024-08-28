/* File:     mha-emulation.cu
 * Purpose:  Implement an emulation of multi-head attention in transformers algorithm on a gpu using cuda
 *
 */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <iomanip>

#define TOLERANCE 1e-5

void generateRandomVector(std::vector<int32_t>& vec) {
    for (auto& elem : vec) {
        elem = rand() % 100;  // Random integer between 0 and 99
    }
}

void gemvBatchGPU(uint64_t rows, uint64_t cols, uint64_t batchSize) {
    // Allocate host memory for batch vectors
    std::vector<std::vector<int32_t>> Abatch(batchSize, std::vector<int32_t>(rows * cols));
    std::vector<std::vector<int32_t>> Bbatch(batchSize, std::vector<int32_t>(cols));
    std::vector<std::vector<int32_t>> Cbatch(batchSize, std::vector<int32_t>(rows));

    // Initialize vectors with random numbers
    for (uint64_t i = 0; i < batchSize; i++) {
        generateRandomVector(Abatch[i]);
        generateRandomVector(Bbatch[i]);
        Cbatch[i].resize(rows);
    }


    float *x, *y, *z;

    cudaError_t errorCode;

    // Allocate memory on GPU for each batch
    errorCode = cudaMalloc((void**)&x, batchSize * rows * cols * sizeof(float));
    if (errorCode != cudaSuccess) {
        std::cerr << "Cuda Error: " << cudaGetErrorString(errorCode) << "\n";
        exit(1);
    }

    errorCode = cudaMalloc((void**)&y, batchSize * cols * sizeof(float));
    if (errorCode != cudaSuccess) {
        std::cerr << "Cuda Error: " << cudaGetErrorString(errorCode) << "\n";
        exit(1);
    }

    errorCode = cudaMalloc((void**)&z, batchSize * rows * sizeof(float));
    if (errorCode != cudaSuccess) {
        std::cerr << "Cuda Error: " << cudaGetErrorString(errorCode) << "\n";
        exit(1);
    }

    cublasHandle_t handle;
    cublasStatus_t status = cublasCreate(&handle);
    if (status != CUBLAS_STATUS_SUCCESS) {
        std::cerr << "CUBLAS initialization failed\n";
        exit(1);
    }

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    float timeElapsed = 0;

    // Start timer
    cudaEventRecord(start, 0);

    // Process each batch
    for (uint64_t i = 0; i < batchSize; i++) {
        // Copy data to device for each batch
        errorCode = cudaMemcpy(x + i * rows * cols, Abatch[i].data(), rows * cols * sizeof(float), cudaMemcpyHostToDevice);
        if (errorCode != cudaSuccess) {
            std::cerr << "Cuda Error: " << cudaGetErrorString(errorCode) << "\n";
            exit(1);
        }

        errorCode = cudaMemcpy(y + i * cols, Bbatch[i].data(), cols * sizeof(float), cudaMemcpyHostToDevice);
        if (errorCode != cudaSuccess) {
            std::cerr << "Cuda Error: " << cudaGetErrorString(errorCode) << "\n";
            exit(1);
        }

        // GEMV kernel for each batch
        const float alpha = 1.0f;
        const float beta = 0.0f;
        status = cublasSgemv(handle, CUBLAS_OP_N, rows, cols, &alpha, x + i * rows * cols, rows, y + i * cols, 1, &beta, z + i * rows, 1);
        if (status != CUBLAS_STATUS_SUCCESS) {
            std::cerr << "CUBLAS SGEMV failed\n";
            exit(1);
        }

        // Copy result back to host for each batch
        errorCode = cudaMemcpy(Cbatch[i].data(), z + i * rows, rows * sizeof(float), cudaMemcpyDeviceToHost);
        if (errorCode != cudaSuccess) {
            std::cerr << "Cuda Error Copy from host to device: " << cudaGetErrorString(errorCode) << "\n";
            exit(1);
        }
    }

    // End timer
    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&timeElapsed, start, stop);

    // Free memory
    cublasDestroy(handle);
    cudaFree(x);
    cudaFree(y);
    cudaFree(z);
}

void multiHeadAttentionEmulationGPU() {
    const unsigned int num_heads = 96;
    const unsigned int dim_head = 128;
    const unsigned int num_layers = 96;
    const unsigned int embed_dim = num_heads * dim_head;

    auto start = std::chrono::high_resolution_clock::now();

    // Loop through each layer
    for (unsigned int layer = 0; layer < num_layers; ++layer) {
        std::cout << "DEBUG: layer = " << layer << std::endl;

        // Q, K, V GEMV1 operations in parallel (3 kernels)
        gemvBatchGPU(embed_dim, embed_dim, 3);

        // Score GEMV2 for 96 heads
        gemvBatchGPU(dim_head, 2049, num_heads);

        // Context GEMV3 for 96 heads
        gemvBatchGPU(2049, dim_head, num_heads);

        // O GEMV4
        gemvBatchGPU(embed_dim, embed_dim, 1);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsedTime = end - start;
    std::cout << "Duration: " << std::fixed << std::setprecision(3) << elapsedTime.count() << " ms." << std::endl;
}

int main() {
    multiHeadAttentionEmulationGPU();
    return 0;
}

