#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "include/gemmini_testutils.h"

#ifndef GEMMINI_BAREMETAL

#define BATCH_SIZE 4
#define IN_DIM 224
#define IN_CHANNELS 3
#define OUT_CHANNELS 32
#define KERNEL_DIM 3
#define PADDING 1
#define STRIDE 2

#define POOL_SIZE 3
#define POOL_STRIDE 2
#define POOL_PADDING 1

#else

#ifdef FAST
#define IN_DIM 9
#define IN_CHANNELS 5
#define OUT_CHANNELS 7
#else
#define IN_DIM 23
#define IN_CHANNELS 17
#define OUT_CHANNELS 31
#endif

#define BATCH_SIZE 3
#define KERNEL_DIM 3
#define PADDING 1
#define STRIDE 2

#define POOL_SIZE 3
#define POOL_STRIDE 2
#define POOL_PADDING 1

#endif

#define NO_BIAS false

#define OUT_DIM ((IN_DIM + 2*PADDING - KERNEL_DIM) / STRIDE + 1)
#define PATCH_SIZE (KERNEL_DIM * KERNEL_DIM * IN_CHANNELS)
#define N_PATCHES (BATCH_SIZE * OUT_DIM * OUT_DIM)

#define POOL_OUT_DIM ((OUT_DIM + 2*POOL_PADDING - POOL_SIZE) / POOL_STRIDE + 1)

#define NO_POOL false

#if NO_POOL == true && !(POOL_SIZE == 1 && POOL_STRIDE == 1 && POOL_PADDING == 0)
#error NO_POOL is not set correctly
#endif

void conv(int batch_size, int in_channels, int in_dim,
        int out_channels, int kernel_dim,
        int out_dim,
        int stride, int padding,
        elem_t input[batch_size][in_dim][in_dim][in_channels],
        elem_t weights[out_channels][kernel_dim][kernel_dim][in_channels],
        acc_t bias[out_channels],
        elem_t output[batch_size][out_dim][out_dim][out_channels]) {

#ifdef GEMMINI_ASSERTIONS
    if (out_dim != (in_dim + 2*padding - kernel_dim) / stride + 1) {
        printf("conv out_dim is not correct\n");
        exit(1);
    }
#endif

    for (int b = 0; b < batch_size; b++) {
        for (int orow = 0; orow < out_dim; orow++) {
            for (int ocol = 0; ocol < out_dim; ocol++) {
                for (int och = 0; och < out_channels; och++) {
                    acc_t result = bias[och];

                    for (int krow = 0; krow < kernel_dim; krow++) {
                        for (int kcol = 0; kcol < kernel_dim; kcol++) {
                            for (int kch = 0; kch < in_channels; kch++) {
                                int irow = orow * stride + krow - padding;
                                int icol = ocol * stride + kcol - padding;

                                elem_t pixel = irow < 0 || irow >= in_dim ||
                                    icol < 0 || icol >= in_dim ?
                                    0 : input[b][irow][icol][kch];

                                result +=
                                    weights[och][krow][kcol][kch] *
                                    pixel;
                            }
                        }
                    }

                    // Clip result
                    result = result > elem_t_max ? elem_t_max : (result < elem_t_min ? elem_t_min : result);

                    output[b][orow][ocol][och] = result;
                }
            }
        }
    }
}

void cpu_tiled_conv_inner(
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

        bool no_bias) {

    const int orows = porows * pool_stride + pool_size - 1 - pupad - pdpad;
    const int ocols = pocols * pool_stride + pool_size - 1 - plpad - prpad;
    const int ochs = pochs;

    const int irows = orows * stride + krows - 1 - upad - dpad;
    const int icols = ocols * stride + kcols - 1 - lpad - rpad;

    elem_t output_buffer[batches*orows*ocols][ochs];

    // Perform conv
    for (int b = 0; b < batches; b++) {
        for (int orow = 0; orow < orows; orow++) {
            for (int ocol = 0; ocol < ocols; ocol++) {
                for (int och = 0; och < ochs; och++) {
                    output_buffer[b * orows * ocols + orow * ocols + ocol][och] = bias[och];

                    for (int krow = 0; krow < krows; krow++) {
                        for (int kcol = 0; kcol < kcols; kcol++) {
                            for (int kch = 0; kch < kchs; kch++) {
                                int irow = orow * stride + krow - upad;
                                int icol = ocol * stride + kcol - lpad;

                                elem_t pixel = irow < 0 || irow >= irows ||
                                    icol < 0 || icol >= icols ?
                                    0 : *(input + (b * in_dim * in_dim + irow * in_dim + icol) * in_channels + kch);

                                elem_t weight = *(weights + (krow * kernel_dim * in_channels + kcol * in_channels + kch) * out_channels + och);

                                output_buffer[b * orows * ocols + orow * ocols + ocol][och] += pixel * weight;
                            }
                        }
                    }
                }
            }
        }
    }

    ASSERT(orows+pupad+pdpad >= pool_size && ocols+plpad+prpad >= pool_size, 
        "pool_size");
    ASSERT(krows == kernel_dim && kcols == kernel_dim && kchs == in_channels,
        "kernel_dim");

    // Pool output_buffer into output
    // Arguments necessary:
    // * pool_stride
    // * pool_size
    // * pool_out_dim
    // * out_channels (this can just be stride)
    // * porows
    // * pocols
    // * pochs (inner) (this can probably just be len in mvout)
    // * output + (b * pool_out_dim + pool_out_dim) * out_channels + poch_outer (this can just be the vaddr in mvout)
    // * orows
    // * ocols
    // * pupad
    // * plpad
    for (int b = 0; b < batches; b++) {
        for (int poch_outer = 0; poch_outer < pochs; poch_outer += DIM) {
            const int channels = poch_outer + DIM >= pochs ? pochs - poch_outer : DIM;

            elem_t * pout_outer = output + (b * pool_out_dim * pool_out_dim)*out_channels + poch_outer;

            // Everything below this will run in hardware
            for (int porow = 0; porow < porows; porow++) {
                for (int pocol = 0; pocol < pocols; pocol++) {
                    for (int poch = 0; poch < channels; poch++) {
                        elem_t * pout = pout_outer + (porow * pool_out_dim + pocol)*out_channels + poch;

                        *pout = elem_t_min;

                        for (int wrow = 0; wrow < pool_size; wrow++) {
                            for (int wcol = 0; wcol < pool_size; wcol++) {
                                int orow = porow * pool_stride + wrow - pupad;
                                int ocol = pocol * pool_stride + wcol - plpad;

                                elem_t pixel = orow < 0 || orow >= orows || ocol < 0 || ocol >= ocols ?
                                    0 :
                                    output_buffer[b * orows * ocols + orow * ocols + ocol][poch];

                                if (pixel > *pout) {
                                    *pout = pixel;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void pool(int batch_size, int channels, int in_dim, int out_dim,
        int window_dim, int stride, int padding,
        elem_t input[batch_size][in_dim][in_dim][channels],
        elem_t output[batch_size][out_dim][out_dim][channels]) {

    for (int b = 0; b < batch_size; b++) {
        for (int orow = 0; orow < out_dim; orow++) {
            for (int ocol = 0; ocol < out_dim; ocol++) {
                for (int ch = 0; ch < channels; ch++) {
                    output[b][orow][ocol][ch] = elem_t_min;

                    for (int wrow = 0; wrow < window_dim; wrow++) {
                        for (int wcol = 0; wcol < window_dim; wcol++) {
                            int irow = orow * stride + wrow - padding;
                            int icol = ocol * stride + wcol - padding;

                            elem_t pixel = irow < 0 || irow >= in_dim ||
                                icol < 0 || icol >= in_dim ?
                                0 : input[b][irow][icol][ch];

                            if (pixel > output[b][orow][ocol][ch]) {
                                output[b][orow][ocol][ch] = pixel;
                            }
                        }
                    }
                }
            }
        }
    }
}

void flatten_weights(int out_channels, int kernel_dim, int in_channels,
        int patch_size,
        elem_t weights[out_channels][kernel_dim][kernel_dim][in_channels],
        elem_t weights_mat[patch_size][out_channels]) {

    ASSERT(patch_size == kernel_dim * kernel_dim * in_channels, "patch_size");

    for (int outc = 0; outc < out_channels; outc++) {
        for (int krow = 0; krow < kernel_dim; krow++) {
            for (int kcol = 0; kcol < kernel_dim; kcol++) {
                for (int inc = 0; inc < in_channels; inc++) {
                    int wmatrow = krow * kernel_dim * in_channels +
                        kcol * in_channels +
                        inc;

                    weights_mat[wmatrow][outc] =
                        weights[outc][krow][kcol][inc];
                }
            }
        }
    }
}

bool vec_is_equal(elem_t * a, elem_t * b, int len) {
    for (int i = 0; i < len; i++)
        if (a[i] != b[i])
            return false;
    return true;
}

void init_random(elem_t * buf, int len) {
    elem_t i = 0;
    for (elem_t * ptr = buf; ptr < buf + len; ptr++) {
        // *ptr = (rand() % 32) - 16;
#ifdef FAST
      *ptr = 1;
#else
        *ptr = (rand() % 5) - 2;
#endif
    }
}

void init_random_acc(acc_t * buf, int len) {
    elem_t i = 0;
    for (acc_t * ptr = buf; ptr < buf + len; ptr++) {
        // *ptr = (rand() % 32) - 16;
#ifdef FAST
      *ptr = 1;
#else
        *ptr = (rand() % 5) - 2;
#endif
    }
}

void init_zeros_acc(acc_t * buf, int len) {
    for (acc_t * ptr = buf; ptr < buf + len; ptr++) {
        *ptr = 0;
    }
}

int main() {
    pin_all();
    gemmini_flush(0);

    // assert((IN_DIM + 2*PADDING - KERNEL_DIM) % STRIDE == 0);
    // assert((OUT_DIM + 2*PADDING - POOL_SIZE) % POOL_STRIDE == 0);

    printf("Output dimension: %u\n", OUT_DIM);
    printf("Pooling output dimension: %u\n\n", POOL_OUT_DIM);

    static elem_t input[BATCH_SIZE][IN_DIM][IN_DIM][IN_CHANNELS];
    static elem_t weights[OUT_CHANNELS][KERNEL_DIM][KERNEL_DIM][IN_CHANNELS];
    static acc_t bias[OUT_CHANNELS];
    static elem_t output[BATCH_SIZE][OUT_DIM][OUT_DIM][OUT_CHANNELS];
    static elem_t pool_output[BATCH_SIZE][POOL_OUT_DIM][POOL_OUT_DIM][OUT_CHANNELS];

    printf("Randomize inputs...\n");
    init_random(&input[0][0][0][0], sizeof(input) / sizeof(elem_t));

    printf("Randomize weights...\n");
    init_random(&weights[0][0][0][0], sizeof(weights) / sizeof(elem_t));

    printf("Randomize bias...\n");
    if (NO_BIAS)
        init_zeros_acc(&bias[0], sizeof(bias) / sizeof(acc_t));
    else
        init_random_acc(&bias[0], sizeof(bias) / sizeof(acc_t));

#ifndef FAST
    printf("CPU conv...\n");
    uint64_t start_cpu = read_cycles();
    conv(BATCH_SIZE, IN_CHANNELS, IN_DIM,
            OUT_CHANNELS, KERNEL_DIM,
            OUT_DIM,
            STRIDE, PADDING,
            input,
            weights,
            bias,
            output);
    uint64_t end_cpu = read_cycles();
    printf("CPU conv took %llu cycles\n", end_cpu - start_cpu);

    printf("CPU pool...\n");
    uint64_t start_cpu_pool = read_cycles();
    pool(BATCH_SIZE, OUT_CHANNELS, OUT_DIM, POOL_OUT_DIM,
            POOL_SIZE, POOL_STRIDE, POOL_PADDING,
            output,
            pool_output);
    uint64_t end_cpu_pool = read_cycles();
    printf("CPU pool took %llu cycles\n", end_cpu_pool - start_cpu_pool);

    printf("CPU conv+pool took %llu cycles\n", end_cpu_pool - start_cpu_pool + end_cpu - start_cpu);
#endif

    static elem_t weights_mat[PATCH_SIZE][OUT_CHANNELS];
    static elem_t output_mat[N_PATCHES][OUT_CHANNELS];
    static elem_t pool_output_mat[BATCH_SIZE*POOL_OUT_DIM*POOL_OUT_DIM][OUT_CHANNELS];

    printf("Flatten weights...\n");
    flatten_weights(OUT_CHANNELS, KERNEL_DIM, IN_CHANNELS,
            PATCH_SIZE,
            weights,
            weights_mat);

    printf("Gemmini conv...\n");
    uint64_t start_gemmini = read_cycles();

    tiled_conv_auto(
    // tiled_conv(
        BATCH_SIZE, IN_DIM, IN_CHANNELS,
        OUT_CHANNELS, OUT_DIM,
        STRIDE, PADDING, KERNEL_DIM,

        // 1,
        // 1, 1, 1,
        // 1, 1, 1,

        (elem_t*)input,
        (elem_t*)weights_mat,
        NO_BIAS ? NULL : (acc_t*)bias,
        (elem_t*)pool_output_mat,

        NO_ACTIVATION, ACC_SCALE_IDENTITY, 0,
        POOL_SIZE, NO_POOL ? 0 : POOL_STRIDE, POOL_PADDING,

        WS);
    uint64_t end_gemmini = read_cycles();
    printf("Gemmini conv took %llu cycles\n", end_gemmini - start_gemmini);

    ASSERT(sizeof(pool_output_mat) == sizeof(pool_output), "pool_output_mat");

#ifdef FAST
    bool success = true;
    for (int orow = 0; orow < BATCH_SIZE * POOL_OUT_DIM * POOL_OUT_DIM; orow++) {
      for (int ocol = 0; ocol < OUT_CHANNELS; ocol++) {
	if (pool_output_mat[orow][ocol] != 46) {
	  success = false;
	  break;
	}
      }
    }
#else
    bool success = vec_is_equal(&pool_output[0][0][0][0], &pool_output_mat[0][0], sizeof(pool_output) / sizeof(elem_t));
#endif

    if (!success) {
        // return 1;

        printf("bias:\n");
        for (int och = 0; och < OUT_CHANNELS; och++) {
            printf("%d,", bias[och]);
        }
        printf("\b\n\n");

        printf("weights:\n");
        for (int och = 0; och < OUT_CHANNELS; och++) {
            printf("[");
            for (int wrow = 0; wrow < KERNEL_DIM; wrow++) {
                printf("[");
                for (int wcol = 0; wcol < KERNEL_DIM; wcol++) {
                    printf("[");
                    for (int ich = 0; ich < IN_CHANNELS; ich++) {
                        printf("%d,", weights[och][wrow][wcol][ich]);
                    }
                    printf("\b],");
                }
                printf("\b],\n");
            }
            printf("\b],");
        }
        printf("\b\n\n");

        printf("weights_mat:\n");
        for (int wrow = 0; wrow < KERNEL_DIM * KERNEL_DIM * IN_CHANNELS; wrow++) {
            printf("[");
            for (int wcol = 0; wcol < OUT_CHANNELS; wcol++) {
                printf("%d,", weights_mat[wrow][wcol]);
            }
            printf("\b],\n");
        }
        printf("\b\n\n");

        printf("input:\n");
        for (int batch = 0; batch < BATCH_SIZE; batch++) {
            printf("[");
            for (int irow = 0; irow < IN_DIM; irow++) {
                printf("[");
                for (int icol = 0; icol < IN_DIM; icol++) {
                    printf("[");
                    for (int ich = 0; ich < IN_CHANNELS; ich++) {
                        printf("%d,", input[batch][irow][icol][ich]);
                    }
                    printf("\b],");
                }
                printf("\b],\n");
            }
            printf("\b],");
        }
        printf("\b\n\n");

        printf("output:\n");
        for (int batch = 0; batch < BATCH_SIZE; batch++) {
            printf("[");
            for (int orow = 0; orow < OUT_DIM; orow++) {
                printf("[");
                for (int ocol = 0; ocol < OUT_DIM; ocol++) {
                    printf("[");
                    for (int och = 0; och < OUT_CHANNELS; och++) {
                        printf("%d,", output[batch][orow][ocol][och]);
                    }
                    printf("\b],");
                }
                printf("\b],\n");
            }
            printf("\b],");
        }
        printf("\b\n\n");

        printf("pool_output:\n");
        for (int batch = 0; batch < BATCH_SIZE; batch++) {
            printf("[");
            for (int orow = 0; orow < POOL_OUT_DIM; orow++) {
                printf("[");
                for (int ocol = 0; ocol < POOL_OUT_DIM; ocol++) {
                    printf("[");
                    for (int och = 0; och < OUT_CHANNELS; och++) {
                        printf("%d,", pool_output[batch][orow][ocol][och]);
                    }
                    printf("\b],");
                }
                printf("\b],\n");
            }
            printf("\b],");
        }
        printf("\b\n\n");


        printf("pool_output_mat:\n");
        for (int orow = 0; orow < BATCH_SIZE * POOL_OUT_DIM * POOL_OUT_DIM; orow++) {
            printf("[");
            for (int ocol = 0; ocol < OUT_CHANNELS; ocol++) {
                printf("%d,", pool_output_mat[orow][ocol]);
            }
            printf("\b],\n");
        }
        printf("\b\n\n");

        printf("Output dimension: %u\n", OUT_DIM);
        printf("Pooling output dimension: %u\n\n", POOL_OUT_DIM);

        return 1;
    }

    return 0;
}
