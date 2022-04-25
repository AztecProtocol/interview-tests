# Applied Cryptographer Test

**WARNING: Do not fork this repository or make a public repository containing your solution. Either copy it to a private repository or submit your solution via other means.**

Links to solutions may be sent to hello@aztecprotocol.com.

---
This is a test for our prospective applied cryptography engineers. 

These questions are a drawn from real topics and problems we've encountered at Aztec. They are representative of the kind of work you will be involved in as part of our team.

Your core proficiencies may not align  with what is needed to answer all questions. We don't expect candidates to provide complete answers to every question. Please answer as many as you can.

---
#### Question 1

**The Plonk permutation argument is described in section 5 of the [paper](https://eprint.iacr.org/2019/953.pdf).**

a) Read the first protocol described in Section 5 (page 20). Suppose we fixed <img src="https://render.githubusercontent.com/render/math?math=\beta=1"> in the protocol instead of choosing it randomly (and kept <img src="https://render.githubusercontent.com/render/math?math=\gamma"> uniformly chosen as in the protocol). Suppose we fix <img src="https://render.githubusercontent.com/render/math?math=n=4">. Give an example of a permutation <img src="https://render.githubusercontent.com/render/math?math=\sigma:[4]\to [4]">, and a pair of polynomials f,g not satisfying <img src="https://render.githubusercontent.com/render/math?math=g=\sigma(f)">, but still causing the verifier to accept with high probability. Explain why the example works.

b) Same question when fixing <img src="https://render.githubusercontent.com/render/math?math=\gamma=0"> instead and uniform <img src="https://render.githubusercontent.com/render/math?math=\beta">.


c) If we restrict the prover to always using f=g, does fixing <img src="https://render.githubusercontent.com/render/math?math=\gamma=0"> still allow them to cheat? If not, give a proof sketch explaining why. (_Hint: reduce to Schwartz-Zippel on monic polynomials, by factoring out a common term in both aggregating products._) 
---
#### Question 2

In [our implementation](https://github.com/AztecProtocol/aztec-2-bug-bounty/tree/master/barretenberg/src/aztec/ecc/curves/bn254/scalar_multiplication) of [Pippenger's multi-exponentiation algorithm](https://jbootle.github.io/Misc/pippenger.pdf) for BN254 elliptic curve points, we implement a subroutine that evaluates large numbers of linearly independent additions of elliptic curve points (represented via the `g1` type, whose size is 64 bytes).

At the stage of the algorithm of interest for this question, we have assigned members of a set of base points to a set of 'buckets' (where `num_buckets ~ log2(num_points)`).

For each bucket, we must add together its associated points into a single accumulator point.

The state at this point of the algorithm can be represented as the following:

```
struct bucket state{
    uint64_t* point_schedule;
    uint64_t num_bucket_points;
};
struct runtime_state{
    uint64_t num_points;
    uint64_t num_buckets;
    g1* base_points;
    bucket_state* buckets;
};
```

`point_schedule` is an array, whose elements describe offsets that are applied to `base_points` to address a single base point. The values in `point_schedule` will range from `0` to `num_points` and can be modelled as being uniformly random within this range.


The method `affine_add(g1* input_pts, g1* output_pts, uint64_t num_points)` can be treated as a black-box algorithm that adds together pairs of points in `input_pts`, returning the output into `output_pts`.

For example, feeding 4 input points into `affine_add` returns the following into `output_pts`:

| data struct | element 0  | element 1 | element 2 | element 3 |
| -------- | -------- | -------- | -------- | -------- |
| `input_pts` | p_1 | p_2 | p_3 | p_4 |
| `output_pts`| p_1 + p_2     | p_3 + p_4     | --- | --- |

The performance of `affine_add` degrades if `num_points` is small.

**a) In a language of your choice (incl. pseudocode), design algorithms that perform the following:**
* i) For each bucket, arrange in memory the set of points to be added together
* ii) For each bucket, use `affine_add` to accumulate the bucket points into a single output point

**b) Assuming a prime-field multiplication takes 17ns and `affine_add` uses 6 of these per point pair, what is the likely performance bottleneck on a 16-core CPU, if each core is executing this algorithm?**

**c) To our knowledge, no other implementations of Pippenger accumulate bucket points using the type of pair-wise addition in `affine_add` (instead they iterate over the bucket points and add them successively into an accumulator point). Why do we do the above instead?**

---
#### Question 3

**a) In a language of your choice (incl. pseudocode), implement a multithreaded radix sort. Inputs are 16-bit integers. Set size is ~2^24**



---
#### (harder/bonus) Question 4
The following C++ code implements a [Montgomery modular multiplication](https://en.wikipedia.org/wiki/Montgomery_modular_multiplication) over a 254-bit prime field for x86/64 CPUs with the [BMI2](https://en.wikipedia.org/wiki/Bit_manipulation_instruction_set) instruction set. Field elements are stored in 4 64-bit 'limbs'. 

Operation is Montgomery-form equivalent of <img src="https://render.githubusercontent.com/render/math?math=a * b = c \text{ mod } q">, where `q` is defined by `T::modulus`.

Both inputs and outputs are allowed to be in an 'unreduced' form, where a factor of `q` may be added into the variable. (i.e. `a, b, c` can be 255-bit integers).

Registers`%rax, %rbx, %rcx` contain references to `a, b, c` respectively.

**a) Spot the error in this code. Why is it incorrect and what is the fix?**  
**b) Can you improve the performance of this code? You cannot use additional registers.**


```
struct alignas(32) field {
    uint64_t data[4];
};

/**
 * Compute Montgomery multiplication of a, b.
 * Result is stored, in "r"
 **/
template <class T> inline void asm_mul(const field& a, const field& b, field& r) noexcept
{
    constexpr uint64_t r_inv = T::r_inv; // r_inv = (-1 / q) mod 2^256
    constexpr uint64_t modulus_0 = T::modulus_0;
    constexpr uint64_t modulus_1 = T::modulus_1;
    constexpr uint64_t modulus_2 = T::modulus_2;
    constexpr uint64_t modulus_3 = T::modulus_3;
    constexpr uint64_t zero_ref = 0;
    __asm__(
        "movq 0(%%rax), %%rdx                      \n\t" /* load a[0] into %rdx                                     */ 
        "xorq %%r8, %%r8                           \n\t" /* clear flags                                             */ 
        /* front-load mul ops, can parallelize 4 of these but latency is 4 cycles */                                   
        "mulxq 0(%%rbx), %%r13, %%r14              \n\t" /* (r[0], r[1]) <- a[0] * b[0]                             */ 
        "mulxq 8(%%rbx), %%r8, %%r9                \n\t" /* (t[0], t[1]) <- a[0] * b[1]                             */ 
        "mulxq 16(%%rbx), %%r15, %%r10             \n\t" /* (r[2] , r[3]) <- a[0] * b[2]                            */ 
        "mulxq 24(%%rbx), %%rdi, %%r12             \n\t" /* (t[2], r[4]) <- a[0] * b[3] (overwrite a[0])            */ 
        /* start computing modular reduction */                                                                        
        "movq %%r13, %%rdx                         \n\t" /* move r[0] into %rdx                                     */ 
        "mulxq %[r_inv], %%rdx, %%r11              \n\t" /* (%rdx, _) <- k = r[0] * r_inv                           */ 
        /* start first addition chain */                                                                               
        "adoxq %%r8, %%r14                         \n\t" /* r[1] += t[0]                                            */ 
        "adcxq %%rdi, %%r10                        \n\t" /* r[3] += t[2] + flag_c                                   */ 
        "adoxq %%r9, %%r15                         \n\t" /* r[2] += t[1] + flag_o                                   */ 
        /* reduce by r[0] * k */                                                                                       
        "mulxq %[modulus_3], %%rdi, %%r11          \n\t" /* (t[2], t[3]) <- (modulus.data[3] * k)                   */ 
        "mulxq %[modulus_0], %%r8, %%r9            \n\t" /* (t[0], t[1]) <- (modulus.data[0] * k)                   */ 
        "adcxq %%rdi, %%r10                        \n\t" /* r[3] += t[2] + flag_c                                   */ 
        "adoxq %%r11, %%r12                        \n\t" /* r[4] += t[3] + flag_o                                   */ 
        "adcxq %[zero_reference], %%r12            \n\t" /* r[4] += flag_c                                          */ 
        "adoxq %%r8, %%r13                         \n\t" /* r[0] += t[0] (%r13 now free)                            */ 
        "adcxq %%r9, %%r14                         \n\t" /* r[1] += t[1] + flag_c                                   */ 
        "mulxq %[modulus_1], %%rdi, %%r11          \n\t" /* (t[0], t[1]) <- (modulus.data[1] * k)                   */ 
        "mulxq %[modulus_2], %%r8, %%r9            \n\t" /* (t[0], t[1]) <- (modulus.data[2] * k)                   */ 
        "adoxq %%rdi, %%r14                        \n\t" /* r[1] += t[0]                                            */ 
        "adcxq %%r11, %%r15                        \n\t" /* r[2] += t[1] + flag_c                                   */ 
        "adoxq %%r8, %%r15                         \n\t" /* r[2] += t[0] + flag_o                                   */ 
        "adcxq %%r9, %%r10                         \n\t" /* r[3] += t[1] + flag_c                                   */ 
        /* modulus = 254 bits, so max(t[3])  = 62 bits                                                              */ 
        /* b also 254 bits, so (a[0] * b[3]) = 62 bits                                                              */ 
        /* i.e. carry flag here is always 0 if b is in mont form, no need to update r[5]                            */ 
        /* (which is very convenient because we're out of registers!)                                               */ 
        /* N.B. the value of r[4] now has a max of 63 bits and can accept another 62 bit value before overflowing   */ 
        /* a[1] * b */                                                                                                 
        "movq 8(%%rax), %%rdx                      \n\t" /* load a[1] into %rdx                                     */   
        "mulxq 16(%%rbx), %%r8, %%r9               \n\t" /* (t[2], t[3]) <- (a[1] * b[2])                           */ 
        "mulxq 24(%%rbx), %%rdi, %%r13             \n\t" /* (t[6], r[5]) <- (a[1] * b[3])                           */ 
        "adoxq %%r8, %%r10                         \n\t" /* r[3] += t[0] + flag_o                                   */ 
        "adcxq %%rdi, %%r12                        \n\t" /* r[4] += t[2] + flag_c                                   */ 
        "adoxq %%r9, %%r12                         \n\t" /* r[4] += t[1] + flag_o                                   */ 
        "adcxq %[zero_reference], %%r13            \n\t" /* r[5] += flag_c                                          */ 
        "adoxq %[zero_reference], %%r13            \n\t" /* r[5] += flag_o                                          */ 
        "mulxq 0(%%rbx), %%r8, %%r9                \n\t" /* (t[0], t[1]) <- (a[1] * b[0])                           */ 
        "mulxq 8(%%rbx), %%rdi, %%r11              \n\t" /* (t[4], t[5]) <- (a[1] * b[1])                           */ 
        "adcxq %%r8, %%r14                         \n\t" /* r[1] += t[0] + flag_c                                   */ 
        "adoxq %%r9, %%r15                         \n\t" /* r[2] += t[1] + flag_o                                   */ 
        "adcxq %%rdi, %%r15                        \n\t" /* r[2] += t[0] + flag_c                                   */ 
        "adoxq %%r11, %%r10                        \n\t" /* r[3] += t[1] + flag_o                                   */ 
        /* reduce by r[1] * k */                                                                                       
        "movq %%r14, %%rdx                         \n\t"  /* move r[1] into %rdx                                    */ 
        "mulxq %[r_inv], %%rdx, %%r8               \n\t"  /* (%rdx, _) <- k = r[1] * r_inv                          */ 
        "mulxq %[modulus_2], %%r8, %%r9            \n\t"  /* (t[0], t[1]) <- (modulus.data[2] * k)                  */ 
        "mulxq %[modulus_3], %%rdi, %%r11          \n\t"  /* (t[2], t[3]) <- (modulus.data[3] * k)                  */ 
        "adcxq %%r8, %%r10                         \n\t"  /* r[3] += t[0] + flag_c                                  */ 
        "adoxq %%r9, %%r12                         \n\t"  /* r[4] += t[2] + flag_o                                  */ 
        "adcxq %%rdi, %%r12                        \n\t"  /* r[4] += t[1] + flag_c                                  */ 
        "adoxq %%r11, %%r13                        \n\t"  /* r[5] += t[3] + flag_o                                  */ 
        "adcxq %[zero_reference], %%r13            \n\t"  /* r[5] += flag_c                                         */ 
        "mulxq %[modulus_0], %%r8, %%r9            \n\t"  /* (t[0], t[1]) <- (modulus.data[0] * k)                  */ 
        "mulxq %[modulus_1], %%rdi, %%r11          \n\t"  /* (t[0], t[1]) <- (modulus.data[1] * k)                  */ 
        "adoxq %%r8, %%r14                         \n\t"  /* r[1] += t[0] (%r14 now free)                           */ 
        "adcxq %%rdi, %%r15                        \n\t"  /* r[2] += t[0] + flag_c                                  */ 
        "adoxq %%r9, %%r15                         \n\t"  /* r[2] += t[1] + flag_o                                  */ 
        "adcxq %%r11, %%r10                        \n\t"  /* r[3] += t[1] + flag_c                                  */ 
        /* a[2] * b */                                                                                                 
        "movq 16(%%rax), %%rdx                     \n\t" /* load a[2] into %rdx                                     */ 
        "mulxq 8(%%rbx), %%rdi, %%r11              \n\t" /* (t[0], t[1]) <- (a[2] * b[1])                           */ 
        "mulxq 16(%%rbx), %%r8, %%r9               \n\t" /* (t[0], t[1]) <- (a[2] * b[2])                           */ 
        "adoxq %%rdi, %%r10                        \n\t" /* r[3] += t[0] + flag_o                                   */ 
        "adcxq %%r11, %%r12                        \n\t" /* r[4] += t[1] + flag_c                                   */ 
        "adoxq %%r8, %%r12                         \n\t" /* r[4] += t[0] + flag_o                                   */ 
        "adcxq %%r9, %%r13                         \n\t" /* r[5] += t[2] + flag_c                                   */ 
        "mulxq 24(%%rbx), %%rdi, %%r14             \n\t" /* (t[2], r[6]) <- (a[2] * b[3])                           */ 
        "mulxq 0(%%rbx), %%r8, %%r9                \n\t" /* (t[0], t[1]) <- (a[2] * b[0])                           */ 
        "adoxq %%rdi, %%r13                        \n\t" /* r[5] += t[1] + flag_o                                   */ 
        "adcxq %[zero_reference], %%r14            \n\t" /* r[6] += flag_c                                          */ 
        "adoxq %[zero_reference], %%r14            \n\t" /* r[6] += flag_o                                          */ 
        "adcxq %%r8, %%r15                         \n\t" /* r[2] += t[0] + flag_c                                   */ 
        "adoxq %%r9, %%r10                         \n\t" /* r[3] += t[1] + flag_o                                   */ 
        /* reduce by r[2] * k */                                                                                       
        "movq %%r15, %%rdx                         \n\t"  /* move r[2] into %rdx                                    */ 
        "mulxq %[r_inv], %%rdx, %%r8               \n\t"  /* (%rdx, _) <- k = r[2] * r_inv                          */ 
        "mulxq %[modulus_1], %%rdi, %%r11          \n\t"  /* (t[0], t[1]) <- (modulus.data[1] * k)                  */ 
        "mulxq %[modulus_2], %%r8, %%r9            \n\t"  /* (t[0], t[1]) <- (modulus.data[2] * k)                  */ 
        "adcxq %%rdi, %%r10                        \n\t"  /* r[3] += t[1] + flag_c                                  */ 
        "adoxq %%r11, %%r12                        \n\t"  /* r[4] += t[1] + flag_o                                  */ 
        "adcxq %%r8, %%r12                         \n\t"  /* r[4] += t[0] + flag_c                                  */ 
        "adoxq %%r9, %%r13                         \n\t"  /* r[5] += t[2] + flag_o                                  */ 
        "mulxq %[modulus_3], %%rdi, %%r11          \n\t"  /* (t[2], t[3]) <- (modulus.data[3] * k)                  */ 
        "mulxq %[modulus_0], %%r8, %%r9            \n\t"  /* (t[0], t[1]) <- (modulus.data[0] * k)                  */ 
        "adcxq %%rdi, %%r13                        \n\t"  /* r[5] += t[1] + flag_c                                  */ 
        "adoxq %%r11, %%r14                        \n\t"  /* r[6] += t[3] + flag_o                                  */ 
        "adcxq %[zero_reference], %%r14            \n\t"  /* r[6] += flag_c                                         */ 
        "adoxq %%r8, %%r15                         \n\t"  /* r[2] += t[0] (%r15 now free)                           */ 
        "adcxq %%r9, %%r10                         \n\t"  /* r[3] += t[0] + flag_c                                  */ 
        /* a[3] * b */                                                                                                 
        "movq 24(%%rax), %%rdx                     \n\t"  /* load a[3] into %rdx                                    */ 
        "mulxq 0(%%rbx), %%r8, %%r9                \n\t"  /* (t[0], t[1]) <- (a[3] * b[0])                          */ 
        "mulxq 8(%%rbx), %%rdi, %%r11              \n\t"  /* (t[4], t[5]) <- (a[3] * b[1])                          */ 
        "adoxq %%r8, %%r10                         \n\t"  /* r[3] += t[0] + flag_o                                  */ 
        "adcxq %%r9, %%r12                         \n\t"  /* r[4] += t[2] + flag_c                                  */ 
        "adoxq %%rdi, %%r12                        \n\t"  /* r[4] += t[1] + flag_o                                  */ 
        "adcxq %%r11, %%r13                        \n\t"  /* r[5] += t[3] + flag_c                                  */ 
        "mulxq 16(%%rbx), %%r8, %%r9               \n\t"  /* (t[2], t[3]) <- (a[3] * b[2])                          */ 
        "mulxq 24(%%rbx), %%rdi, %%r15             \n\t"  /* (t[6], r[7]) <- (a[3] * b[3])                          */ 
        "adoxq %%r8, %%r13                         \n\t"  /* r[5] += t[4] + flag_o                                  */ 
        "adcxq %%r9, %%r14                         \n\t"  /* r[6] += t[6] + flag_c                                  */ 
        "adoxq %%rdi, %%r14                        \n\t"  /* r[6] += t[5] + flag_o                                  */ 
        "adcxq %[zero_reference], %%r15            \n\t"  /* r[7] += flag_c                                         */ 
        "adoxq %[zero_reference], %%r15            \n\t"  /* r[7] += flag_o                                         */ 
        /* reduce by r[3] * k */                                                                                       
        "movq %%r10, %%rdx                         \n\t" /* move r_inv into %rdx                                    */ 
        "mulxq %[r_inv], %%rdx, %%r8               \n\t" /* (%rdx, _) <- k = r[3] * r_inv                           */ 
        "mulxq %[modulus_0], %%r8, %%r9            \n\t" /* (t[0], t[1]) <- (modulus.data[0] * k)                   */ 
        "mulxq %[modulus_1], %%rdi, %%r11          \n\t" /* (t[2], t[3]) <- (modulus.data[1] * k)                   */ 
        "adoxq %%r8, %%r10                         \n\t" /* r[3] += t[0] (%rsi now free)                            */ 
        "adcxq %%r9, %%r12                         \n\t" /* r[4] += t[2] + flag_c                                   */ 
        "adoxq %%rdi, %%r12                        \n\t" /* r[4] += t[1] + flag_o                                   */ 
        "adcxq %%r11, %%r13                        \n\t" /* r[5] += t[3] + flag_c                                   */ 
        "mulxq %[modulus_2], %%r8, %%r9            \n\t" /* (t[4], t[5]) <- (modulus.data[2] * k)                   */ 
        "mulxq %[modulus_3], %%rdi, %%rdx          \n\t" /* (t[6], t[7]) <- (modulus.data[3] * k)                   */ 
        "adoxq %%r8, %%r13                         \n\t" /* r[5] += t[4] + flag_o                                   */ 
        "adcxq %%r9, %%r14                         \n\t" /* r[6] += t[6] + flag_c                                   */ 
        "adoxq %%rdi, %%r14                        \n\t" /* r[6] += t[5] + flag_o                                   */ 
        "adcxq %%rdx, %%r15                        \n\t" /* r[7] += t[7] + flag_c                                   */ 
        "adoxq %[zero_reference], %%r15            \n\t" /* r[7] += flag_o                                          */
        "movq %%r12, 0(%%rcx)                      \n\t"                                                                   
        "movq %%r13, 8(%%rcx)                      \n\t"                                                                   
        "movq %%r14, 16(%%rcx)                     \n\t"                                                                   
        "movq %%r15, 24(%%rcx)                     \n\t"
        :
        : "a"(&a),
          "b"(&b),
          "c"(&r),
          [ modulus_0 ] "m"(modulus_0),
          [ modulus_1 ] "m"(modulus_1),
          [ modulus_2 ] "m"(modulus_2),
          [ modulus_3 ] "m"(modulus_3),
          [ r_inv ] "m"(r_inv),
          [ zero_reference ] "m"(zero_ref)
        : "%rdx", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "cc", "memory");
}
```
