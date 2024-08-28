// Test: C++ version of multi-head attention function  
// Copyright (c) 2024 University of Virginia
// This file is licensed under the MIT License.
// See the LICENSE file in the root of this repository for more details.

#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <iomanip>
#include <getopt.h>

#include "libpimeval.h"
#include "../../util.h"

// Params ---------------------------------------------------------------------
typedef struct Params
{
  char *configFile;
} Params;

void usage()
{
  fprintf(stderr,
          "\nUsage:  ./gemv.out [options]"
          "\n"
          "\n    -c    dramsim config file"
          "\n    -v    t = verifies PIM output with host output. (default=false)"
          "\n");
}

struct Params getInputParams(int argc, char **argv)
{
  struct Params p;
  p.configFile = nullptr;

  int opt;
  while ((opt = getopt(argc, argv, "h:c")) >= 0)
  {
    switch (opt)
    {
    case 'h':
      usage();
      exit(0);
      break;
    case 'c':
      p.configFile = optarg;
      break;
   default:
      fprintf(stderr, "\nUnrecognized option!\n");
      usage();
      exit(0);
    }
  }
  return p;
}


void gemvBatch(uint64_t row, uint64_t col, uint64_t batchSize, const std::vector<std::vector<int>>& srcVectors,
               const std::vector<std::vector<std::vector<int>>>& srcMatrices, std::vector<std::vector<int64_t>>& dstBatch) {
    PimStatus status = PIM_OK;
    unsigned bitsPerElement = sizeof(int) * 8;
    PimObjId srcObj1 = pimAlloc(PIM_ALLOC_AUTO, col * batchSize, bitsPerElement, PIM_INT32);
    if (srcObj1 == -1) {
        std::cout << "Abort" << std::endl;
        return;
    }
    PimObjId srcObj2 = pimAllocAssociated(bitsPerElement, srcObj1, PIM_INT32);
    if (srcObj2 == -1) {
        std::cout << "Abort" << std::endl;
        return;
    }
    PimObjId dstObj = pimAllocAssociated(bitsPerElement, srcObj1, PIM_INT32);
    if (dstObj == -1) {
        std::cout << "Abort" << std::endl;
        return;
    }

    for (uint64_t batchId = 0; batchId < batchSize; batchId++) {
        status = pimCopyHostToDevice((void *)srcVectors[batchId].data(), srcObj2, batchId * col, (batchId + 1) * col);
        if (status != PIM_OK) {
            std::cout << "Abort" << std::endl;
            return;
        }
        dstBatch[batchId].reserve(row);
    }

    for (uint64_t i = 0; i < row; ++i) {
        for (uint64_t batchId = 0; batchId < batchSize; batchId++) {
            status = pimCopyHostToDevice((void *)srcMatrices[batchId][i].data(), srcObj1, batchId * col, (batchId + 1) * col);
            if (status != PIM_OK) {
                std::cout << "Abort" << std::endl;
                return;
            }
        }

        status = pimMul(srcObj1, srcObj2, dstObj);
        if (status != PIM_OK) {
            std::cout << "Abort" << std::endl;
            return;
        }

        for (uint64_t batchId = 0; batchId < batchSize; batchId++) {
            status = pimRedSumRangedInt(dstObj, batchId * col, (batchId + 1) * col, &dstBatch[batchId][i]);
            if (status != PIM_OK) {
                std::cout << "Abort" << std::endl;
                return;
            }
        }
    }

    pimFree(srcObj1);
    pimFree(srcObj2);
    pimFree(dstObj);
}

void multiHeadAttentionEmulationPIM() {
    const unsigned int num_heads = 96;
    const unsigned int dim_head = 128;
    const unsigned int num_layers = 96;
    const unsigned int embed_dim = num_heads * dim_head;

    auto start = std::chrono::high_resolution_clock::now();

    // Loop through each layer
    for (unsigned int layer = 0; layer < num_layers; ++layer) {
        std::cout << "DEBUG: layer = " << layer << std::endl;  

        std::vector<std::vector<int>> QKV_vectors(3, std::vector<int>(embed_dim)); // Placeholder for Q, K, V vectors
        std::vector<std::vector<std::vector<int>>> QKV_matrices(3, std::vector<std::vector<int>>(embed_dim, std::vector<int>(embed_dim)));
        std::vector<std::vector<int64_t>> QKV_results(3, std::vector<int64_t>(embed_dim));

        // Q, K, V GEMV1 operations in parallel (3 kernels)
        gemvBatch(embed_dim, embed_dim, 3, QKV_vectors, QKV_matrices, QKV_results);

        std::vector<std::vector<int>> score_vectors(num_heads, std::vector<int>(dim_head)); // Placeholder for score vectors
        std::vector<std::vector<std::vector<int>>> score_matrices(num_heads, std::vector<std::vector<int>>(2049, std::vector<int>(dim_head)));
        std::vector<std::vector<int64_t>> score_results(num_heads, std::vector<int64_t>(dim_head));

        // Score GEMV2 for 96 heads
        gemvBatch(dim_head, 2049, num_heads, score_vectors, score_matrices, score_results);

        std::vector<std::vector<int>> context_vectors(num_heads, std::vector<int>(2049)); // Placeholder for context vectors
        std::vector<std::vector<std::vector<int>>> context_matrices(num_heads, std::vector<std::vector<int>>(dim_head, std::vector<int>(2049)));
        std::vector<std::vector<int64_t>> context_results(num_heads, std::vector<int64_t>(2049));

        // Context GEMV3 for 96 heads
        gemvBatch(2049, dim_head, num_heads, context_vectors, context_matrices, context_results);

        std::vector<std::vector<int>> output_vectors(1, std::vector<int>(embed_dim)); // Placeholder for output vectors
        std::vector<std::vector<std::vector<int>>> output_matrices(1, std::vector<std::vector<int>>(embed_dim, std::vector<int>(embed_dim)));
        std::vector<std::vector<int64_t>> output_results(1, std::vector<int64_t>(embed_dim));

        // O GEMV4
        gemvBatch(embed_dim, embed_dim, 1, output_vectors, output_matrices, output_results);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTime = end - start;
    std::cout << "Info: Duration: " << std::fixed << std::setprecision(3) << elapsedTime.count() << " ms." << std::endl;
}

int main(int argc, char *argv[]) { 
    struct Params params = getInputParams(argc, argv);
 
    if (!createDevice(params.configFile))
    {
        return 1;
    }
    multiHeadAttentionEmulationPIM();
    return 0;
}

