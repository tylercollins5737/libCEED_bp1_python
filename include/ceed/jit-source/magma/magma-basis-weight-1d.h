// Copyright (c) 2017-2025, Lawrence Livermore National Security, LLC and other CEED contributors.
// All Rights Reserved. See the top-level LICENSE and NOTICE files for details.
//
// SPDX-License-Identifier: BSD-2-Clause
//
// This file is part of CEED:  http://github.com/ceed

/// @file
/// Internal header for MAGMA tensor basis weight in 1D
#include "magma-common-tensor.h"

////////////////////////////////////////////////////////////////////////////////
// weight basis action -- 1D
template <typename T, int Q>
static __device__ __inline__ void magma_weight_1d_device(const T *sTweight, T *sV, const int tx) {
  // Assumptions
  // 1. 1D thread configuration of size Q
  // 2. The output sV is in shared memory -- size Q
  if (tx < Q) {
    sV[tx] = sTweight[tx];
  }
}

////////////////////////////////////////////////////////////////////////////////
extern "C" __launch_bounds__(MAGMA_BASIS_BOUNDS(BASIS_Q, MAGMA_MAXTHREADS_1D)) __global__
    void magma_weight_1d_kernel(const CeedScalar *dqweight1d, CeedScalar *dV, const int v_stride, const int nelem) {
  MAGMA_DEVICE_SHARED(CeedScalar, shared_data)

  const int tx      = threadIdx.x;
  const int ty      = threadIdx.y;
  const int elem_id = (blockIdx.x * blockDim.y) + ty;

  if (elem_id >= nelem) return;

  // global memory pointers
  dV += elem_id * v_stride;

  // shared memory pointers
  CeedScalar *sTweight = (CeedScalar *)shared_data;
  CeedScalar *sV       = sTweight + BASIS_Q;
  sV += ty * BASIS_Q;

  // read dqweight_1d
  if (ty == 0 && tx < BASIS_Q) {
    sTweight[tx] = dqweight1d[tx];
  }

  __syncthreads();
  magma_weight_1d_device<CeedScalar, BASIS_Q>(sTweight, sV, tx);
  __syncthreads();

  // write V
  dV[tx] = sV[tx];
}
