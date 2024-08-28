/**
 * @file mha-emulation.cpp
 * @brief MHA emulation.
 */

#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <cblas.h>

// Function to generate random int32_t vectors
void generateRandomVector(std::vector<int32_t>& vec) {
    for (auto& elem : vec) {
        elem = rand() % 100;  // Random integer between 0 and 99
    }
}

// Function to perform GEMV operations for a batch of matrices using int32_t vectors
void gemvBatch(uint64_t rows, uint64_t cols, uint64_t batchSize) {
    int32_t alpha = 1;
    int32_t beta = 1;

    std::vector<std::vector<int32_t>> Abatch(batchSize, std::vector<int32_t>(rows * cols));
    std::vector<std::vector<int32_t>> Bbatch(batchSize, std::vector<int32_t>(cols));
    std::vector<std::vector<int32_t>> Cbatch(batchSize, std::vector<int32_t>(rows));

    // Initialize matrices and vectors with random numbers
    for (uint64_t i = 0; i < batchSize; i++) {
        generateRandomVector(Abatch[i]);
        generateRandomVector(Bbatch[i]);
    }

    // Perform GEMV for each batch
    for (uint64_t i = 0; i < batchSize; i++) {
        cblas_sgemv(CblasRowMajor, CblasNoTrans, rows, cols, static_cast<float>(alpha), 
                    reinterpret_cast<float*>(Abatch[i].data()), cols, 
                    reinterpret_cast<float*>(Bbatch[i].data()), 1, 
                    static_cast<float>(beta), 
                    reinterpret_cast<float*>(Cbatch[i].data()), 1);
    }
}

// Function to emulate multi-head attention using GEMV operations
void multiHeadAttentionEmulation() {
    const unsigned int num_heads = 96;
    const unsigned int dim_head = 128;
    const unsigned int num_layers = 96;
    const unsigned int embed_dim = num_heads * dim_head;

    auto start = std::chrono::high_resolution_clock::now();

    // Loop through each layer
    for (unsigned int layer = 0; layer < num_layers; ++layer) {
        std::cout << "DEBUG: layer = " << layer << std::endl;

        // Q, K, V GEMV1 operations in parallel (3 kernels)
        gemvBatch(embed_dim, embed_dim, 3);

        // Score GEMV2 for 96 heads
        gemvBatch(dim_head, 2049, num_heads);

        // Context GEMV3 for 96 heads
        gemvBatch(2049, dim_head, num_heads);

        // O GEMV4
        gemvBatch(embed_dim, embed_dim, 1);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsedTime = end - start;
    std::cout << "Info: Duration: " << std::fixed << std::setprecision(3) << elapsedTime.count() << " ms." << std::endl;
}

int main() {
    multiHeadAttentionEmulation();
    return 0;
}

