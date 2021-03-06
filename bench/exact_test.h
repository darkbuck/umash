#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct xoshiro;

/**
 * Reseeds the global state from which we derive independentPRNG
 * states.
 *
 * Reseeding lets us explore different random streams afer fork(2)ing.
 */
void exact_test_prng_seed(uint64_t seed);

/**
 * Returns an independent PRNG state.
 *
 * Wrapper to make embedding easier.
 */
struct xoshiro *exact_test_prng_create(void);

void exact_test_prng_destroy(struct xoshiro *);

/**
 * Shuffles `observations[0 ... m + n - 1]` in place.
 *
 * @param m the number of observations in class A.
 * @param n the number of observations in class B.
 * @param p_a_lower the probability that values in A < B. 0.5 yields
 *   a classic shuffle for permutation testing.
 *
 * On return, the first `m` values in `observation` will correspond
 * to class A, and the remaining `n` to class B.
 *
 * I'm not fully convinced that `p_a_lower != 0.5` does the right
 * thing for imbalanced designs.
 */
bool exact_test_shuffle(struct xoshiro *, uint64_t *observations, size_t m, size_t n,
    double p_a_lower, const char **error);

/**
 * Tags the observations with their class, and sorts them in ascending order.
 *
 * @param observations an array of m + n observation values.
 * @param m the first m observations on entry are in class A.
 * @param n the last n observaions on entry are in class B.
 * @param a_offset value to add to class A.
 * @param b_offset value to add to class B.
 *
 * On exit, the observation array will contain 63-bit observations,
 * with the low bit stolen to denote the class (0 for class A, 1 for
 * class B), and ties broken by letting class A show up first.  This
 * function is deterministic, and the input `xoshiro` only used for an
 * internal sampling sort algorithm.
 *
 * The sum of an observation and its offset will wrap around if it
 * exceeds 2**63 - 1.
 */
void exact_test_offset_sort(struct xoshiro *, uint64_t *observations, size_t m, size_t n,
    uint64_t a_offset, uint64_t b_offset);

/**
 * Computes the sample probability that a value from A > a value from B.
 *
 * @param observations an array of `m + n` values, as generated by
 *  `exact_test_offset_sort`.
 * @param m number of observations in class A.
 * @param n number of observations in class B.
 */
double exact_test_gt_prob(const uint64_t *observations, size_t m, size_t n);

/**
 * Computes the sample probability that a value from A <= a value from B.
 *
 * @param observations an array of `m + n` values, as generated by
 *  `exact_test_offset_sort`.
 * @param m number of observations in class A.
 * @param n number of observations in class B.
 */
double exact_test_lte_prob(const uint64_t *observations, size_t m, size_t n);

/**
 * Computes the difference TruncatedMean(A) - TruncatedMean(B)
 *
 * @param observations an array of `m + n` values, as generated by
 *  `exact_test_offset_sort`.
 * @param m number of observations in class A.
 * @param n number of observations in class B.
 * @param truncate_frac the fraction of observations to remove at each
 *   extreme of A and B (rounded up).
 */
double exact_test_truncated_mean_diff(
    const uint64_t *observations, size_t m, size_t n, double truncate_frac);

/**
 * Computes the difference between the quantile'th least value for A and B.
 *
 * @param quantile the rank at which to compare the two distributions, as
 *   a fraction.  0.5 compares the median, 0.99 the 99th percentile, etc.
 */
double exact_test_quantile_diff(
    const uint64_t *observations, size_t m, size_t n, double quantile);
