// See LICENSE for license details.

#ifndef SRC_MAIN_C_GEMMINI_H
#define SRC_MAIN_C_GEMMINI_H

#undef abs

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <stdbool.h>

#include "include/gemmini_params.h"

#define GEMMINI_ASSERTIONS

// Accelerator interface
#include "rocc-software/src/xcustom.h"

#define k_CONFIG 0
#define k_MVIN2 1
#define k_MVIN 2
#define k_MVOUT 3
#define k_COMPUTE_PRELOADED 4
#define k_COMPUTE_ACCUMULATE 5
#define k_PRELOAD 6
#define k_FLUSH 7

#define k_LOOP_WS 8
#define k_LOOP_WS_CONFIG_BOUNDS 9
#define k_LOOP_WS_CONFIG_ADDRS_AB 10
#define k_LOOP_WS_CONFIG_ADDRS_DC 11
#define k_LOOP_WS_CONFIG_STRIDES_AB 12
#define k_LOOP_WS_CONFIG_STRIDES_DC 13

#define k_MVIN3 14

#define k_MVIN_SP_CONFIG 18
#define k_MVIN_SP_COO 19

//============================================================================
// gemmini-cisc opcodes
//============================================================================
#define k_CONFIG_CISC_EX 10
#define k_ADDR_AB        11
#define k_ADDR_CD        12
#define k_SIZE0          13
#define k_SIZE1          14
#define k_RPT_BIAS       15
#define k_RESET          16
#define k_COMPUTE_CISC   17
//============================================================================

#define CONFIG_EX 0
#define CONFIG_LD 1
#define CONFIG_ST 2
#define CONFIG_IM2COL 3

#define XCUSTOM_ACC 3

#define GARBAGE_ADDR ((uint32_t)(-1))
#define OUTPUT_STATIONARY 0
#define WEIGHT_STATIONARY 1

#define NO_ACTIVATION 0
#define RELU 1
#define RELU6 2

#ifdef ELEM_T_IS_FLOAT
elem_t elem_t_bits_to_elem_t(elem_t_bits x) {
    union {
        elem_t_bits b;
        elem_t f;
    } un;

    un.b = x;
    return un.f;
}

elem_t_bits elem_t_to_elem_t_bits(elem_t x) {
    union {
        elem_t_bits b;
        elem_t f;
    } un;

    un.f = x;
    return un.b;
}

acc_t acc_t_bits_to_acc_t(acc_t_bits x) {
    union {
        acc_t_bits b;
        acc_t f;
    } un;

    un.b = x;
    return un.f;
}

acc_t_bits acc_t_to_acc_t_bits(acc_t x) {
    union {
        acc_t_bits b;
        acc_t f;
    } un;

    un.f = x;
    return un.b;
}

bool elem_t_isnan(elem_t x) {
    elem_t_bits bits = elem_t_to_elem_t_bits(x);
    uint64_t exp = (bits >> (ELEM_T_SIG_BITS-1)) & (((uint64_t)1 << ELEM_T_EXP_BITS) - 1);
    uint64_t sig = bits & (((uint64_t)1 << ELEM_T_SIG_BITS) - 1);
    bool is_nan_or_inf = exp == (((uint64_t)1 << ELEM_T_EXP_BITS) - 1);
    bool is_not_inf = sig != 0;
    return is_nan_or_inf && is_not_inf;
}

bool acc_t_isnan(acc_t x) {
    acc_t_bits bits = acc_t_to_acc_t_bits(x);
    uint64_t exp = (bits >> (ACC_T_SIG_BITS-1)) & (((uint64_t)1 << ACC_T_EXP_BITS) - 1);
    uint64_t sig = bits & (((uint64_t)1 << ACC_T_SIG_BITS) - 1);
    bool is_nan_or_inf = exp == (((uint64_t)1 << ACC_T_EXP_BITS) - 1);
    bool is_not_inf = sig != 0;
    return is_nan_or_inf && is_not_inf;
}
#endif

#ifdef HAS_MVIN_SCALE
scale_t scale_t_bits_to_scale_t(scale_t_bits x) {
    union {
        scale_t_bits b;
        scale_t f;
    } un;

    un.b = x;
    return un.f;
}

scale_t_bits scale_t_to_scale_t_bits(scale_t x) {
    union {
        scale_t_bits b;
        scale_t f;
    } un;

    un.f = x;
    return un.b;
}
#endif

#ifdef HAS_MVIN_ACC_SCALE
scale_acc_t scale_acc_t_bits_to_scale_acc_t(scale_acc_t_bits x) {
    union {
        scale_acc_t_bits b;
        scale_acc_t f;
    } un;

    un.b = x;
    return un.f;
}

scale_acc_t_bits scale_acc_t_to_scale_acc_t_bits(scale_acc_t x) {
    union {
        scale_acc_t_bits b;
        scale_acc_t f;
    } un;

    un.f = x;
    return un.b;
}
#endif

acc_scale_t acc_scale_t_bits_to_acc_scale_t(acc_scale_t_bits x) {
    union {
        acc_scale_t_bits b;
        acc_scale_t f;
    } un;

    un.b = x;
    return un.f;
}

acc_scale_t_bits acc_scale_t_to_acc_scale_t_bits(acc_scale_t x) {
    union {
        acc_scale_t_bits b;
        acc_scale_t f;
    } un;

    un.f = x;
    return un.b;
}

#define ROCC_INSTRUCTION_RS1_RS2(x, rs1, rs2, funct) \
  ROCC_INSTRUCTION_0_R_R(x, rs1, rs2, funct)

#define gemmini_extended_mvin_sparse_coo(dram_addr_dat, dram_addr_ind, array_dim, spad_addr, start_col, cols, start_row, rows) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, dram_addr_dat, dram_addr_ind, k_MVIN_SP_CONFIG) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(rows) << (ADDR_LEN + 16)) | ((uint64_t)(cols) << ADDR_LEN) | (spad_addr), ((uint64_t) (array_dim) << (32)) | ((uint64_t)(start_row) << (16)) | (uint64_t)(start_col), k_MVIN_SP_COO)

#define gemmini_extended_mvin(dram_addr, spad_addr, cols, rows) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, dram_addr, ((uint64_t)(rows) << (ADDR_LEN + 16)) | ((uint64_t)(cols) << ADDR_LEN) | (spad_addr), k_MVIN)

#define gemmini_extended_mvin2(dram_addr, spad_addr, cols, rows) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, dram_addr, ((uint64_t)(rows) << (ADDR_LEN + 16)) | ((uint64_t)(cols) << ADDR_LEN) | (spad_addr), k_MVIN2)

#define gemmini_extended_mvin3(dram_addr, spad_addr, cols, rows) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, dram_addr, ((uint64_t)(rows) << (ADDR_LEN + 16)) | ((uint64_t)(cols) << ADDR_LEN) | (spad_addr), k_MVIN3)

#define gemmini_block_mvin(dram_addr, spad_addr, len) \
  gemmini_extended_mvin(dram_addr, spad_addr, (len) * DIM, DIM)

#define gemmini_mvin(dram_addr, spad_addr) \
  gemmini_extended_mvin(dram_addr, spad_addr, DIM, DIM)

#define gemmini_extended_mvout(dram_addr, spad_addr, cols, rows) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, dram_addr, ((uint64_t)(rows) << (ADDR_LEN + 16)) | ((uint64_t)(cols) << ADDR_LEN) | (uint64_t)(spad_addr), k_MVOUT)

#define gemmini_mvout(dram_addr, spad_addr) \
  gemmini_extended_mvout(dram_addr, spad_addr, DIM, DIM)

// compute
#define gemmini_extended_compute_preloaded(A, BD, A_cols, A_rows, BD_cols, BD_rows) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(A_rows) << (ADDR_LEN + 16)) | ((uint64_t)(A_cols) << ADDR_LEN) | (uint64_t)(A), ((uint64_t)(BD_rows) << (ADDR_LEN + 16)) | ((uint64_t)(BD_cols) << ADDR_LEN) | (uint64_t)(BD), k_COMPUTE_PRELOADED)

#define gemmini_extended_compute_accumulated(A, BD, A_cols, A_rows, BD_cols, BD_rows) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(A_rows) << (ADDR_LEN + 16)) | ((uint64_t)(A_cols) << ADDR_LEN) | (uint64_t)(A), ((uint64_t)(BD_rows) << (ADDR_LEN + 16)) | ((uint64_t)(BD_cols) << ADDR_LEN) | (uint64_t)(BD), k_COMPUTE_ACCUMULATE)

#define gemmini_compute_preloaded(A, BD) \
  gemmini_extended_compute_preloaded(A, BD, DIM, DIM, DIM, DIM)

#define gemmini_compute_accumulated(A, BD) \
  gemmini_extended_compute_accumulated(A, BD, DIM, DIM, DIM, DIM)

// preload
#define gemmini_extended_preload(BD, C, BD_cols, BD_rows, C_cols, C_rows) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(BD_rows) << (ADDR_LEN + 16)) | ((uint64_t)(BD_cols) << ADDR_LEN) | (uint64_t)(BD), ((uint64_t)(C_rows) << (ADDR_LEN + 16)) | ((uint64_t)(C_cols) << ADDR_LEN) | (uint64_t)(C), k_PRELOAD)

#define gemmini_preload(BD, C) \
  gemmini_extended_preload(BD, C, DIM, DIM, DIM, DIM)

#define gemmini_preload_zeros(C) \
  gemmini_preload(GARBAGE_ADDR, C)

// weight-stationary matmul loop
#define gemmini_loop_ws(I, J, K, pad_I, pad_J, pad_K, A, B, D, C, A_stride, B_stride, D_stride, C_stride, A_transpose, B_transpose, full_C, low_D, ex_accumulate) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(pad_K) << 32) | ((uint64_t)(pad_J) << 16) | (uint64_t)(pad_I), ((uint64_t)(K) << 32) | ((uint64_t)(J) << 16) | (uint64_t)(I), k_LOOP_WS_CONFIG_BOUNDS) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, A, B, k_LOOP_WS_CONFIG_ADDRS_AB) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, D, C, k_LOOP_WS_CONFIG_ADDRS_DC) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, A_stride, B_stride, k_LOOP_WS_CONFIG_STRIDES_AB) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, D_stride, C_stride, k_LOOP_WS_CONFIG_STRIDES_DC) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((low_D) << 2) | ((full_C) << 1) | (ex_accumulate), ((B_transpose) << 1) | (A_transpose), k_LOOP_WS)

// config
#define gemmini_extended2_config_ex(dataflow, act, sys_shift, acc_scale, relu6_shift, A_stride, A_transpose, B_transpose, ocol, row_turn, kdim, stride, channel, row_left, kdim2, weight_double_bank, weight_triple_bank) \
  { \
    ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)acc_scale_t_to_acc_scale_t_bits((acc_scale_t)acc_scale) << 32) | ((uint64_t)(A_stride) << 16) | (B_transpose << 9) | (A_transpose << 8) | ((act) << 3) | ((dataflow) << 2) | CONFIG_EX, ((uint64_t)(relu6_shift) << 32) | (sys_shift), k_CONFIG); \
    \
    ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(weight_triple_bank) << 59) | ((uint64_t)(weight_double_bank) << 58) | ((uint64_t)(row_left) << 54) | ((uint64_t)(row_turn) << 42) | CONFIG_IM2COL, ((uint64_t)ocol << 56) | ((uint64_t)kdim2 << 48) | ((uint64_t)kdim << 44) | ((uint64_t)channel << 23) | ((uint64_t)stride << 20), k_CONFIG) \
  }

#define gemmini_extended_config_ex(dataflow, act, sys_shift, acc_scale, relu6_shift, A_stride, A_transpose, B_transpose) \
  gemmini_extended2_config_ex(dataflow, act, sys_shift, acc_scale, relu6_shift, A_stride, A_transpose, B_transpose, 0, 0, 0, 0, 0, 0, 0, 0, 0)

#define gemmini_config_ex(dataflow, act, sys_shift, acc_scale, relu6_shift) \
    gemmini_extended_config_ex(dataflow, act, sys_shift, acc_scale, relu6_shift, 1, 0, 0)

#if defined(HAS_MVIN_SCALE) || defined(HAS_MVIN_ACC_SCALE)
#define gemmini_extended3_config_ld(stride, scale, shrunk, id) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(scale_t_to_scale_t_bits(scale)) << 32) | ((id) << 3) | ((shrunk) << 2) | CONFIG_LD, stride, k_CONFIG)
#else
#define gemmini_extended2_config_ld(stride, scale, shrunk, id) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((id) << 3) | ((shrunk) << 2) | CONFIG_LD, stride, k_CONFIG)
#endif

#define gemmini_extended2_config_ld(stride, scale, shrunk) \
  gemmini_extended3_config_ld(stride, scale, shrunk, 0)

#define gemmini_extended_config_ld(stride, scale) \
  gemmini_extended2_config_ld(stride, scale, false)

#define gemmini_config_ld(stride) \
  gemmini_extended_config_ld(stride, MVIN_SCALE_IDENTITY)

#define gemmini_extended_config_st(stride, pool_stride, pool_size, pool_out_dim, porows, pocols, orows, ocols, upad, lpad) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, ((uint64_t)(ocols) << 56) | ((uint64_t)(orows) << 48) | ((uint64_t)(pocols) << 40) | ((uint64_t)(porows) << 32) | ((uint64_t)(pool_out_dim) << 24) | ((uint64_t)(lpad) << 10) | ((uint64_t)(upad) << 8) | ((uint64_t)(pool_size) << 6) | ((uint64_t)(pool_stride) << 4) | CONFIG_ST, stride, k_CONFIG)

#define gemmini_config_st(stride) \
    gemmini_extended_config_st(stride, 0, 0, 0, 0, 0, 0, 0, 0, 0)

// flush
#define gemmini_flush(skip) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, skip, 0, k_FLUSH)

// fence
#define gemmini_fence() asm volatile("fence")

//============================================================================
// gemmini-cisc opcodes
//============================================================================
#define gemmini_config_cisc_ex(act, sys_shift, acc_shift, relu6_shift) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, \
      ((uint64_t)(acc_shift) << 32) | \
      ((act) << 3) | \
      CONFIG_EX, \
    ((uint64_t)(relu6_shift) << 32) | \
      (sys_shift), \
    k_CONFIG_CISC_EX)

#define gemmini_config_addr_ab(A, B) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, A, B, k_ADDR_AB)

#define gemmini_config_addr_cd(C, D) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, C, D, k_ADDR_CD)

#define gemmini_config_size0(M, N) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, M, N, k_SIZE0)

#define gemmini_config_size1(K) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, K, 0, k_SIZE1)

#define gemmini_config_repeating_bias(repeating_bias) \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, repeating_bias, 0, k_RPT_BIAS)

#define gemmini_compute_cisc() \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, 0, 0, k_COMPUTE_CISC)

#define gemmini_config_reset() \
  ROCC_INSTRUCTION_RS1_RS2(XCUSTOM_ACC, 0, 0, k_RESET)

//============================================================================

// Tiling functions
static void sp_tiled_matmul_os(const elem_t * A, const elem_t * B, const void * D, void * C,
        scale_t A_scale_factor, scale_t B_scale_factor, scale_acc_t D_scale_factor,
        size_t I, size_t J, size_t K, size_t pad_I, size_t pad_J, size_t pad_K,
        size_t A_row_stride, size_t B_row_stride, size_t D_row_stride, size_t C_row_stride,
        bool a_transpose, bool b_transpose,
        bool full_C, bool low_D,
        bool no_bias, bool repeating_bias) {

  const uint32_t A_sp_addr_start = 0;
  const uint32_t B_sp_addr_start = BANK_NUM * BANK_ROWS - K * J * DIM;
  const uint32_t D_sp_addr_start = 1 << (ADDR_LEN-1);
  const uint32_t C_sp_addr_start = (3 << (ADDR_LEN-2)) | (full_C << (ADDR_LEN-3));

  const int A_blocks = K <= MAX_BLOCK_LEN ? K : MAX_BLOCK_LEN;
  const int B_blocks = J <= MAX_BLOCK_LEN ? J : MAX_BLOCK_LEN;
  const int D_blocks = J <= MAX_BLOCK_LEN_ACC ? J : MAX_BLOCK_LEN_ACC;

  // Move-in D
  if (D != NULL && !no_bias) {
    const size_t D_stride = repeating_bias ? 0 : D_row_stride * sizeof(acc_t);
    gemmini_extended_config_ld(D_stride, D_scale_factor);

    for (size_t i = 0; i < I; i++) {
      for (size_t j = 0; j < J; j += D_blocks) {
        const size_t bias_row = repeating_bias ? 0 : i;
        const acc_t * const D_dram_addr = (acc_t *)D + (bias_row * D_row_stride + j)*DIM;

        const uint32_t D_sp_addr_acc = D_sp_addr_start + (i*J + j)*DIM;

        const size_t blocks = j + D_blocks <= J ? D_blocks : J-j;

        const size_t cols = blocks * DIM - (j + blocks >= J ? pad_J : 0);
        const size_t rows = DIM - (i == I-1 ? pad_I : 0);

        gemmini_extended_mvin(D_dram_addr, D_sp_addr_acc, cols, rows);
      }
    }
  }

  // Move-in B
  gemmini_extended_config_ld(B_row_stride * sizeof(elem_t), B_scale_factor);
  for (size_t j = 0; j < J; j += B_blocks) {
    for (size_t k = 0; k < K; k++) {
      const elem_t * const B_dram_addr = B + (k*B_row_stride + j)*DIM;
      const uint32_t B_sp_addr = B_sp_addr_start + (k*J + j)*DIM;
      const size_t blocks = j + B_blocks <= J ? B_blocks : J-j;
      const size_t cols = blocks * DIM - (j + blocks >= J ? pad_J : 0);
      const size_t rows = DIM - (k == K-1 ? pad_K : 0);
      gemmini_extended_mvin(B_dram_addr, B_sp_addr, cols, rows);
    }
  }

  // Move-in A
  gemmini_extended_config_ld(A_row_stride * sizeof(elem_t), A_scale_factor);
  for (size_t i = 0; i < I; i++) {
    for (size_t k = 0; k < K; k += A_blocks) {
      const elem_t * const A_dram_addr = A + (i*A_row_stride + k)*DIM;
      const uint32_t A_sp_addr = A_sp_addr_start + (i*K + k)*DIM;
      const size_t blocks = k + A_blocks <= K ? A_blocks : K-k;
      const size_t cols = blocks * DIM - (k + blocks >= K ? pad_K : 0);
      const size_t rows = DIM - (i == I-1 ? pad_I : 0);
      gemmini_extended_mvin(A_dram_addr, A_sp_addr, cols, rows);
    }
  }

  for (size_t i = 0; i < I; i++) {
    for (size_t j = 0; j < J; j++) {
      const uint32_t C_sp_addr = C_sp_addr_start + (i*J + j)*DIM;

      for (size_t k = 0; k < K; k++) {

        const uint32_t A_sp_addr = A_sp_addr_start + (i*K + k)*DIM;
        const uint32_t B_sp_addr = B_sp_addr_start + (k*J + j)*DIM;

        uint32_t out_sp_addr = k == K-1 ? C_sp_addr : GARBAGE_ADDR;

        // If we're not using a bias, then we want to overwrite what's in the
        // accumulator, rather than writing over it
        int no_bias_new_matrix = no_bias && D != NULL && k == K-1;
        if (no_bias_new_matrix) {
          out_sp_addr &= ~(1 << (ADDR_LEN-2));
        }

        const size_t A_cols = DIM - (k == K - 1 ? pad_K : 0);
        const size_t A_rows = DIM - (i == I - 1 ? pad_I : 0);
        const size_t B_cols = DIM - (j == J - 1 ? pad_J : 0);
        const size_t B_rows = DIM - (k == K - 1 ? pad_K : 0);
        const size_t C_cols = DIM - (j == J - 1 ? pad_J : 0);
        const size_t C_rows = DIM - (i == I - 1 ? pad_I : 0);

        gemmini_extended_preload(GARBAGE_ADDR, out_sp_addr, DIM, DIM, C_cols, C_rows);

        if (k == 0) { // First iteration
          gemmini_extended_compute_preloaded(A_sp_addr, B_sp_addr, A_cols, A_rows, B_cols, B_rows);
        } else { // All other iterations
          gemmini_extended_compute_accumulated(A_sp_addr, B_sp_addr, A_cols, A_rows, B_cols, B_rows);
        }
      }
    }
  }

  // Move-out C
  if (C != NULL) {
    const size_t sizeof_C = full_C ? sizeof(acc_t) : sizeof(elem_t);

    for (size_t i = 0; i < I; i++) {
      for (size_t j = 0; j < J; j++) {
        void * const C_dram_addr = (int8_t*)C + (i*C_row_stride + j)*DIM*sizeof_C;
        const uint32_t C_sp_addr = C_sp_addr_start + (i*J + j)*DIM;

        const size_t C_cols = DIM - (j == J - 1 ? pad_J : 0);
        const size_t C_rows = DIM - (i == I - 1 ? pad_I : 0);

        gemmini_extended_mvout(C_dram_addr, C_sp_addr, C_cols, C_rows);
      }
    }
  }
}

static void sp_tiled_matmul_ws(const elem_t * A, const elem_t * B,
        const void * D, void * C,
        scale_t A_scale_factor, scale_t B_scale_factor, scale_acc_t D_scale_factor,
        size_t I, size_t J, size_t K, size_t pad_I, size_t pad_J, size_t pad_K,
        size_t A_row_stride, size_t B_row_stride, size_t D_row_stride, size_t C_row_stride,
        bool a_transpose, bool b_transpose,
        bool full_C, bool low_D,
        bool no_bias, bool repeating_bias) {

  /*
  const uint32_t A_sp_addr_start = 0;
  const uint32_t B_sp_addr_start = BANK_NUM * BANK_ROWS - K * J * DIM;
  const uint32_t D_sp_addr_start = 1 << (ADDR_LEN-1);
  const uint32_t C_sp_addr_start = 3 << (ADDR_LEN-2) | (full_C << (ADDR_LEN-3));

  const int A_blocks = a_transpose ? (I <= MAX_BLOCK_LEN ? I : MAX_BLOCK_LEN) :
    (K <= MAX_BLOCK_LEN ? K : MAX_BLOCK_LEN);
  const int B_blocks = b_transpose ? (K <= MAX_BLOCK_LEN ? K : MAX_BLOCK_LEN) :
    (J <= MAX_BLOCK_LEN ? J : MAX_BLOCK_LEN);
  const int D_blocks = low_D ? (J <= MAX_BLOCK_LEN ? J : MAX_BLOCK_LEN) :
    (J <= MAX_BLOCK_LEN_ACC ? J : MAX_BLOCK_LEN_ACC);

  const size_t sizeof_D = low_D ? sizeof(elem_t) : sizeof(acc_t);
  const size_t sizeof_C = full_C ? sizeof(acc_t) : sizeof(elem_t);

  // Move-in D
  if (D != NULL && !no_bias) {
    for (size_t i = 0; i < I; i++) {
      const size_t rows = DIM - (i == I-1 ? pad_I : 0);
      for (size_t j = 0; j < J; j += D_blocks) {
        const size_t bias_row = repeating_bias ? 0 : i;
        const void * const D_dram_addr = (int8_t *)D + (bias_row * D_row_stride + j)*DIM*sizeof_D;
        const uint32_t D_sp_addr_acc = D_sp_addr_start + (i*J + j)*DIM;
        size_t blocks = j + D_blocks <= J ? D_blocks : J-j;
        const size_t cols = blocks * DIM - (j + blocks >= J ? pad_J : 0);
        gemmini_extended_mvin3(D_dram_addr, D_sp_addr_acc, cols, rows);
      }
    }
  }

  for (size_t j = 0; j < J; j++) {
    for (size_t k = 0; k < K; k++) {
      for (size_t i = 0; i < I; i++) {
        const uint32_t A_sp_addr = a_transpose ? (A_sp_addr_start + (k*I + i)*DIM) :
          (A_sp_addr_start + (i*K + k)*DIM);
        const uint32_t B_sp_addr = b_transpose ? (B_sp_addr_start + (j*K + k)*DIM) :
          (B_sp_addr_start + (k*J + j)*DIM);
        const uint32_t C_sp_addr = C_sp_addr_start + (i*J + j)*DIM;

        // Mvin A
        if (a_transpose) {
          if (j == 0 && i % A_blocks == 0) {
            const elem_t * const A_dram_addr = A + (k*A_row_stride + i)*DIM;
            const size_t blocks = i + A_blocks <= I ? A_blocks : I-i;
            const size_t cols = blocks * DIM - (i + blocks >= I ? pad_I : 0);
            const size_t rows = DIM - (k == K-1 ? pad_K : 0);
            gemmini_extended_mvin(A_dram_addr, A_sp_addr, cols, rows);
          }
        } else {
          if (j == 0 && k % A_blocks == 0) {
            const elem_t * const A_dram_addr = A + (i*A_row_stride + k)*DIM;
            const size_t blocks = k + A_blocks <= K ? A_blocks : K-k;
            const size_t cols = blocks * DIM - (k + blocks >= K ? pad_K : 0);
            const size_t rows = DIM - (i == I-1 ? pad_I : 0);
            gemmini_extended_mvin(A_dram_addr, A_sp_addr, cols, rows);
          }
        }

        // Mvin B
        if (b_transpose) {
          if (i == 0 && k % B_blocks == 0) {
            const elem_t * const B_dram_addr = B + (j*B_row_stride + k)*DIM;
            const size_t blocks = k + B_blocks <= K ? B_blocks : K-k;
            const size_t cols = blocks * DIM - (k + blocks >= K ? pad_K : 0);
            const size_t rows = DIM - (j == J-1 ? pad_J : 0);
            gemmini_extended_mvin2(B_dram_addr, B_sp_addr, cols, rows);
          }
        } else {
          if (i == 0 && j % B_blocks == 0) {
            const elem_t * const B_dram_addr = B + (k*B_row_stride + j)*DIM;
            const size_t blocks = j + B_blocks <= J ? B_blocks : J-j;
            const size_t cols = blocks * DIM - (j + blocks >= J ? pad_J : 0);
            const size_t rows = DIM - (k == K-1 ? pad_K : 0);
            gemmini_extended_mvin2(B_dram_addr, B_sp_addr, cols, rows);
          }
        }

        // Compute
        {
          uint32_t pre_sp_addr = i == 0 ? B_sp_addr : GARBAGE_ADDR;
          uint32_t out_sp_addr = C_sp_addr;

          // If we're not using a bias, then we want to overwrite what's in the
          // accumulator, rather than writing over it
          int no_bias_new_matrix = no_bias && D != NULL && k == 0;
          if (no_bias_new_matrix) {
            out_sp_addr &= ~(1 << (ADDR_LEN-2));
          }

          const size_t A_cols = DIM - (k == K - 1 ? pad_K : 0);
          const size_t A_rows = DIM - (i == I - 1 ? pad_I : 0);
          const size_t B_cols = DIM - (j == J - 1 ? pad_J : 0);
          const size_t B_rows = DIM - (k == K - 1 ? pad_K : 0);
          const size_t C_cols = DIM - (j == J - 1 ? pad_J : 0);
          const size_t C_rows = DIM - (i == I - 1 ? pad_I : 0);

          gemmini_extended_preload(pre_sp_addr, out_sp_addr, B_cols, B_rows, C_cols, C_rows);

          if (i == 0) { // First iteration
            gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, A_cols, A_rows, DIM, DIM);
          } else { // All other iterations
            gemmini_extended_compute_accumulated(A_sp_addr, GARBAGE_ADDR, A_cols, A_rows, DIM, DIM);
          }
        }

        // Move-out C
        if (C != NULL && k == K-1) {
          void * const C_dram_addr = (int8_t*)C + (i*C_row_stride + j)*DIM*sizeof_C;

          const size_t C_cols = DIM - (j == J - 1 ? pad_J : 0);
          const size_t C_rows = DIM - (i == I - 1 ? pad_I : 0);

          gemmini_extended_mvout(C_dram_addr, C_sp_addr, C_cols, C_rows);
        }
      }
    }
  }
  */

  // Combined loop
  gemmini_loop_ws(I, J, K, pad_I, pad_J, pad_K, A, B, no_bias ? NULL : D, C,
    A_row_stride, B_row_stride, repeating_bias ? 0 : D_row_stride, C_row_stride,
    a_transpose, b_transpose,
    full_C, low_D, !no_bias || D == NULL);
}

static void tiled_matmul_outer(size_t dim_I, size_t dim_J, size_t dim_K,
        const elem_t* A, const elem_t* B,
        const void * D, void * C,
        size_t stride_A, size_t stride_B, size_t stride_D, size_t stride_C,
        scale_t A_scale_factor, scale_t B_scale_factor, scale_acc_t D_scale_factor,
        size_t tile_I, size_t tile_J, size_t tile_K,
        int act, acc_scale_t scale, size_t relu6_shift, bool repeating_bias,
        bool a_transpose, bool b_transpose,
        bool full_C, bool low_D,
        int dataflow) {

  const size_t dim_I_padded = (dim_I / DIM + (dim_I % DIM != 0)) * DIM;
  const size_t dim_J_padded = (dim_J / DIM + (dim_J % DIM != 0)) * DIM;
  const size_t dim_K_padded = (dim_K / DIM + (dim_K % DIM != 0)) * DIM;

  const size_t I0 = dim_I_padded / (tile_I*DIM) + (dim_I_padded % (tile_I*DIM) != 0);
  const size_t J0 = dim_J_padded / (tile_J*DIM) + (dim_J_padded % (tile_J*DIM) != 0);
  const size_t K0 = dim_K_padded / (tile_K*DIM) + (dim_K_padded % (tile_K*DIM) != 0);

  // These lines here are supposed to help us deal with when the dimensions of
  // the systolic array aren't divisible by the tiling factors
  const size_t last_I = dim_I_padded % (tile_I*DIM) == 0 ? tile_I : (dim_I_padded/DIM) % tile_I;
  const size_t last_J = dim_J_padded % (tile_J*DIM) == 0 ? tile_J : (dim_J_padded/DIM) % tile_J;
  const size_t last_K = dim_K_padded % (tile_K*DIM) == 0 ? tile_K : (dim_K_padded/DIM) % tile_K;

  // These lines are supposed to figure out how much padding the hardware is
  // supposed to add for the final tile
  const size_t padding_I = dim_I_padded - dim_I;
  const size_t padding_J = dim_J_padded - dim_J;
  const size_t padding_K = dim_K_padded - dim_K;

  const bool no_bias = D == NULL;

  if (no_bias) {
    D = (void*) 1; // Dummy address which isn't NULL
  }

  const size_t sizeof_D = low_D ? sizeof(elem_t) : sizeof(acc_t) ;
  const size_t sizeof_C = full_C ? sizeof(acc_t) : sizeof(elem_t);

  gemmini_extended_config_ex(dataflow, act, 0, scale, relu6_shift, 1, a_transpose, b_transpose);
  gemmini_config_st(stride_C * sizeof_C);
  gemmini_extended3_config_ld(stride_A * sizeof(elem_t), A_scale_factor, false, 0);
  gemmini_extended3_config_ld(stride_B * sizeof(elem_t), B_scale_factor, false, 1)
  gemmini_extended3_config_ld(repeating_bias ? 0 : (stride_D * sizeof_D), D_scale_factor, low_D, 2);

  void (*inner)(const elem_t *, const elem_t *, const void *, void *,
        scale_t, scale_t, scale_acc_t,
        size_t, size_t, size_t, size_t, size_t, size_t,
        size_t, size_t, size_t, size_t,
        bool, bool,
        bool, bool,
        bool, bool);

  if (dataflow == OUTPUT_STATIONARY) {
    inner = &sp_tiled_matmul_os;
  } else /* if (dataflow == WEIGHT_STATIONARY) */ {
    inner = &sp_tiled_matmul_ws;
  }

  for (size_t i0 = 0; i0 < I0; i0++)
    for (size_t j0 = 0; j0 < J0; j0++)
      for (size_t k0 = 0; k0 < K0; k0++) {

        const void * pre;
        if (k0 != 0) {
          pre = NULL;
        } else {
          size_t bias_row = repeating_bias ? 0 : i0*tile_I*DIM;
          // pre = &(((acc_t*)D)[bias_row * stride_D + j0 * tile_J * DIM]);
          pre = (int8_t*)D + (bias_row * stride_D + j0 * tile_J * DIM)*sizeof_D;
        }

        void * out = k0 == K0-1 ? (int8_t*)C + (i0*tile_I*DIM*stride_C + j0*tile_J*DIM)*sizeof_C : NULL;

        const size_t I = i0 < I0-1 ? tile_I : last_I;
        const size_t J = j0 < J0-1 ? tile_J : last_J;
        const size_t K = k0 < K0-1 ? tile_K : last_K;

        const size_t pad_I = i0 == I0-1 ? padding_I : 0;
        const size_t pad_J = j0 == J0-1 ? padding_J : 0;
        const size_t pad_K = k0 == K0-1 ? padding_K : 0;

        const elem_t * a = a_transpose ? (A + k0*tile_K*DIM*stride_A + i0*tile_I*DIM)
          : (A + i0*tile_I*DIM*stride_A + k0*tile_K*DIM);

        const elem_t * b = b_transpose ? (B + j0*tile_J*DIM*stride_B + k0*tile_K*DIM)
          : (B + k0*tile_K*DIM*stride_B + j0*tile_J*DIM);

        (*inner)(a, b, pre, out,
            A_scale_factor, B_scale_factor, D_scale_factor,
            I, J, K,
            pad_I, pad_J, pad_K,
            stride_A, stride_B, stride_D, stride_C,
            a_transpose, b_transpose,
            full_C, low_D,
            no_bias, repeating_bias);
      }

  gemmini_fence();
}

static elem_t scale_and_sat(acc_t x, int act, acc_scale_t scale, size_t relu6_shift) {
  // Scale value down and round it
  x = ACC_SCALE(x, scale);
  // Clip result
  x = x > elem_t_max ? elem_t_max : (x < elem_t_min ? elem_t_min : x);
  // Apply activation function
  if (act == RELU) {
    x = x < 0 ? 0 : x;
  }
  // TODO add another define to check if relu6_shift is actually used or not
  else if (act == RELU6) {
    int max = 6 << relu6_shift;
    x = x < 0 ? 0 : (x > max ? max : x);
  }
  return x;
}

#ifdef HAS_MVIN_SCALE
#define GEMMINI_SCALE(x, scale) MVIN_SCALE((x), (scale))
#else
#define GEMMINI_SCALE(x, scale) (x)
#endif

static void matmul_cpu(size_t DIM_I, size_t DIM_J, size_t DIM_K,
        const elem_t* A, const elem_t* B, const acc_t * D,
        elem_t* C,
        size_t stride_A, size_t stride_B, size_t stride_D, size_t stride_C,
        scale_t A_scale_factor, scale_t B_scale_factor, scale_acc_t D_scale_factor,
        int act, acc_scale_t scale, size_t relu6_shift, bool repeating_bias) {

  const int no_bias = D == NULL;
  if (DIM_I % 4 == 0 && DIM_J % 4 == 0) {
    for (size_t i = 0; i < DIM_I; i += 4) {
      for (size_t j = 0; j < DIM_J; j += 4) {

        acc_t result[4][4]; // = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

        for (size_t ii = 0; ii < 4; ii++)
          for (size_t jj = 0; jj < 4; jj++) {
            const size_t bias_row = repeating_bias ? 0 : i + ii;
            result[ii][jj] = no_bias ? 0 :
              GEMMINI_SCALE(*(D + bias_row*stride_D + j + jj), D_scale_factor);
          }

        for (size_t k = 0; k < DIM_K; k++) {
          result[0][0] +=
                GEMMINI_SCALE(*(A + i*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j), B_scale_factor);
          result[0][1] +=
                GEMMINI_SCALE(*(A + i*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+1), B_scale_factor);
          result[0][2] +=
                GEMMINI_SCALE(*(A + i*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+2), B_scale_factor);
          result[0][3] +=
                GEMMINI_SCALE(*(A + i*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+3), B_scale_factor);
          result[1][0] +=
                GEMMINI_SCALE(*(A + (i+1)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j), B_scale_factor);
          result[1][1] +=
                GEMMINI_SCALE(*(A + (i+1)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+1), B_scale_factor);
          result[1][2] +=
                GEMMINI_SCALE(*(A + (i+1)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+2), B_scale_factor);
          result[1][3] +=
                GEMMINI_SCALE(*(A + (i+1)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+3), B_scale_factor);
          result[2][0] +=
                GEMMINI_SCALE(*(A + (i+2)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j), B_scale_factor);
          result[2][1] +=
                GEMMINI_SCALE(*(A + (i+2)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+1), B_scale_factor);
          result[2][2] +=
                GEMMINI_SCALE(*(A + (i+2)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+2), B_scale_factor);
          result[2][3] +=
                GEMMINI_SCALE(*(A + (i+2)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+3), B_scale_factor);
          result[3][0] +=
                GEMMINI_SCALE(*(A + (i+3)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j), B_scale_factor);
          result[3][1] +=
                GEMMINI_SCALE(*(A + (i+3)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+1), B_scale_factor);
          result[3][2] +=
                GEMMINI_SCALE(*(A + (i+3)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+2), B_scale_factor);
          result[3][3] +=
                GEMMINI_SCALE(*(A + (i+3)*stride_A + k), A_scale_factor) *
                GEMMINI_SCALE(*(B + k*stride_B + j+3), B_scale_factor);
        }

        *(C + i*stride_C + j) =
             scale_and_sat(result[0][0], act, scale, relu6_shift);
        *(C + i*stride_C + j+1) =
             scale_and_sat(result[0][1], act, scale, relu6_shift);
        *(C + i*stride_C + j+2) =
             scale_and_sat(result[0][2], act, scale, relu6_shift);
        *(C + i*stride_C + j+3) =
             scale_and_sat(result[0][3], act, scale, relu6_shift);
        *(C + (i+1)*stride_C + j) =
             scale_and_sat(result[1][0], act, scale, relu6_shift);
        *(C + (i+1)*stride_C + j+1) =
             scale_and_sat(result[1][1], act, scale, relu6_shift);
        *(C + (i+1)*stride_C + j+2) =
             scale_and_sat(result[1][2], act, scale, relu6_shift);
        *(C + (i+1)*stride_C + j+3) =
             scale_and_sat(result[1][3], act, scale, relu6_shift);
        *(C + (i+2)*stride_C + j) =
             scale_and_sat(result[2][0], act, scale, relu6_shift);
        *(C + (i+2)*stride_C + j+1) =
             scale_and_sat(result[2][1], act, scale, relu6_shift);
        *(C + (i+2)*stride_C + j+2) =
             scale_and_sat(result[2][2], act, scale, relu6_shift);
        *(C + (i+2)*stride_C + j+3) =
             scale_and_sat(result[2][3], act, scale, relu6_shift);
        *(C + (i+3)*stride_C + j) =
             scale_and_sat(result[3][0], act, scale, relu6_shift);
        *(C + (i+3)*stride_C + j+1) =
             scale_and_sat(result[3][1], act, scale, relu6_shift);
        *(C + (i+3)*stride_C + j+2) =
             scale_and_sat(result[3][2], act, scale, relu6_shift);
        *(C + (i+3)*stride_C + j+3) =
             scale_and_sat(result[3][3], act, scale, relu6_shift);
      }
    }
  } else {
    for (size_t i = 0; i < DIM_I; i++) {
      for (size_t j = 0; j < DIM_J; j++) {

        const size_t bias_row = repeating_bias ? 0 : i;

        acc_t result = no_bias ? 0 : GEMMINI_SCALE(*(D + bias_row * stride_D + j), D_scale_factor);

        for (size_t k = 0; k < DIM_K; k++) {
          //acc_t past_opixel = result;
          result += GEMMINI_SCALE(*(A + i*stride_A + k), A_scale_factor) * GEMMINI_SCALE(*((elem_t*)B + k*stride_B + j), B_scale_factor);
        }

        *(C + i*stride_C + j) = scale_and_sat(result, act, scale, relu6_shift);
      }
    }
  }
}

#undef GEMMINI_SCALE

// General matmul which can be run with different dataflows, or on the CPU
enum tiled_matmul_type_t {OS, WS, CPU}; // TODO rename this so it's name also applies to convs

// This function runs a tiled matrix multiplication, with hardcoded tiling
// factors
void tiled_matmul(size_t dim_I, size_t dim_J, size_t dim_K,
        const elem_t* A, const elem_t* B,
        const void * D, void* C,
        size_t stride_A, size_t stride_B, size_t stride_D, size_t stride_C,
        scale_t A_scale_factor, scale_t B_scale_factor, scale_acc_t D_scale_factor,
        int act, acc_scale_t scale, size_t relu6_shift, bool repeating_bias,
        size_t tile_I, size_t tile_J, size_t tile_K,
        bool transpose_A, bool transpose_B,
        bool full_C, bool low_D,
        enum tiled_matmul_type_t tiled_matmul_type) {

#ifdef GEMMINI_ASSERTIONS
  // Make sure that the tiling factors make sense
  if (tile_I <= 0) {
    printf("tile_I is non-positive\n");
    exit(1);
  } else if (tile_J <= 0) {
    printf("tile_J is non-positive\n");
    exit(1);
  } else if (tile_K <= 0) {
    printf("tile_K is non-positive\n");
    exit(1);
  }

  const size_t dim_I_padded = (dim_I / DIM + (dim_I % DIM != 0)) * DIM;
  const size_t dim_J_padded = (dim_J / DIM + (dim_J % DIM != 0)) * DIM;
  const size_t dim_K_padded = (dim_K / DIM + (dim_K % DIM != 0)) * DIM;

  if (tile_I * DIM > dim_I_padded) {
    printf("tile_I is too large (tile_I * DIM > dim_I_padded)\n");
    exit(1);
  } else if (tile_J * DIM > dim_J_padded) {
    printf("tile_J is too large (tile_J * DIM > dim_J_padded)\n");
    exit(1);
  } else if (tile_K * DIM > dim_K_padded) {
    printf("tile_K is too large (tile_K * DIM > dim_K_padded)\n");
    exit(1);
  }

  const bool double_buffered = tiled_matmul_type == WS;

  const size_t total_spad_size = double_buffered ? BANK_NUM * BANK_ROWS / 2 :
      BANK_NUM * BANK_ROWS;
  const size_t total_acc_size = double_buffered ? ACC_ROWS / 2 : ACC_ROWS;

  const size_t total_spad_rows =
      (tile_I * tile_K * DIM) +   // Rows to store A
      (tile_K * tile_J * DIM);    // Rows to store B

  if (total_spad_rows > total_spad_size) {
    printf("Not enough space in scratchpad to store A and B matrices\n");
    exit(1);
  }

  const size_t total_acc_rows =
      tile_I * tile_J * DIM;      // Rows to store C

  if (total_acc_rows > total_acc_size) {
    printf("Not enough space in accumulator to store C\n");
    exit(1);
  }

  if (tile_I > 65535 || tile_J > 65535 || tile_K > 65535) {
    printf("I, J, and K tiling factors must be less than 65535, to fit within the bounds of the LOOP_WS function");
    exit(1);
  }

  char matmul_type_str[][3] = {"OS", "WS", "CPU"};

  // Check if transpose options are correct
  if (((tiled_matmul_type == OS || tiled_matmul_type == CPU) && (transpose_A || transpose_B)) ||
    (tiled_matmul_type == WS && transpose_A && transpose_B)) {
    printf("Not implemented: %s matmul, a_transpose=%d, b_transpose=%d\n", matmul_type_str[tiled_matmul_type], transpose_A, transpose_B);
    exit(1);
  }

  // Check if full_C options are correct
  if ((tiled_matmul_type == CPU && (full_C || low_D)) ||
      (tiled_matmul_type == OS && low_D)) {
    printf("Not implemented: %s matmul, full_C=%d, low_D=%d\n", matmul_type_str[tiled_matmul_type], full_C, low_D);
  }
#endif

  // Run a tiled matrix multiplication on either Gemmini or the CPU
  if (tiled_matmul_type == OS || tiled_matmul_type == WS) {
    tiled_matmul_outer(dim_I, dim_J, dim_K,
        A, B, D, C,
        stride_A, stride_B, stride_D, stride_C,
        A_scale_factor, B_scale_factor, D_scale_factor,
        tile_I, tile_J, tile_K,
        act, scale, relu6_shift, repeating_bias,
        transpose_A, transpose_B,
        full_C, low_D,
        (int)tiled_matmul_type);
  } else /*if (tiled_matmul_type == CPU)*/ {
    matmul_cpu(dim_I, dim_J, dim_K,
            A, B, D, (elem_t*)C,
            stride_A, stride_B, stride_D, stride_C,
            A_scale_factor, B_scale_factor, D_scale_factor,
            act, scale, relu6_shift, repeating_bias);
  }
}

static size_t tiled_matmul_total_spad_rows(size_t I, size_t J, size_t K) {
  return (I * K + K * J) * DIM;
}

static size_t tiled_matmul_total_acc_rows(size_t I, size_t J) {
  return (I * J) * DIM;
}

// This function runs a tiled matrix multiplication, with automatically
// calculated tiling factors
void tiled_matmul_auto(size_t dim_I, size_t dim_J, size_t dim_K,
        const elem_t* A, const elem_t* B,
        const void * D, void * C,
        size_t stride_A, size_t stride_B, size_t stride_D, size_t stride_C,
        scale_t A_scale_factor, scale_t B_scale_factor, scale_acc_t D_scale_factor,
        int act, acc_scale_t scale, size_t relu6_shift, bool repeating_bias,
        bool transpose_A, bool transpose_B,
        bool full_C, bool low_D,
        enum tiled_matmul_type_t tiled_matmul_type) {

#define partition_rows (BANK_NUM * BANK_ROWS / 2)
#define mats_in_partition (partition_rows / DIM)
#define mats_in_acc (ACC_ROWS / DIM)
#define max_tile_i_j ((size_t)sqrt(mats_in_acc))
#define max_tile_k (mats_in_partition / max_tile_i_j)

    // "db_" means "double-buffered"
#define db_partition_rows ((BANK_NUM * BANK_ROWS / 2) / 2)
#define db_mats_in_partition (db_partition_rows / DIM)
#define db_mats_in_acc ((ACC_ROWS / 2) / DIM)
#define db_max_tile_i_j ((size_t)sqrt(db_mats_in_acc))
#define db_max_tile_k (db_mats_in_partition / db_max_tile_i_j)

    const size_t dim_I_padded = (dim_I / DIM + (dim_I % DIM != 0)) * DIM;
    const size_t dim_J_padded = (dim_J / DIM + (dim_J % DIM != 0)) * DIM;
    const size_t dim_K_padded = (dim_K / DIM + (dim_K % DIM != 0)) * DIM;

    const bool double_buffered = tiled_matmul_type == WS;

    const size_t max_spad_rows = double_buffered ? BANK_NUM * BANK_ROWS / 2 :
      BANK_NUM * BANK_ROWS;
    const size_t max_acc_rows = double_buffered ? ACC_ROWS / 2 : ACC_ROWS;

    size_t tile_I, tile_J, tile_K;

    if (double_buffered) {
       tile_I = dim_I_padded/DIM < db_max_tile_i_j ? dim_I_padded/DIM : db_max_tile_i_j;
       tile_J = dim_J_padded/DIM < db_max_tile_i_j ? dim_J_padded/DIM : db_max_tile_i_j;
       tile_K = dim_K_padded/DIM < db_max_tile_k ? dim_K_padded/DIM : db_max_tile_k;
    } else {
       tile_I = dim_I_padded/DIM < max_tile_i_j ? dim_I_padded/DIM : max_tile_i_j;
       tile_J = dim_J_padded/DIM < max_tile_i_j ? dim_J_padded/DIM : max_tile_i_j;
       tile_K = dim_K_padded/DIM < max_tile_k ? dim_K_padded/DIM : max_tile_k;
    }

    // Fill scratchpad as much as possible
    while (true) {
      bool increased = false;

      if (tiled_matmul_total_spad_rows(tile_I, tile_J+1, tile_K) <= max_spad_rows &&
          tiled_matmul_total_acc_rows(tile_I, tile_J+1) <= max_acc_rows &&
          (tile_J+1) * DIM <= dim_J_padded) {
        tile_J++;
        increased = true;
      }

      if (tiled_matmul_total_spad_rows(tile_I+1, tile_J, tile_K) <= max_spad_rows &&
          tiled_matmul_total_acc_rows(tile_I+1, tile_J) <= max_acc_rows &&
          (tile_I+1) * DIM <= dim_I_padded) {
        tile_I++;
        increased = true;
      }

      if (tiled_matmul_total_spad_rows(tile_I, tile_J, tile_K+1) <= max_spad_rows &&
          (tile_K+1) * DIM <= dim_K_padded) {
        tile_K++;
        increased = true;
      }

      if (!increased)
        break;
    }

    tiled_matmul(dim_I, dim_J, dim_K,
        A, B, D, C,
        stride_A, stride_B, stride_D, stride_C,
        A_scale_factor, B_scale_factor, D_scale_factor,
        act, scale, relu6_shift, repeating_bias,
        tile_I, tile_J, tile_K,
        transpose_A, transpose_B,
        full_C, low_D,
        tiled_matmul_type);

#undef partition_rows
#undef mats_in_partition
#undef mats_in_acc
#undef max_tile_i_j
#undef max_tile_k
}

static void tiled_matmul_auto_cisc(
  size_t M, size_t N, size_t K,
  const elem_t* A, const elem_t* B, const acc_t * D, elem_t* C,
  int act, size_t shift, size_t relu6_shift, bool repeating_bias)
{
  gemmini_config_reset();
  gemmini_config_cisc_ex(act, 0, shift, relu6_shift);
  gemmini_config_addr_ab((uintptr_t)A, (uintptr_t)B);
  gemmini_config_addr_cd((uintptr_t)C, (uintptr_t)D);
  gemmini_config_size0(M, N);
  gemmini_config_size1(K);
  gemmini_config_repeating_bias(repeating_bias);
  gemmini_compute_cisc();
  gemmini_fence();
}

void sp_tiled_conv(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim, int pool_out_dim,

        int stride, int padding, int kernel_dim,

        int pool_size, int pool_stride, int pool_padding,

        int batches,
        int porows, int pocols, int pochs,
        int krows, int kcols, int kchs,

        int lpad, int rpad, int upad, int dpad,
        int plpad, int prpad, int pupad, int pdpad,

        elem_t * input,
        elem_t * weights,
        elem_t * output,
        acc_t * bias,

        bool no_bias, bool no_pool) {

    // const bool no_padding = lpad == 0 && rpad == 0 && upad == 0 && dpad == 0;
    // printf("SP_TILED_CONV no_padding: %d", no_padding);

    const int orows = porows * pool_stride + pool_size - 1 - pupad - pdpad;
    const int ocols = pocols * pool_stride + pool_size - 1 - plpad - prpad;
    const int ochs = pochs;

    // Calculate image dimensions
    const int irows = orows * stride + krows - 1; // - 2 * padding;
    const int icols = ocols * stride + kcols - 1; // - 2 * padding;
    const int irows_unpadded = irows - upad - dpad;
    const int icols_unpadded = icols - lpad - rpad;
    const int ichs = kchs;

    int icols_per_systolic_row = ichs > DIM ? 1 : DIM / ichs;
    if (kcols < icols_per_systolic_row) {
        icols_per_systolic_row = kcols;
    }
    if (lpad != 0 || rpad != 0 || upad != 0 || dpad != 0) {
        icols_per_systolic_row = 1;
    }
    icols_per_systolic_row = 1; // TODO
    // printf("  icols_per_systolic_row %d\n\n", icols_per_systolic_row);

    // Calculate spad address offsets
    const int out_channels_per_bank = ochs / DIM + (ochs % DIM != 0);
    const int B_rows = out_channels_per_bank * kcols * krows * kchs;

    const uint32_t A_sp_addr_start = 0;
    const uint32_t B_sp_addr_start = BANK_NUM * BANK_ROWS - B_rows;
    const uint32_t D_sp_addr_start = 1 << (ADDR_LEN - 1);
    const uint32_t C_sp_addr_start = 3 << (ADDR_LEN - 2);

    // printf("mvin bias\n");
    // mvin bias
    if (!no_bias && bias != NULL) {
        // TODO we probably don't need quite this many nested loops for this part
        gemmini_config_ld(0);
        for (int b = 0; b < batches; b++)
            for (int orow = 0; orow < orows; orow++)
                for (int ocol = 0; ocol < ocols; ocol += DIM) {
                    const int I = ocols - ocol > DIM ? DIM : ocols - ocol;

                    for (int och = 0; och < ochs; och += DIM) {
                        const int J = ochs - och > DIM ? DIM : ochs - och;

                        const uint32_t D_sp_addr = D_sp_addr_start + (och / DIM) * batches * orows * ocols + b * orows * ocols + orow * ocols + ocol;

                        gemmini_extended_mvin(bias + och,
                                D_sp_addr,
                                J, I);
                    }
                }
    }

    // mvin input
    // printf("mvin inputs\n");
    gemmini_config_ld(in_channels * sizeof(elem_t));
    gemmini_fence(); // TODO fix ROB to get rid of this requirement
    for (int b = 0; b < batches; b++) {
        for (int irow = -upad; irow < irows_unpadded + dpad; irow++) {
            const int irow_padded = irow + upad;

            for (int icol = -lpad; icol < icols_unpadded + rpad;) {
                // TODO There might be some unnecessary mvins here at the edge of the image

                int icols_moved_in_per_row = icol + icols_per_systolic_row <= icols_unpadded ? icols_per_systolic_row : icols_unpadded - icol;

                int I = icols_unpadded - icol > DIM ? DIM : icols_unpadded - icol;

                if (icol < 0) {
                    I = -icol > DIM ? DIM : -icol;
                    icols_moved_in_per_row = 1;
                } else if (icol >= icols_unpadded) {
                    I = icols_unpadded + rpad - icol > DIM ? DIM : icols_unpadded + rpad - icol;
                    icols_moved_in_per_row = 1;
                }

                const int icol_padded = icol + lpad;

                for (int ich = 0; ich < ichs; ich += DIM) {
                    const int K = (ichs - ich > DIM ? DIM : ichs - ich) * icols_moved_in_per_row;
                    // printf("Mvin K: %d, ichs: %d, ich: %d, icols_moved_in_per_row: %d, icol: %d, icols_unpadded: %d\n", K, ichs, ich, icols_moved_in_per_row, icol, icols_unpadded);

                    elem_t * in = input + (b*in_dim*in_dim + irow*in_dim + icol) * in_channels + ich;

                    const bool is_zeros = irow < 0 || irow >= irows_unpadded || icol < 0 || icol >= icols_unpadded;
                    if (is_zeros) {
                        gemmini_config_ld(0);
                        static elem_t zeros[MAX_BYTES / sizeof(elem_t)] = {0};
                        in = &zeros[0];
                    }

                    const uint32_t A_sp_addr = A_sp_addr_start + (ich / DIM) * batches * irows * icols + b * irows * icols + irow_padded * icols + icol_padded;

                    gemmini_extended_mvin(in,
                            A_sp_addr,
                            K, I);

                    if (is_zeros) {
                        gemmini_config_ld(in_channels * sizeof(elem_t));
                    }
                }

                icol += I;
            }
        }
    }
    gemmini_fence(); // TODO fix ROB to get rid of this requirement

    // mvin weights
    // printf("mvin weights\n");
    gemmini_config_ld(out_channels * sizeof(elem_t));
    for (int och = 0; och < ochs; och += DIM) {
        const int J = ochs - och > DIM ? DIM : ochs - och;

        for (int krow = 0; krow < krows; krow++)
            for (int kcol = 0; kcol < kcols; kcol++)
                for (int kch = 0; kch < kchs; kch += DIM) {
                    const int K = kchs - kch > DIM ? DIM : kchs - kch;

                    const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * krows * kcols * kchs + krow * kcols * kchs + kcol * kchs + kch;

                    gemmini_extended_mvin(weights + (krow*kernel_dim*in_channels + kcol*in_channels + kch) * out_channels + och,
                        B_sp_addr,
                        J, K);
                }
    }

    // Compute
    // printf("compute\n");

    // if (icols_per_systolic_row == 1 || 
    for (int b = 0; b < batches; b++)
        for (int orow = 0; orow < orows; orow++)
            for (int ocol = 0; ocol < ocols; ocol += DIM) {
                const int I = ocols - ocol > DIM ? DIM : ocols - ocol;

                for (int och = 0; och < ochs; och += DIM) {
                    const int J = ochs - och > DIM ? DIM : ochs - och;

                    const int C_sp_addr = C_sp_addr_start + (och / DIM) * batches * orows * ocols + b * orows * ocols + orow * ocols + ocol;

                    for (int krow = 0; krow < krows; krow++) {
                        const int irow = orow * stride + krow;

                        // for (int kcol = 0; kcol < kcols; kcol++) {
                        for (int kcol = 0; kcol < kcols; kcol += icols_per_systolic_row) {
                            const int icol = ocol * stride + kcol;

                            for (int kch = 0; kch < kchs; kch += DIM) {
                                // Over here, construct a new matrix
                                //
                                // Let us assume that we only ever operate on
                                // one pixel in one row.
                                // Thus, krow == kcol == 1
                                //
                                // Then, for every set of I, J, and K values
                                //     - I = ocol
                                //     - J = och
                                //     - K = kch

                                const int icols_in_systolic_row = kcol + icols_per_systolic_row <= kcols ? icols_per_systolic_row : kcols - kcol;

                                const int K = (kchs - kch > DIM ? DIM : kchs - kch) * icols_in_systolic_row;

                                const uint32_t A_sp_addr = A_sp_addr_start + (kch / DIM) * batches * irows * icols + b * irows * icols + irow * icols + icol;
                                const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * krows * kcols * kchs + krow * kcols * kchs + kcol * kchs + kch;

                                // perform matmul
                                const uint32_t out_sp_addr =
                                    (bias != NULL && no_bias) && krow == 0 && kcol == 0 && kch == 0 ?
                                    C_sp_addr & ~((uint32_t)(1 << (ADDR_LEN - 2))) :
                                    C_sp_addr;

                                // printf("icols_in_systolic_row: %d\n", icols_in_systolic_row);
                                // printf("I: %d, J: %d, K: %d\n", I, J, K);
                                // printf("A_sp_addr: %u (%x), B_sp_addr: %u (%x), C_sp_addr: %u (%x)\n\n", A_sp_addr, A_sp_addr, B_sp_addr, B_sp_addr, out_sp_addr, out_sp_addr);

                                gemmini_extended_preload(B_sp_addr, out_sp_addr,
                                        J, K, J, I);
                                gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, K, I, J, I);
                            }
                        }
                    }
                }
            }

    // mvout output
    if (output != NULL) {
        if (no_pool) {
            for (int b = 0; b < batches; b++)
                for (int orow = 0; orow < orows; orow++)
                    for (int ocol = 0; ocol < ocols; ocol += DIM) {
                        const int I = ocols - ocol > DIM ? DIM : ocols - ocol;

                        for (int och = 0; och < ochs; och += DIM) {
                            const int J = ochs - och > DIM ? DIM : ochs - och;

                            const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * orows * ocols + b * orows * ocols + orow * ocols + ocol;

                            gemmini_extended_mvout(output + (b*out_dim*out_dim + orow*out_dim + ocol) * out_channels + och,
                                    C_sp_addr,
                                    J, I);
                        }
                    }
        } else {
            gemmini_extended_config_st(out_channels * sizeof(elem_t), pool_stride, pool_size, pool_out_dim, porows, pocols, orows, ocols, pupad, plpad);

            gemmini_fence(); // TODO remove this when the ROB can accurately handle these
            for (int b = 0; b < batches; b++) {
                for (int poch = 0; poch < pochs; poch += DIM) {
                    const int channels = poch + DIM >= pochs ? pochs - poch : DIM;

                    elem_t * pout = output + (b * pool_out_dim * pool_out_dim)*out_channels + poch;

                    const uint32_t C_sp_addr = C_sp_addr_start + (poch / DIM) * batches * orows * ocols + b * orows * ocols;

                    gemmini_extended_mvout(pout,
                            C_sp_addr,
                            channels, 0);
                }
            }
            gemmini_fence();
        }
    }
}

//resnet downsampling layer (no padding, kernel size 1, stride 2)
//due to poor instruction issue bandwidth
void sp_tiled_conv_ds(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim, int pool_out_dim,

        int stride, //int padding, int kernel_dim,
        int pool_size, int pool_stride, int pool_padding  __attribute__((unused)),

        int batches,
        int porows, int pocols, int pochs,
        int kchs,

        int lpad, int rpad, int upad, int dpad,
        int plpad, int prpad, int pupad, int pdpad,

        elem_t * input,
	    uint32_t B_sp_addr_outer,
        elem_t * weights,
        elem_t * output,
        acc_t * bias,

	    int act, acc_scale_t scale, int relu6_shift,
        bool no_bias, bool no_pool,
	    int weight_bank) {

    // const bool no_padding = lpad == 0 && rpad == 0 && upad == 0 && dpad == 0;
    // printf("SP_TILED_CONV no_padding: %d", no_padding);

    const int orows = porows * pool_stride + pool_size - 1 - pupad - pdpad;
    const int ocols = pocols * pool_stride + pool_size - 1 - plpad - prpad;
    const int ochs = pochs;

    // Calculate image dimensions
    const int irows = (orows - 1) * stride + 1;
    const int icols = (ocols - 1) * stride + 1;//kcols; 
    const int ichs = kchs;

	const int im2col_height = ocols*orows;
    // const int im2col_width = kchs;
	const int row_left = im2col_height%DIM;
	const int row_turn = row_left == 0 ? im2col_height/DIM - 1 : im2col_height/DIM;
	const int double_bank = weight_bank > 1 ? 1 : 0;
	const int triple_bank = weight_bank > 2 ? 1 : 0;
	  
    int odims = im2col_height;
 
	  gemmini_extended2_config_ex(WEIGHT_STATIONARY, act, 0, scale, relu6_shift, 1, false, false, ocols, row_turn, 1, stride, kchs, row_left, 1, double_bank, triple_bank); //if want 2 banks for weight, last is 1

    const uint32_t A_sp_addr_start = 0;
    const uint32_t B_sp_addr_start = B_sp_addr_outer == 0 ? (BANK_NUM - weight_bank) * BANK_ROWS : B_sp_addr_outer;
    const uint32_t D_sp_addr_start = 1 << (ADDR_LEN - 1);
    const uint32_t C_sp_addr_start = 3 << (ADDR_LEN - 2);

    // printf("mvin bias\n");
    // mvin bias

    if (!no_bias && bias != NULL) {
        // TODO we probably don't need quite this many nested loops for this part
        gemmini_config_ld(0);
        for (int b = 0; b < batches; b++)
          for (int och = 0; och < ochs; och += DIM) {
               //const int J = ochs - och > DIM ? DIM : ochs - och;
               const uint32_t D_sp_addr = D_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;
	        for(int odim = 0; odim < odims; odim += DIM){
 		    const int I = odims - odim > DIM ? DIM : odims - odim;
                        gemmini_extended_mvin(bias + och,
                                D_sp_addr+odim,
                                DIM, I);
                    }
                }
    }

   // mvin weights if it hasn't moved-in in outer loop
//    printf("weight move in \n");
   if(B_sp_addr_outer == 0){
    gemmini_config_ld(out_channels*sizeof(elem_t));
    for (int och = 0; och < ochs; och += DIM) {
        const int J = ochs - och > DIM ? DIM : ochs - och;
        const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kchs; 
        for (int kch = 0; kch < kchs; kch += DIM) {
           const int K = kchs - kch > DIM ? DIM : kchs - kch;
           gemmini_extended_mvin(weights + kch * out_channels + och,
                        B_sp_addr+kch,
                        J, K);
	}
    }
   }

//	gemmini_fence();
 int idims = irows*icols; 
int bidims = batches*idims;
    // mvin input
//     printf("mvin inputs\n");
    gemmini_config_ld(in_channels * sizeof(elem_t));

   for (int b = 0; b < batches; b++) {
        for (int irow = 0; irow < irows; irow++) {
                elem_t * in = input + (b*in_dim*in_dim + irow*in_dim) * in_channels;// + ich;
       		const uint32_t A_sp_addr = A_sp_addr_start + b * idims + irow * icols;
                   for (int ich = 0; ich < ichs; ich += DIM) {
                      // const int K = ichs - ich > DIM ? DIM : ichs - ich;
                       gemmini_extended_mvin(in+ich,
                            A_sp_addr + (ich/DIM)*bidims,
                            DIM, icols);
		}
       }
    }

  // Compute
  // previously attempted to merge with mvout
//   printf("compute  \n");
	//gemmini_fence();
	 if(odims > DIM){ //output dimension (row*col) bigger than DIM
	   for (int b = 0; b < batches; b++){
	        for (int och = 0; och < ochs; och += DIM) {
 		   const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kchs;// + kch;	 
     		   const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;

 		   for (int kch = 0; kch < kchs; kch += DIM) {
//                	gemmini_extended_mvin(weights + kch * out_channels + och,
//                        	B_sp_addr+kch,
//                        	DIM, DIM);
	
			const uint32_t A_sp_addr = A_sp_addr_start + (kch / DIM)*batches*idims + b*idims;           
            		for(int odim = 0; odim < odims; odim += DIM){ //both dimension at the same time
				const int I = odims - odim > DIM ? DIM : odims - odim;
                        	gemmini_extended_preload(B_sp_addr+kch, C_sp_addr+odim,
                                	 DIM, DIM, DIM, I);
                        	gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, DIM, I, DIM, I);
			}
            	  }
//	if(output!=NULL) gemmini_extended_mvout(output + (b * out_dim * out_dim)*out_channels + och, C_sp_addr, DIM, 0);
	
              }
     	  }
  	}else{//ds layer
	   for (int b = 0; b < batches; b++){
        	for (int och = 0; och < ochs; och += DIM) {
  	    		const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;
            		int kch_bound = 0;
  	    		for (int kch = 0; kch + 7*DIM < kchs; kch += 8*DIM) {
				const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kchs + kch;
/*
        			for(int kk = 0; kk < 8*DIM; kk += DIM){
                    			gemmini_extended_mvin(weights + (kk+kch) * out_channels + och,
                        			B_sp_addr+kk,
                        		DIM, DIM);
				}
*/
				const uint32_t A_sp_addr = A_sp_addr_start + (kch / DIM)*bidims + b*idims; 
                		gemmini_extended_preload(B_sp_addr, C_sp_addr,
                                        DIM, DIM, DIM, odims);
               		 	gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, DIM, odims, DIM, odims);

                		gemmini_extended_preload(B_sp_addr+DIM, C_sp_addr,
                                        DIM, DIM, DIM, odims);
                		gemmini_extended_compute_preloaded(A_sp_addr+bidims, GARBAGE_ADDR, DIM, odims, DIM, odims);

                		gemmini_extended_preload(B_sp_addr+2*DIM, C_sp_addr,
                                        DIM, DIM, DIM, odims);
                		gemmini_extended_compute_preloaded(A_sp_addr+2*bidims, GARBAGE_ADDR, DIM, odims, DIM, odims);

                		gemmini_extended_preload(B_sp_addr+3*DIM, C_sp_addr,
                                        DIM, DIM, DIM, odims);
                		gemmini_extended_compute_preloaded(A_sp_addr+3*bidims, GARBAGE_ADDR, DIM, odims, DIM, odims);

                		gemmini_extended_preload(B_sp_addr + 4*DIM, C_sp_addr,
                                        DIM, DIM, DIM, odims);
                		gemmini_extended_compute_preloaded(A_sp_addr + 4*bidims, GARBAGE_ADDR, DIM, odims, DIM, odims);

                		gemmini_extended_preload(B_sp_addr + 5*DIM, C_sp_addr,
                                        DIM, DIM, DIM, odims);
                		gemmini_extended_compute_preloaded(A_sp_addr+5*bidims, GARBAGE_ADDR, DIM, odims, DIM, odims);

				gemmini_extended_preload(B_sp_addr + 6*DIM, C_sp_addr,
                                        DIM, DIM, DIM, odims);
                		gemmini_extended_compute_preloaded(A_sp_addr+6*bidims, GARBAGE_ADDR, DIM, odims, DIM, odims);

				gemmini_extended_preload(B_sp_addr + 7 * DIM, C_sp_addr,
                                        DIM, DIM, DIM, odims);
				gemmini_extended_compute_preloaded(A_sp_addr+7*bidims, GARBAGE_ADDR, DIM, odims, DIM, odims);
				kch_bound = kch + 8*DIM;
                
                	}
	    //if kch is not divisible by DIM
 	    		for (; kch_bound < kchs; kch_bound += DIM) {
//	        		const int K = kchs - kch > DIM ? DIM : kchs - kch;
				const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kchs + kch_bound;
//                		gemmini_extended_mvin(weights + kch_bound * out_channels + och,
//                       		B_sp_addr,
//                        		DIM, DIM);
				const uint32_t A_sp_addr = A_sp_addr_start + (kch_bound / DIM)*bidims + b*idims; 
	
		                gemmini_extended_preload(B_sp_addr, C_sp_addr,
                		           DIM, DIM, DIM, odims);
                		gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, DIM, odims, DIM, odims);
               
                	}
//	const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;
//	if(output!=NULL) gemmini_extended_mvout(output + (b * out_dim * out_dim)*out_channels + och, C_sp_addr, DIM, 0);
	
            	}
       	   }
  	}

 // mvout output
//   printf("mvout \n");
   if (output != NULL) {
		gemmini_extended_config_st(out_channels * sizeof(elem_t), 0, 1, out_dim, 0, 0, orows, ocols, 0, 0);
		for(int b = 0; b < batches; b++)
			for(int och = 0; och < ochs; och += DIM){
				const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;
				gemmini_extended_mvout(output + (b * out_dim * out_dim)*out_channels + och, C_sp_addr, DIM, 0);
			}	
	}

}

void sp_tiled_conv_dw(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim, int pool_out_dim,

        int stride, int padding, int kernel_dim,

        int pool_size, int pool_stride, int pool_padding,

        int batches,
        int porows, int pocols,// int pochs,
        int lpad, int rpad, int upad, int dpad,
        int plpad, int prpad, int pupad, int pdpad,

        elem_t * input,
        // uint32_t B_sp_addr_start,
        elem_t * weights,
        elem_t * output,
        acc_t * bias,

	    int act, acc_scale_t scale, int relu6_shift,
        bool no_bias, bool no_pool, bool mvin_weight
	) {

    const int orows = porows * pool_stride + pool_size - 1 - pupad - pdpad;
    const int ocols = pocols * pool_stride + pool_size - 1 - plpad - prpad;
    // Calculate image dimensions
    const int irows = (orows - 1) * stride + kernel_dim;
    const int icols = (ocols - 1) * stride + kernel_dim;//kcols; 
    const int irows_unpadded = irows - upad - dpad;
    const int icols_unpadded = icols - lpad - rpad;
    int kchs = 1;
    int kdims = kernel_dim * kernel_dim; 

    int double_bank = 0;//weight_bank > 1 ? 1 : 0;
    int triple_bank = 0;//weight_bank > 2 ? 1 : 0;
	const int odims = ocols*orows;
	const int row_left = odims%DIM;
	const int row_turn = row_left == 0 ? odims/DIM - 1 : odims/DIM;
	gemmini_extended2_config_ex(WEIGHT_STATIONARY, act, 0, scale, relu6_shift, 1, false, false, ocols, row_turn, kernel_dim, stride, kchs, row_left, kdims, double_bank, triple_bank); //if want 2 banks for weight, last is 1

    int idims = irows*icols;
    int bidims = batches*idims;

    const uint32_t A_sp_addr_start = 0;
    const uint32_t B_sp_addr_start = (BANK_NUM-1) * BANK_ROWS;
    const uint32_t D_sp_addr_start = 1 << (ADDR_LEN - 1);
    const uint32_t C_sp_addr_start = 3 << (ADDR_LEN - 2);

   if (!no_bias && bias != NULL) { 
       gemmini_config_ld(0);
       for (int b = 0; b < batches; b++){
	    const int J = 1;
	    const uint32_t D_sp_addr = D_sp_addr_start + b * odims;// + odim;
            for(int odim = 0; odim < odims; odim += DIM){ //both dimension at the same time
		const int I = odims - odim > DIM ? DIM : odims - odim;
                gemmini_extended_mvin(bias,// + och,
                      	D_sp_addr+odim,
                       	J, I);
	    }
	  }
    }	

  if (mvin_weight) {
    // mvin weights
    // printf("weight move in \n");
       gemmini_config_ld(out_channels * sizeof(elem_t));
       for (int krow = 0; krow < kernel_dim; krow++){
            const uint32_t B_sp_addr = B_sp_addr_start+ krow*kernel_dim;
 
            for (int kcol = 0; kcol < kernel_dim; kcol++){
                    gemmini_extended_mvin(weights + (krow*kernel_dim + kcol) * out_channels,
                        B_sp_addr+kcol,
                        1, 1);
	    }
      }
    }
  
    // mvin input
    // printf("mvin inputs\n");
    gemmini_config_ld(in_channels * sizeof(elem_t));
    static elem_t zeros[MAX_BYTES / sizeof(elem_t)] = {0};
//    gemmini_fence(); // TODO fix ROB to get rid of this requirement
    for (int b = 0; b < batches; b++) {
        for (int irow = -upad; irow < irows_unpadded + dpad; irow++) {
            const int irow_padded = irow + upad;

            for (int icol = -lpad; icol < icols_unpadded + rpad;) {
                // TODO There might be some unnecessary mvins here at the edge of the image

                int I = icols_unpadded - icol > DIM ? DIM : icols_unpadded - icol;
                elem_t * in = input + (b*in_dim*in_dim + irow*in_dim + icol) * in_channels;// + ich;
 
                if (icol < 0) {
                    I = -icol > DIM ? DIM : -icol;
                } else if (icol >= icols_unpadded) {
                    I = icols_unpadded + rpad - icol > DIM ? DIM : icols_unpadded + rpad - icol;
                }
                const bool is_zeros = irow < 0 || irow >= irows_unpadded || icol < 0 || icol >= icols_unpadded; 
                const int icol_padded = icol + lpad;
		const uint32_t A_sp_addr = A_sp_addr_start + b * idims + irow_padded * icols + icol_padded;
		if(is_zeros){
	           	   gemmini_config_ld(0);
			//for (int ich = 0; ich < ichs; ich += DIM) {
                    	   //const int K = ichs - ich > DIM ? DIM : ichs - ich;
                           in = &zeros[0];
                           gemmini_extended_mvin(in,//+ich,
                            A_sp_addr,// + (ich/DIM)*bidims,
                            1, I);
                    	//}
		   gemmini_config_ld(in_channels * sizeof(elem_t));


		}else{
                   //for (int ich = 0; ich < ichs; ich += DIM) {
                       //const int K = ichs - ich > DIM ? DIM : ichs - ich;
                       gemmini_extended_mvin(in,//+ich,
                            A_sp_addr,// + (ich/DIM)*bidims,
                            1, I);

                   // }
		}
                icol += I;
            }
        }
    }
//    gemmini_fence();
	
//   gemmini_config_ld(0);
   for (int b = 0; b < batches; b++){
	    const int J = 1;
	   //const uint32_t D_sp_addr = D_sp_addr_start + b * odims;// + odim;
      	    const uint32_t C_sp_addr_outer = C_sp_addr_start + b * odims;// + odim;
	    
		const uint32_t A_sp_addr = A_sp_addr_start + b*idims;           
		const int kkdims = kdims;
		const uint32_t B_sp_addr = B_sp_addr_start;
		const int K = 1;

            	for(int odim = 0; odim < odims; odim += DIM){ //both dimension at the same time
			const int I = odims - odim > DIM ? DIM : odims - odim;
                        //	gemmini_extended_mvin(bias,// + och,
                        //        	D_sp_addr+odim,
                        //        	J, I);
			const uint32_t C_sp_addr = C_sp_addr_outer + odim;

			for(int kkdim = 0; kkdim < kkdims; kkdim += K){	
                                gemmini_extended_preload(B_sp_addr + kkdim, C_sp_addr,
                                        J, K, J, I);
                                gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, K, I, J, I);
                 
                	}
//		gemmini_extended_mvout(output + (b * out_dim * out_dim)*out_channels, C_sp_addr_outer, J, 0);
//	   }
         }
     }

   // mvout output
//    printf("mvout \n");
//    if (output != NULL) {
//        if (no_pool) {
            for (int b = 0; b < batches; b++)
                for (int orow = 0; orow < orows; orow++)
                    for (int ocol = 0; ocol < ocols; ocol += DIM) {
                        const int I = ocols - ocol > DIM ? DIM : ocols - ocol;
                        const uint32_t C_sp_addr = C_sp_addr_start + b * orows * ocols + orow * ocols + ocol;

                        gemmini_extended_mvout(output + (b*out_dim*out_dim + orow*out_dim + ocol) * out_channels,
                                    C_sp_addr,
                                    1, I);
                        }    
}

//for first layer
void sp_tiled_conv_first(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim, int pool_out_dim,

        int stride, int padding, //int kernel_dim,

        int pool_size, int pool_stride, int pool_padding,

        int batches,
        int porows, int pocols, int pochs,
	int krows, int kchs,

//        int lpad, int rpad, int upad, int dpad,
        int plpad, int prpad, int pupad, int pdpad,

        elem_t * input,
	//uint32_t B_sp_addr_start,
        elem_t * weights,
        elem_t * output,
        acc_t * bias,

	    int act, acc_scale_t scale, int relu6_shift,
        bool no_bias, bool no_pool, bool mvin_weight,
	int weight_bank) {

    const int orows = porows * pool_stride + pool_size - 1 - pupad - pdpad;
    const int ocols = pocols * pool_stride + pool_size - 1 - plpad - prpad;
    const int ochs = pochs;

    // Calculate image dimensions
    const int irows = (orows - 1) * stride + krows;
    const int icols = (ocols - 1) * stride + 1;//krows; 
    int kdims = krows*krows; 
    const int ichs = kchs*krows; //pack rows (kchs: normal channel number)

    int double_bank = weight_bank > 1 ? 1 : 0;
    int triple_bank = weight_bank > 2 ? 1 : 0;
    const int odims = ocols*orows;
    const int row_left = odims%DIM;
    const int row_turn = row_left == 0 ? odims/DIM - 1 : odims/DIM;
    gemmini_extended2_config_ex(WEIGHT_STATIONARY, act, 0, scale, relu6_shift, 1, false, false, ocols, row_turn, 1, stride, ichs, row_left, krows, double_bank, triple_bank); //if want 2 banks for weight, last is 1

    int idims = irows*icols;
    int bidims = batches*idims;

    const uint32_t A_sp_addr_start = 0;
    const uint32_t B_sp_addr_start = (BANK_NUM - weight_bank) * BANK_ROWS;// - B_rows;
    const uint32_t D_sp_addr_start = 1 << (ADDR_LEN - 1);
    const uint32_t C_sp_addr_start = 3 << (ADDR_LEN - 2);

    // printf("mvin bias\n");
    // mvin bias

    if (!no_bias && bias != NULL) {
        // TODO we probably don't need quite this many nested loops for this part
        gemmini_config_ld(0);
        for (int b = 0; b < batches; b++)
           for (int och = 0; och < ochs; och += DIM) {
               const int J = ochs - och > DIM ? DIM : ochs - och;
               const uint32_t D_sp_addr = D_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;
	        for(int odim = 0; odim < odims; odim += DIM){
                   // const int I = ocols - ocol > DIM ? DIM : ocols - ocol;
		    const int I = odims - odim > DIM ? DIM : odims - odim;
                        gemmini_extended_mvin(bias + och,
                                D_sp_addr+odim,
                                J, I);
                    }
                }
    	   }

//	printf("mvin_weight \n");
    if(mvin_weight){
	gemmini_config_ld(out_channels*sizeof(elem_t));
	  for (int och = 0; och < ochs; och += DIM) {
       		const int J = ochs - och > DIM ? DIM : ochs - och;
		for (int ich = 0; ich < ichs; ich += DIM) { //duplication for first layer
       		    const int K = ichs - ich > DIM ? DIM : ichs - ich;
        		    for (int krow = 0; krow < krows; krow++){
	               		   const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * krows * ichs+ ich*krows + krow*K;//krow * kcols * kchs + kcol * kchs + kch;
                      		   gemmini_extended_mvin(weights + (krow*(krows*in_channels) + ich) * out_channels + och,
                      				B_sp_addr,
                        			J, K);
	    			}
       			    }
    		  }	
    }
    // mvin input
//     printf("mvin inputs\n");
    gemmini_config_ld(in_channels * sizeof(elem_t));
    for (int b = 0; b < batches; b++) {
        for (int irow = 0; irow < irows; irow++) {
            for (int icol = 0; icol < icols;) {
                int I = icols - icol > DIM ? DIM : icols- icol;
                elem_t * in = input + (b*in_dim*in_dim + irow*in_dim + icol) * in_channels;// + ich;
 
		const uint32_t A_sp_addr = A_sp_addr_start + b * idims + irow * icols + icol;
                for (int ich = 0; ich < ichs; ich += DIM) {
                     const int K = ichs - ich > DIM ? DIM : ichs - ich;
                     gemmini_extended_mvin(in+ich,
                            A_sp_addr + (ich/DIM)*bidims,
                            K, I);

                }
                icol += I;
            }
        }
    }

//   printf("matmul computation \n");
   for (int b = 0; b < batches; b++){
        for (int och = 0; och < ochs; och += DIM) {
            const int J = ochs - och > DIM ? DIM : ochs - och;
	    const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;
	    for (int kch = 0; kch < ichs; kch += DIM) { //treat as 3x7=21 channels
	        const int K = ichs - kch > DIM ? DIM : ichs - kch;
		const uint32_t A_sp_addr = A_sp_addr_start + (kch / DIM)*batches*idims + b*idims;           
		const int kkdims = K*krows;//kdims;
		const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kchs * kdims + kch*krows;

            	for(int odim = 0; odim < odims; odim += DIM){ //both dimension at the same time
			const int I = odims - odim > DIM ? DIM : odims - odim;
			for(int kkdim = 0; kkdim < kkdims; kkdim += K){	
                                gemmini_extended_preload(B_sp_addr + kkdim, C_sp_addr+odim,
                                        J, K, J, I);
                                gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, K, I, J, I);
			}
                }
             }
/*
 //attempt to merge mvout with matmul
     	       if(output!=NULL){
                     elem_t * pout = output + (b * pool_out_dim * pool_out_dim)*out_channels + och;
                    gemmini_extended_mvout(pout,
                            C_sp_addr,
                            J, 0);
 		}
*/ 

        }
   }

    // mvout output
   if (output != NULL) {
               gemmini_extended_config_st(out_channels * sizeof(elem_t), pool_stride, pool_size, pool_out_dim, porows, pocols, orows, ocols, pupad, plpad);
           for (int b = 0; b < batches; b++) {
               for (int poch = 0; poch < pochs; poch += DIM) {
                    const int channels = poch + DIM >= pochs ? pochs - poch : DIM;
                     elem_t * pout = output + (b * pool_out_dim * pool_out_dim)*out_channels + poch;
                    const uint32_t C_sp_addr = C_sp_addr_start + (poch / DIM) * batches * orows * ocols + b * orows * ocols;
                    gemmini_extended_mvout(pout,
                            C_sp_addr,
                            channels, 0);
                }
            }
   }
}

//has mvin weight
void sp_tiled_conv_ws_original(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim, int pool_out_dim,

        int stride, int padding, int kernel_dim,

        int pool_size, int pool_stride, int pool_padding,

        int batches,
        int porows, int pocols, int pochs,
	int krows, int kchs,

        int lpad, int rpad, int upad, int dpad,
        int plpad, int prpad, int pupad, int pdpad,

        elem_t * input,
        elem_t * weights,
        elem_t * output,
        acc_t * bias,

	    int act, int scale, int relu6_shift,
        bool no_bias, bool no_pool,
	int weight_bank) {

    const int orows = porows * pool_stride + pool_size - 1 - pupad - pdpad;
    const int ocols = pocols * pool_stride + pool_size - 1 - plpad - prpad;
    const int ochs = pochs;

    // Calculate image dimensions
    const int irows = (orows - 1) * stride + krows;
    const int icols = (ocols - 1) * stride + krows;//kcols; 
    const int irows_unpadded = irows - upad - dpad;
    const int icols_unpadded = icols - lpad - rpad;
    const int ichs = kchs;
    int kdims = krows*krows; 
    int idims = irows*icols;
    int bidims = batches*irows*icols;

    int odims = ocols*orows;
    const int im2col_width = kdims*kchs;
    const int row_left = odims%DIM;
    const int row_turn = row_left == 0 ? odims/DIM - 1 : odims/DIM;
    const int turn = im2col_width%DIM == 0 ? im2col_width/DIM : im2col_width/DIM + 1;
    const int double_bank = weight_bank > 1 ? 1 : 0;
    const int triple_bank = weight_bank > 2 ? 1 : 0;
    gemmini_extended2_config_ex(WEIGHT_STATIONARY, act, 0, scale, relu6_shift, 1, false, false, ocols, row_turn, krows, stride, kchs, row_left, kdims, double_bank, triple_bank); //if want 2 banks for weight, last is 1

    const uint32_t A_sp_addr_start = 0;
    const uint32_t B_sp_addr_start = (BANK_NUM - weight_bank) * BANK_ROWS;
    const uint32_t D_sp_addr_start = 1 << (ADDR_LEN - 1);
    const uint32_t C_sp_addr_start = 3 << (ADDR_LEN - 2);

    // printf("mvin bias\n");
    // mvin bias
    if (!no_bias && bias != NULL) {
        // TODO we probably don't need quite this many nested loops for this part
        gemmini_config_ld(0);
        for (int b = 0; b < batches; b++)
          for (int och = 0; och < ochs; och += DIM) {
               const int J = ochs - och > DIM ? DIM : ochs - och;
               const uint32_t D_sp_addr = D_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;
	        for(int odim = 0; odim < odims; odim += DIM){
       		    const int I = odims - odim > DIM ? DIM : odims - odim;
                    gemmini_extended_mvin(bias + och,
                                D_sp_addr+odim,
                                J, I);
                }
          }
    }

    // mvin weights
//    printf("weight move in \n");
    gemmini_config_ld(out_channels * sizeof(elem_t));
    for (int och = 0; och < ochs; och += DIM) {
        const int J = ochs - och > DIM ? DIM : ochs - och;

      for (int kch = 0; kch < kchs; kch += DIM) {
        const int K = kchs - kch > DIM ? DIM : kchs - kch;
        for (int krow = 0; krow < krows; krow++){
            const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kdims * kchs + kch*kdims + krow*krows*K;
 
            for (int kcol = 0; kcol < krows; kcol++){
                    gemmini_extended_mvin(weights + (krow*kernel_dim*in_channels + kcol*in_channels + kch) * out_channels + och,
                        B_sp_addr+kcol*K,
                        J, K);
	    }
        }
      }
    }
    // mvin input
    // printf("mvin inputs\n");
    gemmini_config_ld(in_channels * sizeof(elem_t));
    static elem_t zeros[MAX_BYTES / sizeof(elem_t)] = {0}; 
//    gemmini_fence(); // TODO fix ROB to get rid of this requirement
    for (int b = 0; b < batches; b++) {
        for (int irow = -upad; irow < irows_unpadded + dpad; irow++) {
            const int irow_padded = irow + upad;

            for (int icol = -lpad; icol < icols_unpadded + rpad;) {
                int I = icols_unpadded - icol > DIM ? DIM : icols_unpadded - icol;
                elem_t * in = input + (b*in_dim*in_dim + irow*in_dim + icol) * in_channels;// + ich;
 
                if (icol < 0) {
                    I = -icol > DIM ? DIM : -icol;
                } else if (icol >= icols_unpadded) {
                    I = icols_unpadded + rpad - icol > DIM ? DIM : icols_unpadded + rpad - icol;
                }
                const bool is_zeros = irow < 0 || irow >= irows_unpadded || icol < 0 || icol >= icols_unpadded; 
                const int icol_padded = icol + lpad;
		const uint32_t A_sp_addr = A_sp_addr_start + b * idims + irow_padded * icols + icol_padded;
		if(is_zeros){
                  	   gemmini_config_ld(0); 
			for (int ich = 0; ich < ichs; ich += DIM) {
                    	   const int K = ichs - ich > DIM ? DIM : ichs - ich;
                           in = &zeros[0];
                           gemmini_extended_mvin(in+ich,
                            A_sp_addr + (ich/DIM)*bidims,
                            K, I);
                         }
		        gemmini_config_ld(in_channels * sizeof(elem_t));
		}else{
                   for (int ich = 0; ich < ichs; ich += DIM) {
                       const int K = ichs - ich > DIM ? DIM : ichs - ich;
                       gemmini_extended_mvin(in+ich,
                            A_sp_addr + (ich/DIM)*bidims,
                            K, I);
                    }
		}
                icol += I;
            }
        }
    }
//    gemmini_fence(); // TODO fix ROB to get rid of this requirement
  // Compute
   for (int b = 0; b < batches; b++){
        for (int och = 0; och < ochs; och += DIM) {
            const int J = ochs - och > DIM ? DIM : ochs - och;
 	    for (int kch = 0; kch < kchs; kch += DIM) {
	        const int K = kchs - kch > DIM ? DIM : kchs - kch;
		const uint32_t A_sp_addr = A_sp_addr_start + (kch / DIM)*batches*idims + b*idims;           
		const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kdims * kchs + kch*kdims;	
            	for(int odim = 0; odim < odims; odim += DIM){ //both dimension at the same time
			const int I = odims - odim > DIM ? DIM : odims - odim;
     	       	    	const int C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims + odim;	           
			for(int kkdim = 0; kkdim < K*kdims; kkdim += K){
			        gemmini_extended_preload(B_sp_addr+kkdim, C_sp_addr,
                                        J, K, J, I);
                                gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, K, I, J, I);
			}
                    }
                }
            }
       }

    // mvout output
   if (output != NULL) {
        if (no_pool) {
            for (int b = 0; b < batches; b++)
                for (int orow = 0; orow < orows; orow++)
                    for (int ocol = 0; ocol < ocols; ocol += DIM) {
                        const int I = ocols - ocol > DIM ? DIM : ocols - ocol;

                        for (int och = 0; och < ochs; och += DIM) {
                            const int J = ochs - och > DIM ? DIM : ochs - och;
		            const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * orows * ocols + b * orows * ocols + orow * ocols + ocol;

                            gemmini_extended_mvout(output + (b*out_dim*out_dim + orow*out_dim + ocol) * out_channels + och,
                                    C_sp_addr,
                                    J, I);
                        }
                    
                    }

	   } else {
//		   printf("pool \n");
              gemmini_extended_config_st(out_channels * sizeof(elem_t), pool_stride, pool_size, pool_out_dim, porows, pocols, orows, ocols, pupad, plpad);
            gemmini_fence(); // TODO remove this when the ROB can accurately handle these
            for (int b = 0; b < batches; b++) {
                for (int poch = 0; poch < pochs; poch += DIM) {
                    const int channels = poch + DIM >= pochs ? pochs - poch : DIM;
                    elem_t * pout = output + (b * pool_out_dim * pool_out_dim)*out_channels + poch;
                    const uint32_t C_sp_addr = C_sp_addr_start + (poch / DIM) * batches * orows * ocols + b * orows * ocols;
                    gemmini_extended_mvout(pout,
                            C_sp_addr,
                            channels, 0);
                }
            }
            gemmini_fence(); // TODO remove this when the ROB can accurately handle these
       }
   }

}

//first layer padding region
void sp_tiled_conv_ws_original_first(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim, int pool_out_dim,

        int stride, int padding, int kernel_dim,

        int pool_size, int pool_stride, int pool_padding,

        int batches,
        int porows, int pocols, int pochs,
	int krows, int kchs,

        int lpad, int rpad, int upad, int dpad,
        int plpad, int prpad, int pupad, int pdpad,

        elem_t * input,
        elem_t * weights,
        elem_t * output,
        acc_t * bias,

        int act, acc_scale_t scale, int relu6_shift,
        bool no_bias, bool no_pool, bool mvin_weight,
	int weight_bank) {

    const int orows = porows * pool_stride + pool_size - 1 - pupad - pdpad;
    const int ocols = pocols * pool_stride + pool_size - 1 - plpad - prpad;
    const int ochs = pochs;

    // Calculate image dimensions
    const int irows = (orows - 1) * stride + krows;
    const int icols = (ocols - 1) * stride + krows;//kcols; 
    const int irows_unpadded = irows - upad - dpad;
    const int icols_unpadded = icols - lpad - rpad;
    const int ichs = kchs;
    int kdims = krows*krows; 
int idims = irows*icols;
int bidims = batches*irows*icols;

    int odims = ocols*orows;
    const int row_left = odims%DIM;
    const int row_turn = row_left == 0 ? odims/DIM - 1 : odims/DIM;
    const int double_bank = weight_bank > 1 ? 1 : 0;
    const int triple_bank = weight_bank > 2 ? 1 : 0;
    gemmini_extended2_config_ex(WEIGHT_STATIONARY, act, 0, scale, relu6_shift, 1, false, false, ocols, row_turn, krows, stride, kchs, row_left, kdims, double_bank, triple_bank); //if want 2 banks for weight, last is 1

    const uint32_t A_sp_addr_start = 0;
    const uint32_t B_sp_addr_start = (BANK_NUM - weight_bank) * BANK_ROWS;
    const uint32_t D_sp_addr_start = 1 << (ADDR_LEN - 1);
    const uint32_t C_sp_addr_start = 3 << (ADDR_LEN - 2);

    // printf("mvin bias\n");
    // mvin bias
    if (!no_bias && bias != NULL) {
        // TODO we probably don't need quite this many nested loops for this part
        gemmini_config_ld(0);
        for (int b = 0; b < batches; b++)
          for (int och = 0; och < ochs; och += DIM) {
               const int J = ochs - och > DIM ? DIM : ochs - och;
               const uint32_t D_sp_addr = D_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;
	        for(int odim = 0; odim < odims; odim += DIM){
		    const int I = odims - odim > DIM ? DIM : odims - odim;
                    gemmini_extended_mvin(bias + och,
                                D_sp_addr+odim,
                                J, I);
                }
           }
    }

    // mvin input
    // printf("mvin inputs\n");
    gemmini_config_ld(in_channels * sizeof(elem_t));
    static elem_t zeros[MAX_BYTES / sizeof(elem_t)] = {0}; 
   for (int b = 0; b < batches; b++) {
        for (int irow = -upad; irow < irows_unpadded + dpad; irow++) {
            const int irow_padded = irow + upad;

            for (int icol = -lpad; icol < icols_unpadded + rpad;) {
                int I = icols_unpadded - icol > DIM ? DIM : icols_unpadded - icol;
                elem_t * in = input + (b*in_dim*in_dim + irow*in_dim + icol) * in_channels;// + ich;
 
                if (icol < 0) {
                    I = -icol > DIM ? DIM : -icol;
                } else if (icol >= icols_unpadded) {
                    I = icols_unpadded + rpad - icol > DIM ? DIM : icols_unpadded + rpad - icol;
                }
                const bool is_zeros = irow < 0 || irow >= irows_unpadded || icol < 0 || icol >= icols_unpadded; 
                const int icol_padded = icol + lpad;
		const uint32_t A_sp_addr = A_sp_addr_start + b * idims + irow_padded * icols + icol_padded;
		if(is_zeros){
                        gemmini_config_ld(0); 
                        in = &zeros[0];
                        gemmini_extended_mvin(in,
                            A_sp_addr,
                            ichs, I);
	 	        gemmini_config_ld(in_channels * sizeof(elem_t));
		}else{
                      gemmini_extended_mvin(in,
                            A_sp_addr,
                            ichs, I);

       		}
                icol += I;
            }
        }
    }
//    gemmini_fence(); // TODO fix ROB to get rid of this requirement

  if(mvin_weight){
    gemmini_config_ld(out_channels * sizeof(elem_t));
    for (int och = 0; och < ochs; och += DIM) {
        const int J = ochs - och > DIM ? DIM : ochs - och;
        const int K = kchs;//kchs - kch > DIM ? DIM : kchs - kch;
        for (int krow = 0; krow < krows; krow++){
            const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kdims * kchs + krow*krows*K;
 
            for (int kcol = 0; kcol < krows; kcol++){
                    gemmini_extended_mvin(weights + (krow*kernel_dim*in_channels + kcol*in_channels) * out_channels + och,
                        B_sp_addr+kcol*K,
                        J, K);
	    }
        }
    
    }
  }

  // Compute
//if(krows != 1){
    for (int b = 0; b < batches; b++){
        for (int och = 0; och < ochs; och += DIM) {
            const int J = ochs - och > DIM ? DIM : ochs - och;
      	    const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;	           
	    const int K = kchs;// - kch > DIM ? DIM : kchs - kch;
	    const uint32_t A_sp_addr = A_sp_addr_start + b*idims;           
	    const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kdims * kchs;// + kch*kdims;	
            for(int odim = 0; odim < odims; odim += DIM){ //both dimension at the same time
		const int I = odims - odim > DIM ? DIM : odims - odim;
     		for(int kkdim = 0; kkdim < K*kdims; kkdim += K){
		        gemmini_extended_preload(B_sp_addr+kkdim, C_sp_addr+odim,
                                J, K, J, I);
                        gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, K, I, J, I);
		}
            }
/* 
 //attempt to merge matmul and mvout          
                    elem_t * pout = output + (b * pool_out_dim * pool_out_dim)*out_channels + och;
                    gemmini_extended_mvout(pout,
                            C_sp_addr,
                            J, 0);
*/ 
	}
    }

   if (output != NULL) {
           gemmini_extended_config_st(out_channels * sizeof(elem_t), pool_stride, pool_size, pool_out_dim, porows, pocols, orows, ocols, pupad, plpad);
           for (int b = 0; b < batches; b++) {
                for (int poch = 0; poch < pochs; poch += DIM) {
                    const int channels = poch + DIM >= pochs ? pochs - poch : DIM;

                    elem_t * pout = output + (b * pool_out_dim * pool_out_dim)*out_channels + poch;

                    const uint32_t C_sp_addr = C_sp_addr_start + (poch / DIM) * batches * orows * ocols + b * orows * ocols;

                    gemmini_extended_mvout(pout,
                            C_sp_addr,
                            channels, 0);
                }
            }
   }

}

static int tiled_conv_total_spad_rows(bool acc, bool weight,
        int stride,
        int batches,
        int porows, int pocols, int ochs,
        int krows, int kcols, int kchs,
        int pool_size, int pool_stride) {

    const int orows = porows * pool_stride + pool_size - 1;
    const int ocols = pocols * pool_stride + pool_size - 1;

    const int irows = orows * stride + krows - 1; // - 2 * padding;
    const int icols = ocols * stride + kcols - 1; // - 2 * padding;
    const int ichs = kchs;

    const int in_channels_per_bank = ichs / DIM + (ichs % DIM != 0);
    const int out_channels_per_bank = ochs / DIM + (ochs % DIM != 0);

    const int A_rows = in_channels_per_bank * batches * irows * icols;
    const int B_rows = out_channels_per_bank * kcols * krows * kchs;
    const int C_rows = out_channels_per_bank * batches * orows * ocols;

    if (acc)
        return C_rows;
    else if(weight)
	return B_rows;
    else
        return A_rows;
}


void conv_cpu_without_pool(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift) {

  bool no_bias = bias == NULL;

  for (int b = 0; b < batch_size; b++) {
    for (int orow = 0; orow < out_dim; orow++) {
      for (int ocol = 0; ocol < out_dim; ocol++) {
        for (int och = 0; och < out_channels; och++) {

          acc_t opixel = no_bias ? 0 : bias[och];

          for (int krow = 0; krow < kernel_dim; krow++) {
            const int irow = orow * stride + krow - padding;

            for (int kcol = 0; kcol < kernel_dim; kcol++) {
              const int icol = ocol * stride + kcol - padding;

              for (int kch = 0; kch < in_channels; kch++) {
                elem_t ipixel = irow < 0 || irow >= in_dim || icol < 0 || icol >= in_dim ?
                    0 :
                    *(input + (b * in_dim * in_dim + irow * in_dim + icol) * in_channels + kch);

                elem_t weight = *(weights + (krow * kernel_dim * in_channels + kcol * in_channels + kch) * out_channels + och);

                //acc_t past_opixel = opixel;
                opixel += weight * ipixel;
              }
            }
          }

          *(output+(b*out_dim*out_dim+orow*out_dim+ocol)*out_channels + och) =
            scale_and_sat(opixel, act, scale, relu6_shift);
        }
      }
    }
  }
}

void conv_cpu(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding) {

  const bool no_pool = pool_stride == 0;
  if (no_pool) {
    conv_cpu_without_pool(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding, kernel_dim,
        input, weights, bias, output,
        act, scale, relu6_shift);
    return;
  }

  const bool no_bias = bias == NULL;
  const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;

  for (int b = 0; b < batch_size; b++) {
    for (int porow = 0; porow < pool_out_dim; porow++) {
      for (int pocol = 0; pocol < pool_out_dim; pocol++) {
        for (int poch = 0; poch < out_channels; poch++) {

          elem_t running_max = 0;
          bool running_max_initialized = false;

          for (int pwrow = 0; pwrow < pool_size; pwrow++) {
            const int orow = porow * pool_stride + pwrow - pool_padding;

            for (int pwcol = 0; pwcol < pool_size; pwcol++) {
              const int ocol = pocol * pool_stride + pwcol - pool_padding;

              if (orow < 0 || orow >= out_dim || ocol < 0 || ocol >= out_dim) {
                if (!running_max_initialized || running_max < 0) {
                  running_max = 0;
                  running_max_initialized = true;
                }
              } else {

                acc_t opixel = no_bias ? 0 : bias[poch];

                for (int krow = 0; krow < kernel_dim; krow++) {
                  const int irow = orow * stride + krow - padding;

                  for (int kcol = 0; kcol < kernel_dim; kcol++) {
                    const int icol = ocol * stride + kcol - padding;

                    for (int kch = 0; kch < in_channels; kch++) {
                      elem_t ipixel = irow < 0 || irow >= in_dim || icol < 0 || icol >= in_dim ?
                          0 :
                          *(input + (b * in_dim * in_dim + irow * in_dim + icol) * in_channels + kch);

                      elem_t weight = *(weights + (krow * kernel_dim * in_channels + kcol * in_channels + kch) * out_channels + poch);

                      opixel += weight * ipixel;
                    }
                  }
                }

                opixel = scale_and_sat(opixel, act, scale, relu6_shift);
                if (!running_max_initialized || opixel > running_max) {
                  running_max = opixel;
                  running_max_initialized = true;
                }
              }

              if (pwrow == pool_size - 1 && pwcol == pool_size - 1) {
                *(output + (b*pool_out_dim*pool_out_dim + porow*pool_out_dim + pocol)*out_channels + poch) = running_max;
              }
            }
          }
        }
      }
    }
  }
}

void tiled_conv_dw(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        int batches,
        int porows, int pocols,// int pochs,
//        int krows, int kcols, int kchs,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,

        enum tiled_matmul_type_t tiled_conv_type) {

    const int krows = kernel_dim;
    const int kcols = kernel_dim;
    bool no_bias = false;
    if (bias == NULL) {
        bias = (acc_t*)1;
        no_bias = true;
    }

    bool no_pool = pool_stride == 0;
    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }

#ifdef GEMMINI_ASSERTIONS
    {
        // Check that data will fit in scratchpad
        const int spad_rows_weight = tiled_conv_total_spad_rows(false, true,
            stride, batches, porows, pocols, 1, kcols, kcols, 1, pool_size, pool_stride);
        const int spad_rows_input = tiled_conv_total_spad_rows(false, false,
            stride, batches, porows, pocols, 1, kcols, kcols, 1, pool_size, pool_stride);
        const int acc_rows = tiled_conv_total_spad_rows(true, false,
            stride, batches, porows, pocols, 1, kcols, kcols, 1, pool_size, pool_stride);

	const int weight_bank = 1;

        if (spad_rows_weight > BANK_ROWS * weight_bank) {
            printf("not enough scratchpad space to store weights\n");
            exit(1);
        }
        if (spad_rows_input > BANK_ROWS*(BANK_NUM - weight_bank)) {
            printf("not enough scratchpad space to store inputs\n");
            exit(1);
        }
        if (acc_rows > ACC_ROWS) {
            printf("not enough accumulator space to store outputs\n");
            exit(1);
        }
    }
#endif

    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;
    if (no_pool) {
        gemmini_config_st(out_channels * sizeof(elem_t));
    }
       for (int b = 0; b < batch_size; b += batches) {
     	    for (int porow = 0; porow < pool_out_dim; porow += porows) {
            	const int orow = porow * pool_stride - pool_padding;
            	for (int pocol = 0; pocol < pool_out_dim; pocol += pocols) {
		    for(int poch = 0; poch < out_channels; poch += 1){
			int kch = poch;
		        bool mvin_weight = true;
                        const int ocol = pocol * pool_stride - pool_padding;
                        const int orow_floored = orow < 0 ? 0 : orow;
                        const int irow = orow_floored * stride - padding;
                        const int ocol_floored = ocol < 0 ? 0 : ocol;
                        const int icol = ocol_floored * stride - padding;

                            //for (int kch = 0; kch < in_channels; kch += 1) {
                                elem_t * out = output + (b*pool_out_dim*pool_out_dim + porow*pool_out_dim + pocol) * out_channels + poch;

                                acc_t * bias_ = bias + poch;
                                const int batches_ = batch_size - b > batches ? batches : batch_size - b;
                                const int porows_ = pool_out_dim - porow > porows ? porows : pool_out_dim - porow;
                                const int pocols_ = pool_out_dim - pocol > pocols ? pocols : pool_out_dim - pocol;
                                const int ocols_ = pocols_ * pool_stride + pool_size - 1;
                                const int orows_ = porows_ * pool_stride + pool_size - 1;

                                const int plpad = ocol < 0 ? -ocol : 0;
                                const int prpad = ocol + ocols_ > out_dim ? ocol + ocols_ - out_dim : 0;
                                const int pupad = orow < 0 ? -orow : 0;
                                const int pdpad = orow + orows_ > out_dim ? orow + orows_ - out_dim : 0;

				const int icols_ = (ocols_ - 1 - plpad - prpad) * stride + kcols;//+ kcols_;
                                const int irows_ = (orows_ - 1 - pupad - pdpad) * stride + kcols;//krows_;


                                const int lpad = icol < 0 ? -icol : 0;
                                const int rpad = icol + icols_ > in_dim ? icol + icols_ - in_dim : 0;
                                const int upad = irow < 0 ? -irow : 0;
                                const int dpad = irow + irows_ > in_dim ? irow + irows_ - in_dim : 0;

                                // printf("upad: %d\n", upad);
                                // printf("dpad: %d\n", dpad);
                                // printf("lpad: %d\n", lpad);
                                // printf("rpad: %d\n", rpad);
                                // printf("pupad: %d\n", pupad);
                                // printf("pdpad: %d\n", pdpad);
                                // printf("plpad: %d\n", plpad);
                                // printf("prpad: %d\n", prpad);

                                sp_tiled_conv_dw(
                                    batch_size, in_dim, in_channels,
                                    out_channels, out_dim, pool_out_dim,

                                    stride, padding, kernel_dim,

                                    pool_size, pool_stride, pool_padding,

                                    batches_,
                                    porows_, pocols_,// pochs_,

				    lpad, rpad, upad, dpad,
                                    plpad, prpad, pupad, pdpad,

                                    input + (b*in_dim*in_dim + (irow+upad)*in_dim + (icol+lpad)) * in_channels + kch,
                                    //weights + (krow*kernel_dim*in_channels + kcol*in_channels + kch) * out_channels + poch,
			     	    weights + poch,
                                    out,
                                    bias_,

                                    act, scale, relu6_shift,
                                    no_bias, no_pool, mvin_weight);
                            }
                        }
                    }

    }
}

void tiled_conv_first(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding,

        int batches,
        int porows, int pocols, int pochs,
	int kcols, int kchs,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,

	enum tiled_matmul_type_t tiled_conv_type,
	int weight_bank) {


    if (tiled_conv_type == CPU) {
      if (pool_size == 1 && pool_stride == 1 && pool_padding == 0) {
        pool_stride = 0;
      }

      conv_cpu(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding, kcols,//kernel_dim,
        input, weights, bias, output,
        act, scale, relu6_shift,
        pool_size, pool_stride, pool_padding);
      return;
    } else if (tiled_conv_type == OS) {
      printf("Gemmini convs do not currently support OS\n");
      exit(1);
    }

    bool no_bias = false;
    if (bias == NULL) {
        bias = (acc_t*)1;
        no_bias = true;
    }

    bool no_1d = pool_stride == 0 && pool_size == 0;
    bool no_pool = pool_stride == 0;
    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }

//    printf("no_1d: %d, no_pool: %d, pool_size: %d, pool_stride: %d \n", no_1d, no_pool, pool_size, pool_stride);

#ifdef GEMMINI_ASSERTIONS
    {
        // Check that data will fit in scratchpad
        const int spad_rows_weight = tiled_conv_total_spad_rows(false, true,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);
        const int spad_rows_input = tiled_conv_total_spad_rows(false, false,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);
        const int acc_rows = tiled_conv_total_spad_rows(true, false,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);

        // printf("spad_rows: %d\n", spad_rows);
        // printf("acc_rows: %d\n", acc_rows);
        // exit(1);

        if (spad_rows_weight > weight_bank*BANK_ROWS) {
            printf("not enough scratchpad space to store weights\n");
            exit(1);
        }
        if (spad_rows_input > BANK_ROWS*(BANK_NUM - weight_bank)) {
            printf("not enough scratchpad space to store inputs\n");
            exit(1);
        }
        if (acc_rows > ACC_ROWS) {
            printf("not enough accumulator space to store outputs\n");
            exit(1);
        }

    }
#endif

    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;

    if (no_pool && no_1d) {
        gemmini_config_st(out_channels * sizeof(elem_t));
    }
    int pmax = 0;
    for(int porow = 0; porow < pool_out_dim; porow += porows)
	pmax = porow;
    bool mvin_weight = false;

    for (int b = 0; b < batch_size; b += batches) {
        for (int porow = 0; porow < pool_out_dim; porow += porows) {
            const int orow = porow * pool_stride - pool_padding;

            for (int pocol = 0; pocol < pool_out_dim; pocol += pocols) {
		if(pocol == 0 || porow == 0 || pocol == pmax || porow == pmax){ //when there is padding
                const int ocol = pocol * pool_stride - pool_padding;
                for (int poch = 0; poch < out_channels; poch += pochs) {
			if(pocol == 0 && porow == 0 && b == 0 && poch == 0) mvin_weight = true;
			else mvin_weight = false;

	                const int orow_floored = orow < 0 ? 0 : orow;
                        const int irow = orow_floored * stride - padding;//+ krow - padding;
                        const int ocol_floored = ocol < 0 ? 0 : ocol;
                        const int icol = ocol_floored * stride - padding; //+ kcol - padding;
//			    printf("icol: %d, ocols: %d, ocol: %d, ocol_floored: %d, kcol: %d \n", icol, ocols, ocol, ocol_floored, kcol);

                            //for (int kch = 0; kch < in_channels; kch += kchs) {
                        elem_t * out = output + (b*pool_out_dim*pool_out_dim + porow*pool_out_dim + pocol) * out_channels + poch;
                                
                        acc_t * bias_ = bias + poch;
                               
                        const int batches_ = batch_size - b > batches ? batches : batch_size - b;
                        const int porows_ = pool_out_dim - porow > porows ? porows : pool_out_dim - porow;
                        const int pocols_ = pool_out_dim - pocol > pocols ? pocols : pool_out_dim - pocol;
                        const int pochs_ = out_channels - poch > pochs ? pochs : out_channels - poch;
			const int kchs_ = in_channels;

                                const int ocols_ = pocols_ * pool_stride + pool_size - 1;
                                const int orows_ = porows_ * pool_stride + pool_size - 1;

                                const int plpad = ocol < 0 ? -ocol : 0;
                                const int prpad = ocol + ocols_ > out_dim ? ocol + ocols_ - out_dim : 0;
                                const int pupad = orow < 0 ? -orow : 0;
                                const int pdpad = orow + orows_ > out_dim ? orow + orows_ - out_dim : 0;

        			const int icols_ = (ocols_ - 1 - plpad - prpad) * stride + kcols;//+ kcols_;
                                const int irows_ = (orows_ - 1 - pupad - pdpad) * stride + kcols;//krows_;

                                const int lpad = icol < 0 ? -icol : 0;
                                const int rpad = icol + icols_ > in_dim ? icol + icols_ - in_dim : 0;
                                const int upad = irow < 0 ? -irow : 0;
                                const int dpad = irow + irows_ > in_dim ? irow + irows_ - in_dim : 0;

                               sp_tiled_conv_ws_original_first(
                                    batch_size, in_dim, in_channels,
                                    out_channels, out_dim, pool_out_dim,

                                    stride, padding, kcols,

                                    pool_size, pool_stride, pool_padding,

                                    batches_,
                                    porows_, pocols_, pochs_,
			            kcols, kchs_,

                                    lpad, rpad, upad, dpad,
                                    plpad, prpad, pupad, pdpad,

                                    input + (b*in_dim*in_dim + (irow+upad)*in_dim + (icol+lpad)) * in_channels,// + kch,	
				    weights + poch,
		 		    //weights + (krow*kernel_dim*in_channels + kcol*in_channels + kch) * out_channels + poch,
                                    out,
                                    bias_,

                                    act, scale, relu6_shift,
                                    no_bias, no_pool, mvin_weight,
                                    weight_bank);
	
                           }
                        }
                    }
	       }
    }

    for (int b = 0; b < batch_size; b += batches) {
        for (int porow = porows; porow < pmax; porow += porows) {
            const int orow = porow * pool_stride - pool_padding;

            for (int pocol = pocols; pocol < pmax; pocol += pocols) {
                const int ocol = pocol * pool_stride - pool_padding;

                for (int poch = 0; poch < out_channels; poch += pochs) {
			if(poch == 0 && pocol == pocols && porow == porows && b == 0) mvin_weight = true;
			else mvin_weight = false;
	                const int orow_floored = orow < 0 ? 0 : orow;
                        const int irow = orow_floored * stride - padding;//+ krow - padding;
                        const int ocol_floored = ocol < 0 ? 0 : ocol;
                        const int icol = ocol_floored * stride - padding; //+ kcol - padding;
                               elem_t * out = output + (b*pool_out_dim*pool_out_dim + porow*pool_out_dim + pocol) * out_channels + poch;
                                
                                acc_t * bias_ = bias + poch;
                               

                                const int batches_ = batch_size - b > batches ? batches : batch_size - b;
                                const int porows_ = pool_out_dim - porow > porows ? porows : pool_out_dim - porow;
                                const int pocols_ = pool_out_dim - pocol > pocols ? pocols : pool_out_dim - pocol;
                                const int pochs_ = out_channels - poch > pochs ? pochs : out_channels - poch;
				const int kchs_ = in_channels;

                                const int ocols_ = pocols_ * pool_stride + pool_size - 1;
                                const int orows_ = porows_ * pool_stride + pool_size - 1;

                                const int plpad = ocol < 0 ? -ocol : 0;
                                const int prpad = ocol + ocols_ > out_dim ? ocol + ocols_ - out_dim : 0;
                                const int pupad = orow < 0 ? -orow : 0;
                                const int pdpad = orow + orows_ > out_dim ? orow + orows_ - out_dim : 0;

       				const int icols_ = (ocols_ - 1 - plpad - prpad) * stride + kcols;//+ kcols_;
                                const int irows_ = (orows_ - 1 - pupad - pdpad) * stride + kcols;//krows_;

                               sp_tiled_conv_first(
                                    batch_size, in_dim, in_channels,
                                    out_channels, out_dim, pool_out_dim,

                                    stride, padding,

                                    pool_size, pool_stride, pool_padding,

                                    batches_,
                                    porows_, pocols_, pochs_,
			            kcols, kchs_,

                                    plpad, prpad, pupad, pdpad,

                                    input + (b*in_dim*in_dim + (irow)*in_dim + (icol)) * in_channels,// + kch,
				    weights + poch,
		 		    //weights + (krow*kernel_dim*in_channels + kcol*in_channels + kch) * out_channels + poch,
                                    out,
                                    bias_,

                                    act, scale, relu6_shift,
                                    no_bias, no_pool, mvin_weight,
                                    weight_bank);
	                           }
                        }
                    }
	     //  }
    }
}


void sp_tiled_conv_ws(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim, int pool_out_dim,

        int stride, int padding, //int kernel_dim,

        int pool_size, int pool_stride, int pool_padding,

        int batches,
        int porows, int pocols, int pochs,
	int krows, int kchs,

        int lpad, int rpad, int upad, int dpad,
        int plpad, int prpad, int pupad, int pdpad,

        elem_t * input,
	uint32_t B_sp_addr_start,
        //elem_t * weights,
        elem_t * output,
        acc_t * bias,

	int act, acc_scale_t scale, int relu6_shift,
        bool no_bias, bool no_pool,
	int weight_bank) {

    const int orows = porows * pool_stride + pool_size - 1 - pupad - pdpad;
    const int ocols = pocols * pool_stride + pool_size - 1 - plpad - prpad;
    const int ochs = pochs;

    // Calculate image dimensions
    const int irows = (orows - 1) * stride + krows;
    const int icols = (ocols - 1) * stride + krows;//kcols; 
    const int irows_unpadded = irows - upad - dpad;
    const int icols_unpadded = icols - lpad - rpad;
    const int ichs = kchs;
    int kdims = krows*krows; 

    int double_bank = weight_bank > 1 ? 1 : 0;
    int triple_bank = weight_bank > 2 ? 1 : 0;
	const int odims = ocols*orows;
//	const int im2col_width = kdims*kchs;
	const int row_left = odims%DIM;
	const int row_turn = row_left == 0 ? odims/DIM - 1 : odims/DIM;
//	const int turn = im2col_width%DIM == 0 ? im2col_width/DIM : im2col_width/DIM + 1;
	gemmini_extended2_config_ex(WEIGHT_STATIONARY, act, 0, scale, relu6_shift, 1, false, false, ocols, row_turn, krows, stride, kchs, row_left, kdims, double_bank, triple_bank); //if want 2 banks for weight, last is 1

    int idims = irows*icols;
    int bidims = batches*idims;
   if(no_pool){
	gemmini_config_st(out_channels*sizeof(elem_t));
   }
   else{
	   gemmini_extended_config_st(out_channels * sizeof(elem_t), pool_stride, pool_size, pool_out_dim, porows, pocols, orows, ocols, pupad, plpad);
   }
    const uint32_t A_sp_addr_start = 0;
    const uint32_t D_sp_addr_start = 1 << (ADDR_LEN - 1);
    const uint32_t C_sp_addr_start = 3 << (ADDR_LEN - 2);

     //printf("mvin bias\n");
    // mvin bias

    if (!no_bias && bias != NULL) {
        // TODO we probably don't need quite this many nested loops for this part
        gemmini_config_ld(0);
        for (int b = 0; b < batches; b++)
          for (int och = 0; och < ochs; och += DIM) {
               const int J = ochs - och > DIM ? DIM : ochs - och;
               const uint32_t D_sp_addr = D_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;
	        for(int odim = 0; odim < odims; odim += DIM){
                   // const int I = ocols - ocol > DIM ? DIM : ocols - ocol;
		    const int I = odims - odim > DIM ? DIM : odims - odim;
                        gemmini_extended_mvin(bias + och,
                                D_sp_addr+odim,
                                J, I);
                    }
                }
    }

    // mvin input
    // printf("mvin inputs\n");
    gemmini_config_ld(in_channels * sizeof(elem_t));
    static elem_t zeros[MAX_BYTES / sizeof(elem_t)] = {0};
//    gemmini_fence(); // TODO fix ROB to get rid of this requirement
    for (int b = 0; b < batches; b++) {
        for (int irow = -upad; irow < irows_unpadded + dpad; irow++) {
            const int irow_padded = irow + upad;

            for (int icol = -lpad; icol < icols_unpadded + rpad;) {
                int I = icols_unpadded - icol > DIM ? DIM : icols_unpadded - icol;
                elem_t * in = input + (b*in_dim*in_dim + irow*in_dim + icol) * in_channels;// + ich;
 
                if (icol < 0) {
                    I = -icol > DIM ? DIM : -icol;
                } else if (icol >= icols_unpadded) {
                    I = icols_unpadded + rpad - icol > DIM ? DIM : icols_unpadded + rpad - icol;
                }
                const bool is_zeros = irow < 0 || irow >= irows_unpadded || icol < 0 || icol >= icols_unpadded; 
                const int icol_padded = icol + lpad;
		const uint32_t A_sp_addr = A_sp_addr_start + b * idims + irow_padded * icols + icol_padded;
		if(is_zeros){
	           	   gemmini_config_ld(0);
			for (int ich = 0; ich < ichs; ich += DIM) {
                    	   const int K = ichs - ich > DIM ? DIM : ichs - ich;
                           in = &zeros[0];
                           gemmini_extended_mvin(in+ich,
                            A_sp_addr + (ich/DIM)*bidims,
                            K, I);
                    }
		   gemmini_config_ld(in_channels * sizeof(elem_t));


		}else{
                   for (int ich = 0; ich < ichs; ich += DIM) {
                       const int K = ichs - ich > DIM ? DIM : ichs - ich;
                       gemmini_extended_mvin(in+ich,
                            A_sp_addr + (ich/DIM)*bidims,
                            K, I);

                    }
		}
                icol += I;
            }
        }
    }

//printf("matmul \n");
   for (int b = 0; b < batches; b++){
        for (int och = 0; och < ochs; och += DIM) {
            const int J = ochs - och > DIM ? DIM : ochs - och;
     	    const uint32_t C_sp_addr_outer = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;

	    for (int kch = 0; kch < kchs; kch += DIM) {
	        const int K = kchs - kch > DIM ? DIM : kchs - kch;
		const uint32_t A_sp_addr = A_sp_addr_start + (kch / DIM)*batches*idims + b*idims;           
		const int kkdims = K*kdims;
		const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kdims * kchs + kch*kdims;
	
            	for(int odim = 0; odim < odims; odim += DIM){ //both dimension at the same time
			const int I = odims - odim > DIM ? DIM : odims - odim;
			const uint32_t C_sp_addr = C_sp_addr_outer + odim;

			for(int kkdim = 0; kkdim < kkdims; kkdim += K){	
                                gemmini_extended_preload(B_sp_addr + kkdim, C_sp_addr,
                                        J, K, J, I);
                                gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, K, I, J, I);
			}
                   }
                }
   	}
     }

   //gemmini_fence();
//printf("mvout\n");
  if(output!=NULL){
     if (no_pool) {
            for (int b = 0; b < batches; b++)
                for (int orow = 0; orow < orows; orow++)
                    for (int ocol = 0; ocol < ocols; ocol += DIM) {
                        const int I = ocols - ocol > DIM ? DIM : ocols - ocol;

                        for (int och = 0; och < ochs; och += DIM) {
                            const int J = ochs - och > DIM ? DIM : ochs - och;
                            const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * orows * ocols + b * orows * ocols + orow * ocols + ocol;

                            gemmini_extended_mvout(output + (b*out_dim*out_dim + orow*out_dim + ocol) * out_channels + och,
                                    C_sp_addr,
                                    J, I);
                        }
                    
                    }

      } else {
            gemmini_fence(); // TODO remove this when the ROB can accurately handle these
            for (int b = 0; b < batches; b++) {
                for (int poch = 0; poch < pochs; poch += DIM) {
                    const int channels = poch + DIM >= pochs ? pochs - poch : DIM;
                    elem_t * pout = output + (b * pool_out_dim * pool_out_dim)*out_channels + poch;
                    const uint32_t C_sp_addr = C_sp_addr_start + (poch / DIM) * batches * orows * ocols + b * orows * ocols;
                    gemmini_extended_mvout(pout,
                            C_sp_addr,
                            channels, 0);
                }
            }
            gemmini_fence(); // TODO remove this when the ROB can accurately handle these
      }
   }
//attempts to merge mvin-matmul-mvout loops
//ROB problem: some have wrong results
/*
   gemmini_config_ld(0);
   for (int b = 0; b < batches; b++){
        for (int och = 0; och < ochs; och += DIM) {
            const int J = ochs - och > DIM ? DIM : ochs - och;
            const uint32_t D_sp_addr = D_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;
      	    const uint32_t C_sp_addr_outer = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;// + odim;

	    for (int kch = 0; kch < kchs; kch += DIM) {
	        const int K = kchs - kch > DIM ? DIM : kchs - kch;
		const uint32_t A_sp_addr = A_sp_addr_start + (kch / DIM)*batches*idims + b*idims;           
		const int kkdims = K*kdims;
		const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kdims * kchs + kch*kdims;
	
            	for(int odim = 0; odim < odims; odim += DIM){ //both dimension at the same time
			const int I = odims - odim > DIM ? DIM : odims - odim;
			if(kch == 0)
                        	gemmini_extended_mvin(bias + och,
                                	D_sp_addr+odim,
                                	J, I);
			const uint32_t C_sp_addr = C_sp_addr_outer + odim;

			for(int kkdim = 0; kkdim < kkdims; kkdim += K){	
                                gemmini_extended_preload(B_sp_addr + kkdim, C_sp_addr,
                                        J, K, J, I);
                                gemmini_extended_compute_preloaded(A_sp_addr, GARBAGE_ADDR, K, I, J, I);
			}
                   }
                }

 	    if(output!=NULL){
		//const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;
		if(no_pool){
  		   gemmini_extended_mvout(output + (b * out_dim * out_dim)*out_channels + och, C_sp_addr_outer, J, 0);
		}
		else{
		   gemmini_extended_mvout(output + (b * pool_out_dim * pool_out_dim) * out_channels + och, C_sp_addr_outer, J, 0);
		}
	   }

   	}
     }
           if (no_pool) {
            for (int b = 0; b < batches; b++)
                for (int orow = 0; orow < orows; orow++)
                    for (int ocol = 0; ocol < ocols; ocol += DIM) {
                        const int I = ocols - ocol > DIM ? DIM : ocols - ocol;

                        for (int och = 0; och < ochs; och += DIM) {
                            const int J = ochs - och > DIM ? DIM : ochs - och;
//			int J = 1; int och = 0;
                            const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * orows * ocols + b * orows * ocols + orow * ocols + ocol;

                            gemmini_extended_mvout(output + (b*out_dim*out_dim + orow*out_dim + ocol) * out_channels + och,
                                    C_sp_addr,
                                    J, I);
                        }

                    }
	   } else {
//		   printf("pool \n");
              gemmini_extended_config_st(out_channels * sizeof(elem_t), pool_stride, pool_size, pool_out_dim, porows, pocols, orows, ocols, pupad, plpad);
//             gemmini_fence(); // TODO remove this when the ROB can accurately handle these
            for (int b = 0; b < batches; b++) {
                for (int poch = 0; poch < pochs; poch += DIM) {
                    const int channels = poch + DIM >= pochs ? pochs - poch : DIM;
                    elem_t * pout = output + (b * pool_out_dim * pool_out_dim)*out_channels + poch;
                    const uint32_t C_sp_addr = C_sp_addr_start + (poch / DIM) * batches * orows * ocols + b * orows * ocols;
                    gemmini_extended_mvout(pout,
                            C_sp_addr,
                            channels, 0);
                }
            }
//            gemmini_fence();
        }
*/
/*
    // mvout output
   if (output != NULL) {
		gemmini_extended_config_st(out_channels * sizeof(elem_t), 0, 1, out_dim, 0, 0, orows, ocols, 0, 0);
		for(int b = 0; b < batches; b++)
			for(int och = 0; och < ochs; och += DIM){
				const int J = ochs - och > DIM ? DIM : ochs - och;
				const uint32_t C_sp_addr = C_sp_addr_start + (och / DIM) * batches * odims + b * odims;
				gemmini_extended_mvout(output + (b * out_dim * out_dim)*out_channels + och, C_sp_addr, J, 0);
			}	
		//}

*/
//            gemmini_fence();
//    	uint64_t end_mvout = read_cyclesh();
//	printf("mvout cycles: %d \n", end_mvout - start_mvout);

}

//outer loop without weight mvin (due to large channel size)
void tiled_conv_original(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        int batches,
        int porows, int pocols, int pochs,
	int kcols, int kchs,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,

	enum tiled_matmul_type_t tiled_conv_type,
	int weight_bank) {


    if (tiled_conv_type == CPU) {
      if (pool_size == 1 && pool_stride == 1 && pool_padding == 0) {
        pool_stride = 0;
      }

      conv_cpu(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding, kernel_dim,
        input, weights, bias, output,
        act, scale, relu6_shift,
        pool_size, pool_stride, pool_padding);
      return;
    } else if (tiled_conv_type == OS) {
      printf("Gemmini convs do not currently support OS\n");
      exit(1);
    }

    bool no_bias = false;
    if (bias == NULL) {
        bias = (acc_t*)1;
        no_bias = true;
    }

//    bool no_1d = pool_stride == 0 && pool_size == 0;
    bool no_pool = pool_stride == 0;
    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }
#ifdef GEMMINI_ASSERTIONS
    {
        // Check that data will fit in scratchpad
        const int spad_rows_weight = tiled_conv_total_spad_rows(false, true,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);
        const int spad_rows_input = tiled_conv_total_spad_rows(false, false,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);
        const int acc_rows = tiled_conv_total_spad_rows(true, false,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);

        if (spad_rows_weight > weight_bank*BANK_ROWS) {
            printf("not enough scratchpad space to store weights\n");
            exit(1);
        }
        if (spad_rows_input > BANK_ROWS*(BANK_NUM - weight_bank)) {
            printf("not enough scratchpad space to store inputs\n");
            exit(1);
        }
        if (acc_rows > ACC_ROWS) {
            printf("not enough accumulator space to store outputs\n");
            exit(1);
        }
        if (kernel_dim <= padding) {
            printf("kernel_dim must be larger than padding\n");
            exit(1);
        }
    }
#endif

    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;

    if (no_pool) {
        gemmini_config_st(out_channels * sizeof(elem_t));
    }

    for (int b = 0; b < batch_size; b += batches) {
        for (int porow = 0; porow < pool_out_dim; porow += porows) {
            const int orow = porow * pool_stride - pool_padding;

            for (int pocol = 0; pocol < pool_out_dim; pocol += pocols) {
                const int ocol = pocol * pool_stride - pool_padding;

                for (int poch = 0; poch < out_channels; poch += pochs) {
	                const int orow_floored = orow < 0 ? 0 : orow;
                        const int irow = orow_floored * stride - padding;//+ krow - padding;
                        const int ocol_floored = ocol < 0 ? 0 : ocol;
                        const int icol = ocol_floored * stride - padding; //+ kcol - padding;
//			    printf("icol: %d, ocols: %d, ocol: %d, ocol_floored: %d, kcol: %d \n", icol, ocols, ocol, ocol_floored, kcol);

                            for (int kch = 0; kch < in_channels; kch += kchs) {
                                elem_t * out = output + (b*pool_out_dim*pool_out_dim + porow*pool_out_dim + pocol) * out_channels + poch;
                                if (kch + kchs < in_channels) {
                                    out = NULL;
                                }
                                acc_t * bias_ = bias + poch;
                                if (kch > 0) {
                                    bias_ = NULL;
                                }

                                const int batches_ = batch_size - b > batches ? batches : batch_size - b;
                                const int porows_ = pool_out_dim - porow > porows ? porows : pool_out_dim - porow;
                                const int pocols_ = pool_out_dim - pocol > pocols ? pocols : pool_out_dim - pocol;
                                const int pochs_ = out_channels - poch > pochs ? pochs : out_channels - poch;
			        const int kchs_ = in_channels - kch > kchs ? kchs : in_channels - kch;

                                const int ocols_ = pocols_ * pool_stride + pool_size - 1;
                                const int orows_ = porows_ * pool_stride + pool_size - 1;

                                const int plpad = ocol < 0 ? -ocol : 0;
                                const int prpad = ocol + ocols_ > out_dim ? ocol + ocols_ - out_dim : 0;
                                const int pupad = orow < 0 ? -orow : 0;
                                const int pdpad = orow + orows_ > out_dim ? orow + orows_ - out_dim : 0;

				const int icols_ = (ocols_ - 1 - plpad - prpad) * stride + kcols;//+ kcols_;
                                const int irows_ = (orows_ - 1 - pupad - pdpad) * stride + kcols;//krows_;

                                const int lpad = icol < 0 ? -icol : 0;
                                const int rpad = icol + icols_ > in_dim ? icol + icols_ - in_dim : 0;
                                const int upad = irow < 0 ? -irow : 0;
                                const int dpad = irow + irows_ > in_dim ? irow + irows_ - in_dim : 0;

				if(kernel_dim != 1)
                                  sp_tiled_conv_ws_original(
                                    batch_size, in_dim, in_channels,
                                    out_channels, out_dim, pool_out_dim,

                                    stride, padding, kernel_dim,

                                    pool_size, pool_stride, pool_padding,

                                    batches_,
                                    porows_, pocols_, pochs_,
			            kcols, kchs_,

                                    lpad, rpad, upad, dpad,
                                    plpad, prpad, pupad, pdpad,

                                    input + (b*in_dim*in_dim + (irow+upad)*in_dim + (icol+lpad)) * in_channels + kch,
				    weights + kch * out_channels + poch,
		 		    //weights + (krow*kernel_dim*in_channels + kcol*in_channels + kch) * out_channels + poch,
                                    out,
                                    bias_,

                                    act, scale, relu6_shift,
                                    no_bias, no_pool,
                                    weight_bank);
			  
				else //downsampling layer
                                  sp_tiled_conv_ds(
                                    batch_size, in_dim, in_channels,
                                    out_channels, out_dim, pool_out_dim,

                                    stride, pool_size, pool_stride, pool_padding,

                                    batches_,
                                    porows_, pocols_, pochs_,
			            kchs_,

                                    lpad, rpad, upad, dpad,
                                    plpad, prpad, pupad, pdpad,

                                    input + (b*in_dim*in_dim + (irow+upad)*in_dim + (icol+lpad)) * in_channels + kch,
				    0,	
				    weights + kch * out_channels + poch,
		 		    //weights + (krow*kernel_dim*in_channels + kcol*in_channels + kch) * out_channels + poch,
                                    out,
                                    bias_,

                                    act, scale, relu6_shift,
                                    no_bias, no_pool, 
                                    weight_bank);
                           }
                        }
                    }
	       }
    }
}


void tiled_conv(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        int batches,
        int porows, int pocols, int pochs,
        int krows, int kcols, int kchs,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,

        int weight_bank, enum tiled_matmul_type_t tiled_conv_type) {

    if (tiled_conv_type == CPU) {
      if (pool_size == 1 && pool_stride == 1 && pool_padding == 0) {
        pool_stride = 0;
      }

      conv_cpu(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding, kernel_dim,
        input, weights, bias, output,
        act, scale, relu6_shift,
        pool_size, pool_stride, pool_padding);
      return;
    } else if (tiled_conv_type == OS) {
      printf("Gemmini convs do not currently support OS\n");
      exit(1);
    }

    // TODO move everything below this into a tiled_conv_outer function to match the tiled_matmul function

    bool no_bias = false;
    if (bias == NULL) {
        bias = (acc_t*)1;
        no_bias = true;
    }

    bool no_pool = pool_stride == 0;
    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }



#ifdef GEMMINI_ASSERTIONS
    {
        // Check that data will fit in scratchpad
        const int spad_rows_weight = tiled_conv_total_spad_rows(false, true,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);
        const int spad_rows_input = tiled_conv_total_spad_rows(false, false,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);
        const int acc_rows = tiled_conv_total_spad_rows(true, false,
            stride, batches, porows, pocols, pochs, kcols, kcols, kchs, pool_size, pool_stride);

	if (spad_rows_weight > BANK_ROWS * weight_bank) {
            printf("not enough scratchpad space to store weights\n");
            exit(1);
        }
        if (spad_rows_input > BANK_ROWS*(BANK_NUM - weight_bank)) {
            printf("not enough scratchpad space to store inputs\n");
            exit(1);
        }
        if (acc_rows > ACC_ROWS) {
            printf("not enough accumulator space to store outputs\n");
            exit(1);
        }
    }
#endif
    int kdims = kcols*kcols;
    const uint32_t B_sp_addr_start = (BANK_NUM - weight_bank) * BANK_ROWS;
 
    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;

      for (int poch = 0; poch < out_channels; poch += pochs) {
           const int pochs_ = out_channels - poch > pochs ? pochs : out_channels - poch;
                acc_t * bias_ = bias + poch;

		const int kchs_ = in_channels;
		gemmini_config_ld(out_channels*sizeof(elem_t));
		//mvin weight on the outer loop
		//printf("mvin weight\n");
		  for (int och = 0; och < pochs_; och += DIM) {
        		const int J = pochs_ - och > DIM ? DIM : pochs_ - och;
      			for (int ich = 0; ich < kchs_; ich += DIM) {
        		    const int K = kchs_ - ich > DIM ? DIM : kchs_ - ich;
        		    for (int krow = 0; krow < kcols; krow++){
				elem_t * weight = weights + poch + (krow*kcols*in_channels + ich) * out_channels + och;
				const uint32_t B_sp_addr = B_sp_addr_start + (och / DIM) * kdims * kchs_ + ich*kdims + krow*kcols*K;// + kcol*K;
            			for (int kcol = 0; kcol < kcols; kcol++){
					gemmini_extended_mvin(weight + kcol*in_channels*out_channels,
						B_sp_addr+kcol*K,
                        			J, K);
	    			}
       			    }
			}
    		  }	
		  for (int b = 0; b < batch_size; b += batches) {
		        for (int porow = 0; porow < pool_out_dim; porow += porows) {
		            const int orow = porow * pool_stride - pool_padding;
            		    for (int pocol = 0; pocol < pool_out_dim; pocol += pocols) {
				elem_t * out = output + (b*pool_out_dim*pool_out_dim + porow*pool_out_dim + pocol) * out_channels + poch;
		                const int ocol = pocol * pool_stride - pool_padding;
                                const int batches_ = batch_size - b > batches ? batches : batch_size - b;
                                const int porows_ = pool_out_dim - porow > porows ? porows : pool_out_dim - porow;
                                const int pocols_ = pool_out_dim - pocol > pocols ? pocols : pool_out_dim - pocol;
			        const int orow_floored = orow < 0 ? 0 : orow;
            			const int irow = orow_floored * stride - padding;//+ krow - padding;
            			const int ocol_floored = ocol < 0 ? 0 : ocol;
            			const int icol = ocol_floored * stride - padding; //+ kcol - padding;
 
                                const int ocols_ = pocols_ * pool_stride + pool_size - 1;
                                const int orows_ = porows_ * pool_stride + pool_size - 1;

                                const int plpad = ocol < 0 ? -ocol : 0;
                                const int prpad = ocol + ocols_ > out_dim ? ocol + ocols_ - out_dim : 0;
                                const int pupad = orow < 0 ? -orow : 0;
                                const int pdpad = orow + orows_ > out_dim ? orow + orows_ - out_dim : 0;

     				const int icols_ = (ocols_ - 1 - plpad - prpad) * stride + kcols;//+ kcols_;
                                const int irows_ = (orows_ - 1 - pupad - pdpad) * stride + kcols;//krows_;

                                const int lpad = icol < 0 ? -icol : 0;
                                const int rpad = icol + icols_ > in_dim ? icol + icols_ - in_dim : 0;
                                const int upad = irow < 0 ? -irow : 0;
                                const int dpad = irow + irows_ > in_dim ? irow + irows_ - in_dim : 0;
/*				printf("ocols_: %d \n", ocols_);
				printf("orows_: %d \n", orows_);
      				printf("icols_: %d \n", icols_);
				printf("irows_: %d \n", irows_);
				printf("kchs_: %d \n", kchs_);
				printf("kch: %d \n", kch);

                                 printf("upad: %d\n", upad);
                                 printf("dpad: %d\n", dpad);
                                 printf("lpad: %d\n", lpad);
                                 printf("rpad: %d\n", rpad);
                                 printf("pupad: %d\n", pupad);
                                 printf("pdpad: %d\n", pdpad);
                                 printf("plpad: %d\n", plpad);
                                 printf("prpad: %d\n", prpad);
*/
			if(kcols != 1)
                               sp_tiled_conv_ws(
                                    batch_size, in_dim, in_channels,
                                    out_channels, out_dim, pool_out_dim,

                                    stride, padding,// kernel_dim,

                                    pool_size, pool_stride, pool_padding,

                                    batches_,
                                    porows_, pocols_, pochs_,
                                    kcols, kchs_,

                                    lpad, rpad, upad, dpad,
                                    plpad, prpad, pupad, pdpad,

                                    input + (b*in_dim*in_dim + (irow+upad)*in_dim + (icol+lpad)) * in_channels,// + kch,
				    B_sp_addr_start,
				   // weights + kch * out_channels + poch,
		 		    out,
                                    bias_,

                                    act, scale, relu6_shift,
                                    no_bias, no_pool,
                                    weight_bank);
				else
                               sp_tiled_conv_ds(
                                    batch_size, in_dim, in_channels,
                                    out_channels, out_dim, pool_out_dim,

                                    stride, //padding, kernel_dim,

                                    pool_size, pool_stride, pool_padding,

                                    batches_,
                                    porows_, pocols_, pochs_,
                                    kchs_,

                                    lpad, rpad, upad, dpad,
                                    plpad, prpad, pupad, pdpad,

                                    input + (b*in_dim*in_dim + (irow+upad)*in_dim + (icol+lpad)) * in_channels, //+ kch,
                                    B_sp_addr_start,
                                    NULL, //weights + kch * out_channels + poch,
                                    out,
                                    bias_,

                                    act, scale, relu6_shift,
                                    no_bias, no_pool,
                                    weight_bank);

                            }
                        }
                    }
//	       }
    }
//	printf("mvin total cycles %d \n", mvin_cycles);
}

void tiled_conv_auto_first(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,

	enum tiled_matmul_type_t tiled_conv_type) {
   int weight_bank = 1;

   const bool no_pool = pool_stride == 0 || (pool_stride == 1 && pool_size == 1 && pool_padding == 0);
//    const bool no_1d = pool_stride == 0 && pool_size == 0;
    const bool no_1d =false;// no_pool; //Todo: change to 1d

    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }
    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;

    // int args[] = {batch_size, porows, pocols, pochs, krows, kcols, kchs};
    int args[] = {batch_size, pool_out_dim, pool_out_dim, out_channels, in_channels};

    int acc_rows = tiled_conv_total_spad_rows(true, false,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
//   printf("batch: %d, out_dim: %d, out_channel: %d, in_channel: %d \n", args[0], args[1], args[3], args[4]);

    int och_floor = (args[3]/DIM) + 1;
    while(acc_rows > ACC_ROWS){ //batch output channel, output dimension affects
		if(args[1] != 1){
			args[1]--;
			args[2]--;
		}else{
			if(args[3] >= args[0]){
				och_floor = och_floor - 1;
				args[3] = och_floor * DIM;
			}
			else args[0]--;
		}
	acc_rows = tiled_conv_total_spad_rows(true, false, stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
    }
//	printf("batch: %d, out_dim: %d, out_channel: %d, in_channel: %d \n", args[0], args[1], args[3], args[4]);

    int spad_rows_weight = tiled_conv_total_spad_rows(false, true,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
    while(spad_rows_weight > BANK_ROWS * weight_bank){ //tile weight first (allocate bank3 to weight)
	//input channel, output channel
	och_floor = och_floor -	1;
	args[3] = och_floor * DIM;
	spad_rows_weight = tiled_conv_total_spad_rows(false, true,
        	stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
    }

    int batches = args[0];
    int orows = args[1];
    int ocols = args[2];
    int ochs = args[3];
    int kchs = args[4];

/*
     printf("batches = %d\n", batches);
     printf("orows = %d\n", orows);
     printf("ocols = %d\n", ocols);
     printf("ochs = %d\n", ochs);
//     printf("kcols = %d\n", kernel_dim);
     printf("kchs = %d\n", kchs);
*/

   tiled_conv_first(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding,// kernel_dim,

        batches,
        orows, ocols, ochs,
	kernel_dim, kchs,

        input,
        weights,
        bias,
        output,

        act, scale, relu6_shift,
        no_1d ? 0 : pool_size, no_pool ? 0 : pool_stride, pool_padding,

	tiled_conv_type,
	weight_bank);


	gemmini_fence();
}

//for mobilenet depthwise conv
void tiled_conv_auto_dw(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,

        enum tiled_matmul_type_t tiled_conv_type) {

    const bool no_pool = pool_stride == 0;
    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }

    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;

    const int weight_bank = 1;//((int)(kernel_dim*kernel_dim*in_channels)/BANK_ROWS)+1;
    // int args[] = {batch_size, porows, pocols, pochs, krows, kcols, kchs};
    int args[] = {batch_size, pool_out_dim, pool_out_dim, 1, 1}; //out_channel, in_channel to 1

    int acc_rows = tiled_conv_total_spad_rows(true, false,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
//   printf("batch: %d, out_dim: %d, out_channel: %d, in_channel: %d \n", args[0], args[1], args[3], args[4]);

    int och_floor = (args[3]/DIM) + 1;
    while(acc_rows > ACC_ROWS){ //batch output channel, output dimension affects
 //tile output dimension
		if(args[1] != 1){
			args[1]--;
			args[2]--;
		}else{
			args[0]--;	
		}

	acc_rows = tiled_conv_total_spad_rows(true, false, stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);	
    }

    int batches = args[0];
    int orows = args[1];
    int ocols = args[2];
    int ochs = args[3];
    int krows = kernel_dim;//args[4];
    int kcols = kernel_dim;//args[5];
    int kchs = args[4];

/*
     printf("batches = %d\n", batches);
     printf("orows = %d\n", orows);
     printf("ocols = %d\n", ocols);
     printf("ochs = %d\n", ochs);
     printf("kcols = %d\n", kernel_dim);
     printf("kchs = %d\n", kchs);
*/
    tiled_conv_dw(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding, kernel_dim,

        batches,
        orows, ocols,// ochs,
//        krows, kcols, kchs,

        input,
        weights,
        bias,
        output,

        act, scale, relu6_shift,
        pool_size, no_pool ? 0 : pool_stride, pool_padding,
        
        tiled_conv_type);
}

//for resnet deeper layers
//when we need to tile input channel dimension
void tiled_conv_auto_original(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,
	
	enum tiled_matmul_type_t tiled_conv_type) {

   const int weight_bank = in_channels > 500? 3 : 2;
   const bool no_pool = pool_stride == 0 || (pool_stride == 1 && pool_size == 1 && pool_padding == 0);
    const bool no_1d = no_pool; //Todo: change to 1d
 
    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }
    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;

    int args[] = {batch_size, pool_out_dim, pool_out_dim, out_channels, in_channels};

    int acc_rows = tiled_conv_total_spad_rows(true, false,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);

    int och_floor = (args[3]/DIM) + 1;
    int kch_floor = (args[4]/DIM) + 1;

    int spad_rows_weight = tiled_conv_total_spad_rows(false, true,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
    while(spad_rows_weight > weight_bank * BANK_ROWS){ //tile weight first (allocate bank3 to weight)
		if(kch_floor > och_floor){
			kch_floor--;
			args[4] = kch_floor * DIM;
		}else{
			och_floor--;
			args[3] = och_floor * DIM;
		}
		spad_rows_weight = tiled_conv_total_spad_rows(false, true,
        	stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
	}


       while(acc_rows > ACC_ROWS){ //batch output channel, output dimension affects
	   if(args[1] <= 7){
		if(args[0] > 1){
			args[0]--;
		}
		else{
			och_floor--;
			args[3] = och_floor*DIM;
		}
	   }
	   else{

		int max_val = -1;
		int max_idx = -1;
		if(args[0]*2 < args[1]){
			args[1]--;
			args[2]--;
		}
		else args[0]--;
	   }
	acc_rows = tiled_conv_total_spad_rows(true, false, stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);	
    }
//	printf("batch: %d, out_dim: %d, out_channel: %d, in_channel: %d \n", args[0], args[1], args[3], args[4]);
    int spad_rows_input = tiled_conv_total_spad_rows(false, false,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
 //   printf("batch: %d, out_dim: %d, out_channel: %d, in_channel: %d \n", args[0], args[1], args[3], args[4]);


    while(spad_rows_input > BANK_ROWS*(BANK_NUM-weight_bank)){// tile input last
	//batch, input dimension, input channel	
		int max_val = -1;
		int max_idx = -1;
		for(int i = 0; i < 5; i++){
			if(args[i] > max_val && i != 3){
				if(i!=4){
					max_val = args[i];
					max_idx = i;
				}else if(kch_floor > 1){
					max_val = args[4];
					max_idx = 4;
				}
			}
		}
		if(max_idx == 4){
			kch_floor = kch_floor -1;
			args[4] = kch_floor * DIM;
		}
		else  args[max_idx]--;

		spad_rows_input = tiled_conv_total_spad_rows(false, false, stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);	
    }


    int batches = args[0];
    int orows = args[1];
    int ocols = args[2];
    int ochs = args[3];
    int kchs = args[4];


    /*
    printf("batches = %d\n", batches);
    printf("orows = %d\n", orows);
    printf("ocols = %d\n", ocols);
    printf("ochs = %d\n", ochs);
    printf("kcols = %d\n", kernel_dim);
    printf("kchs = %d\n", kchs);
    */

    tiled_conv_original(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding, kernel_dim,

        batches,
        orows, ocols, ochs,
	kernel_dim, kchs,

        input,
        weights,
        bias,
        output,

        act, scale, relu6_shift,
        no_1d ? 0 : pool_size, no_pool ? 0 : pool_stride, pool_padding,

	tiled_conv_type, weight_bank);
	gemmini_fence();
}


//tiling function for deeper layers (when C is large)
void tiled_conv_auto_largeC(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,

        enum tiled_matmul_type_t tiled_conv_type) {

    const bool no_pool = pool_stride == 0;
    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }
    const int weight_bank = 2; //hard-coded number of weight banks to use

    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;

    // int args[] = {batch_size, porows, pocols, pochs, krows, kcols, kchs};
    int args[] = {batch_size, pool_out_dim, pool_out_dim, out_channels, in_channels};

    int acc_rows = tiled_conv_total_spad_rows(true, false,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
//   printf("batch: %d, out_dim: %d, out_channel: %d, in_channel: %d \n", args[0], args[1], args[3], args[4]);


    int och_floor = (args[3]/DIM) + 1;
    int kch_floor = (args[4]/DIM) + 1;

 int spad_rows_weight = tiled_conv_total_spad_rows(false, true,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
    while(spad_rows_weight > weight_bank * BANK_ROWS){ //tile weight first (allocate bank3 to weight)
	//input channel, output channel
	och_floor--;
	args[3] = och_floor * DIM;
	spad_rows_weight = tiled_conv_total_spad_rows(false, true,
       	stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
    }


       while(acc_rows > ACC_ROWS){ //batch output channel, output dimension affects
		int max_val = -1;
		int max_idx = -1;
		for(int i = 0; i < 3; i++){
			if(args[i] > max_val){
				max_val = args[i];
				max_idx = i;
			}
		}

		args[max_idx]--;
	acc_rows = tiled_conv_total_spad_rows(true, false, stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);	
    }
//	printf("batch: %d, out_dim: %d, out_channel: %d, in_channel: %d \n", args[0], args[1], args[3], args[4]);
    int spad_rows_input = tiled_conv_total_spad_rows(false, false,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);

    while(spad_rows_input > BANK_ROWS*(BANK_NUM-weight_bank)){// tile input last
	//batch, input dimension	
		int max_val = -1;
		int max_idx = -1;
		for(int i = 0; i < 3; i++){
			if(args[i] > max_val){
				max_val = args[i];
				max_idx = i;
			}
		}
		args[max_idx]--;

	spad_rows_input = tiled_conv_total_spad_rows(false, false, stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);	
    }

    int batches = args[0];
    int orows = args[1];
    int ocols = args[2];
    int ochs = args[3];
    int kchs = args[4];
    int krows = kernel_dim;
    int kcols = kernel_dim;
/*
     printf("batches = %d\n", batches);
     printf("orows = %d\n", orows);
     printf("ocols = %d\n", ocols);
     printf("ochs = %d\n", ochs);
     printf("kcols = %d\n", kernel_dim);
     printf("kchs = %d\n", kchs);
*/


    tiled_conv(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding, kernel_dim,

        batches,
        orows, ocols, ochs,
        krows, kcols, kchs,

        input,
        weights,
        bias,
        output,

        act, scale, relu6_shift,
        pool_size, no_pool ? 0 : pool_stride, pool_padding,
        weight_bank, tiled_conv_type);
}

void tiled_conv_auto(
        int batch_size, int in_dim, int in_channels,
        int out_channels, int out_dim,
        int stride, int padding, int kernel_dim,

        elem_t * input,
        elem_t * weights,
        acc_t * bias,
        elem_t * output,

        int act, acc_scale_t scale, size_t relu6_shift,
        int pool_size, int pool_stride, int pool_padding,

        enum tiled_matmul_type_t tiled_conv_type) {

    const bool no_pool = pool_stride == 0;
    if (no_pool) {
        pool_size = 1;
        pool_stride = 1;
        pool_padding = 0;
    }

    const int pool_out_dim = (out_dim + 2*pool_padding - pool_size) / pool_stride + 1;
    const int weight_bank = 1;
    int args[] = {batch_size, pool_out_dim, pool_out_dim, out_channels, in_channels};

    int och_floor = (args[3]/DIM) + 1;
 
    int spad_rows_weight = tiled_conv_total_spad_rows(false, true,
        stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
    while(spad_rows_weight > BANK_ROWS * weight_bank){ //tile weight first (allocate bank3 to weight)
	//input channel, output channel
	och_floor = och_floor -	1;
	args[3] = och_floor * DIM;
	spad_rows_weight = tiled_conv_total_spad_rows(false, true,
        	stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);
    }
    
    int acc_rows = tiled_conv_total_spad_rows(true, false,
		stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);

    while(acc_rows > ACC_ROWS){ //batch output channel, output dimension affects
 //tile output dimension
	args[1]--;
	args[2]--;

	acc_rows = tiled_conv_total_spad_rows(true, false, stride, args[0], args[1], args[2], args[3], kernel_dim, kernel_dim, args[4], pool_size, pool_stride);	
    }
//	printf("batch: %d, out_dim: %d, out_channel: %d, in_channel: %d \n", args[0], args[1], args[3], args[4]);

    int batches = args[0];
    int orows = args[1];
    int ocols = args[2];
    int ochs = args[3];
    int krows = kernel_dim;//args[4];
    int kcols = kernel_dim;//args[5];
    int kchs = args[4];

/*
     printf("batches = %d\n", batches);
     printf("orows = %d\n", orows);
     printf("ocols = %d\n", ocols);
     printf("ochs = %d\n", ochs);
     printf("kcols = %d\n", kernel_dim);
     printf("kchs = %d\n", kchs);
*/
    tiled_conv(
        batch_size, in_dim, in_channels,
        out_channels, out_dim,
        stride, padding, kernel_dim,

        batches,
        orows, ocols, ochs,
        krows, kcols, kchs,

        input,
        weights,
        bias,
        output,

        act, scale, relu6_shift,
        pool_size, no_pool ? 0 : pool_stride, pool_padding,
        
        weight_bank, tiled_conv_type);
}

void resadd_cpu(const size_t I, const size_t J,
        const scale_t A_scale,
        const scale_t B_scale,
        const acc_scale_t C_scale,
        const elem_t * A,
        const elem_t * B,
        elem_t * C,
        bool relu) {

	const int minimum = relu ? 0 : elem_t_min;

    for (size_t i = 0; i < I; i++) {
        for (size_t j = 0; j < J; j++) {
            const elem_t * a = A + i * J + j;
            const elem_t * b = B + i * J + j;
            elem_t * c = C + i * J + j;

            acc_t result = MVIN_SCALE(*a, A_scale) + MVIN_SCALE(*b, B_scale);
            result = ACC_SCALE(result, C_scale);
            result = result > elem_t_max ? elem_t_max :
                (result < minimum ? minimum : result);

            *c = result;
        }
    }
}

void sp_tiled_resadd(const size_t I, const size_t J,
        const scale_t A_scale,
        const scale_t B_scale,
        const elem_t * A, const elem_t * B, elem_t * C,
        size_t A_row_stride, size_t B_row_stride, size_t C_row_stride,
        bool relu) {

    // Use the new mvin2 command to overlap mvin A, mvin B, and mvout C

    size_t blocks = J/DIM < MAX_BLOCK_LEN ? J/DIM : MAX_BLOCK_LEN;
    if (blocks == 0) blocks = 1;

    const uint32_t D_sp_addr_start = 1 << (ADDR_LEN-1);
    const uint32_t C_sp_addr_start = 3 << (ADDR_LEN-2);

    const size_t rounded_up_J = (J / DIM + (J % DIM != 0)) * DIM;

    // Mvin A
    // printf("Mving A\n");
    gemmini_extended2_config_ld(A_row_stride * sizeof(elem_t), A_scale, true);
    for (size_t i = 0; i < I; i += DIM) {
        for (size_t j = 0; j < J; j += blocks * DIM) {
            const size_t cols = j + blocks*DIM <= J ? blocks*DIM : J-j;
            const size_t rows = i + DIM <= I ? DIM : I-i;

            const elem_t * const A_dram_addr = A + i * A_row_stride + j;
            const uint32_t A_sp_addr = D_sp_addr_start + i * (rounded_up_J/DIM) + j;

            gemmini_extended_mvin(A_dram_addr, A_sp_addr, cols, rows);
        }
    }

    // Mvin B
    // printf("Mving B\n");
    gemmini_extended2_config_ld(B_row_stride * sizeof(elem_t), B_scale, true);
    for (size_t i = 0; i < I; i += DIM) {
        for (size_t j = 0; j < J; j += blocks * DIM) {
            const size_t cols = j + blocks*DIM <= J ? blocks*DIM : J-j;
            const size_t rows = i + DIM <= I ? DIM : I-i;

            const elem_t * const B_dram_addr = B + i * B_row_stride + j;
            const uint32_t B_sp_addr = C_sp_addr_start + i * (rounded_up_J/DIM) + j;
            gemmini_extended_mvin(B_dram_addr, B_sp_addr, cols, rows);
        }
    }

    // Mvout C from accumulator
    // printf("Mvout C from accumulator\n");
    for (size_t i = 0; i < I; i += DIM) {
        for (size_t j = 0; j < J; j += DIM) {
            const size_t cols = j + DIM <= J ? DIM : J-j;
            const size_t rows = i + DIM <= I ? DIM : I-i;

            elem_t * const C_dram_addr = C + i * C_row_stride + j;
            const uint32_t C_sp_addr = D_sp_addr_start + i * (rounded_up_J/DIM) + j;
            gemmini_extended_mvout(C_dram_addr, C_sp_addr, cols, rows);
        }
    }

}

// Compute MVIN_SCALE(A, A_scale) + MVIN_SCALE(B, B_scale) = C
void tiled_resadd(const size_t I, const size_t J,
        const size_t tile_I, const size_t tile_J,
        const scale_t A_scale,
        const scale_t B_scale,
        const acc_scale_t C_scale,
        const elem_t * A,
        const elem_t * B,
        elem_t * C,
        bool relu,
        enum tiled_matmul_type_t matadd_type) {

    gemmini_config_st(J * sizeof(elem_t));
    gemmini_config_ex(WS, relu ? RELU : NO_ACTIVATION, 0, C_scale, 0);

    for (size_t i = 0; i < I; i += tile_I) {
        for (size_t j = 0; j < J; j += tile_J) {
            const size_t I_tile = i + tile_I <= I ? tile_I : I - i;
            const size_t J_tile = j + tile_J <= J ? tile_J : J - j;

            const elem_t * a = A + i * J + j;
            const elem_t * b = B + i * J + j;
            elem_t * c = C + i * J + j;

            sp_tiled_resadd(I_tile, J_tile,
                    A_scale, B_scale, a, b, c,
                    J, J, J,
                    relu);
        }
    }

    gemmini_fence();
}

// Compute (A >> A_shift) + B = C
void tiled_resadd_auto(const size_t I, const size_t J,
        const scale_t A_scale,
        const scale_t B_scale,
        const acc_scale_t C_scale,
        const elem_t * A,
        const elem_t * B,
        elem_t * C,
        bool relu,
        enum tiled_matmul_type_t matadd_type) {

    if (matadd_type == CPU) {
        resadd_cpu(I, J,
            A_scale, B_scale, C_scale, A, B, C,
            relu);
        return;
    }

    size_t tile_I = I, tile_J = J;

    // size_t total_spad_rows = 2 * (tile_I / DIM + (tile_I % DIM != 0))*DIM * (tile_J / DIM + (tile_J % DIM != 0));
    size_t total_acc_rows = (tile_I / DIM + (tile_I % DIM != 0))*DIM * (tile_J / DIM + (tile_J % DIM != 0));

    // TODO this is a very inefficient way of doing this...
    while (total_acc_rows > ACC_ROWS) {
        if (tile_I > tile_J)
            tile_I--;
        else
            tile_J--;

        total_acc_rows = (tile_I / DIM + (tile_I % DIM != 0))*DIM * (tile_J / DIM + (tile_J % DIM != 0));
    }

    if (matadd_type == WS) {
      tiled_resadd(I, J, tile_I, tile_J,
            A_scale, B_scale, C_scale, A, B, C,
            relu, matadd_type);
    } else if(matadd_type == CPU){
	  resadd_cpu(I, J, A_scale, B_scale, C_scale,
		A, B, C, relu);
    }
    else {
      printf("Unsupported type\n");
      exit(1);
    }
}
#undef abs


//============================================================================
// pk/linux page-fault prevention mechanisms
// - YOU MUST PIN ALL MEMORY IN PK/LINUX BEFORE MVIN OR MVOUT!
// - gemmini cannot raise a page-fault, since there is currently no mechanism
//   to pass the requested vaddr to the rocket-chip from a rocc-accelerator
// - in the future, it might be worth implementing support in rocket-core and
//   gemmini for accelerator-initiated page-fault handling, but this seems
//   like a lot of work.
//============================================================================
static bool all_pinned = false;

#ifdef GEMMINI_LINUX
#include <sys/mman.h>
static inline void pin_all() {
  if(all_pinned) return;
  all_pinned = true;
  if (mlockall(MCL_CURRENT) != 0) {
    perror("mlockall failed");
    exit(1);
  }
}
static inline void unpin_all() {
  if(!all_pinned) return;
  all_pinned = false;
  if (munlockall()) {
    perror("munlockall failed");
    exit(1);
  }
}
#define pin_matrices(M,N,K,A,B,D,C,r) do {} while(0)
#define unpin_matrices() do {} while(0)
#else
#ifdef GEMMINI_PK
#define PAGESIZE 0x1000
#define pin_all() do {} while(0)
#define unpin_all() do {} while(0)
static inline void __pin_vector(const char*vec, size_t len) {
  volatile char item[4];
  size_t i;
  for(i=3*PAGESIZE; i<len; i+=4*PAGESIZE) {
    item[0] = vec[i-3*PAGESIZE];
    item[1] = vec[i-2*PAGESIZE];
    item[2] = vec[i-1*PAGESIZE];
    item[3] = vec[i-0*PAGESIZE];
  }
  if(i-3*PAGESIZE < len) item[0] = vec[i-3*PAGESIZE];
  if(i-2*PAGESIZE < len) item[1] = vec[i-2*PAGESIZE];
  if(i-1*PAGESIZE < len) item[2] = vec[i-1*PAGESIZE];
                         item[3] = vec[len-1];
}
static inline void pin_matrices(
  size_t M, size_t N, size_t K,
  const elem_t *A, const elem_t *B, const acc_t * D, elem_t *C,
  bool repeating_bias)
{
  // this is really inefficient, but we don't have mlockall() in newlib, so the
  // best we can do is just touch every page before the accelerator uses it
  const char* A_vec = (const char*)A;
  const char* B_vec = (const char*)B;
  const char* C_vec = (const char*)C;
  size_t A_len = sizeof(elem_t)*M*K;
  size_t B_len = sizeof(elem_t)*K*N;
  size_t C_len = sizeof(elem_t)*M*N;
  __pin_vector(A_vec, A_len);
  __pin_vector(B_vec, B_len);
  __pin_vector(C_vec, C_len);

  const char* D_vec = (const char*)D;
  size_t D_len = sizeof(acc_t)*(repeating_bias ? N : M*N);
  if(D != NULL)
    __pin_vector(D_vec, D_len);
}
#define unpin_matrices() do {} while(0)
#else
// GEMMINI_BAREMETAL
#define pin_all() do {} while(0)
#define unpin_all() do {} while(0)
#define pin_matrices(M,N,K,A,B,D,C,r) do {} while(0)
#define unpin_matrices() do {} while(0)
#endif // GEMMINI_PK
#endif // GEMMINI_LINUX

#endif // SRC_MAIN_C_GEMMINI_H

