# Applied Cryptographer Test

**WARNING: Do not fork this repository or make a public repository containing your solution. Either copy it to a private repository or submit your solution via other means.**

Links to solutions may be sent to travis@aztecprotocol.com.

---
This is a test for our prospective applied cryptography engineers. 

These questions are a drawn from real topics and problems we've encountered at Aztec. They are representative of the kind of work you will be involved in as part of our team.

Your core proficiencies may not align with what is needed to answer all questions. We don't expect candidates to provide complete answers to every question (this is not a trick, we genuinely don't) Please answer as many as you can.

---
#### Question 1

**The Plonk permutation argument is described in section 5 of the [paper](https://eprint.iacr.org/2019/953.pdf).**

a) Read the first protocol described in Section 5 (page 20). Suppose we fixed <img src="https://render.githubusercontent.com/render/math?math=\beta=1"> in the protocol instead of choosing it randomly (and kept <img src="https://render.githubusercontent.com/render/math?math=\gamma"> uniformly chosen as in the protocol). Suppose we fix <img src="https://render.githubusercontent.com/render/math?math=n=4">. Give an example of a permutation <img src="https://render.githubusercontent.com/render/math?math=\sigma:[4]\to [4]">, and a pair of polynomials f,g not satisfying <img src="https://render.githubusercontent.com/render/math?math=g=\sigma(f)">, but still causing the verifier to accept with high probability. Explain why the example works.

b) Same question when fixing <img src="https://render.githubusercontent.com/render/math?math=\gamma=0"> instead and uniform <img src="https://render.githubusercontent.com/render/math?math=\beta">.



---
#### Question 2

In [our implementation](https://github.com/AztecProtocol/barretenberg/tree/master/cpp/src/aztec/ecc/curves/bn254/scalar_multiplication) of [Pippenger's multi-exponentiation algorithm](https://jbootle.github.io/Misc/pippenger.pdf) for BN254 elliptic curve points we add points in pairs in affine form in the function *add_affine_points*. To our knowledge not many other implementations of Pippenger accumulate bucket points using the type of pair-wise addition (instead they iterate over the bucket points and add them successively into an accumulator point). **Why do we do the above instead?**

---
#### Question 3

**a) In a language of your choice (incl. pseudocode), implement a multithreaded radix sort. Inputs are 16-bit integers. Set size is ~2^24**

---
#### Question 4

Using https://github.com/AztecProtocol/barretenberg/blob/master/cpp/src/aztec/ecc/curves/bn254/fr.test.cpp as reference write a test where Alice convinces Bob of the value of the inner product of randomly sampled fr vectors of size n=2^d using the sumcheck protocol. Bob also has access to the sampled vectors. References for sumcheck:[1](https://people.cs.georgetown.edu/jthaler/sumcheck.pdf),[2](https://people.cs.georgetown.edu/jthaler/ProofsArgsAndZK.html).

---
#### (harder/bonus) Question 5
The following C++ code implements a [Montgomery modular multiplication](https://en.wikipedia.org/wiki/Montgomery_modular_multiplication) over a 254-bit prime field for x86/64 CPUs with the [BMI2](https://en.wikipedia.org/wiki/Bit_manipulation_instruction_set) instruction set. Field elements are stored in 4 64-bit 'limbs'. 

Operation is Montgomery-form equivalent of <img src="https://render.githubusercontent.com/render/math?math=a * b = c \text{ mod } q">, where `q` is defined by `T::modulus`.

Both inputs and outputs are allowed to be in an 'unreduced' form, where a factor of `q` may be added into the variable. (i.e. `a, b, c` can be 255-bit integers).

Registers`%rax, %rbx, %rcx` contain references to `a, b, c` respectively.
 
**a) Spot the error in this code. Why is it incorrect and what is the fix?**  
**b) What could be the impact of this error? How would you find erroneous inputs?**


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
        "movq 0(%%rax), %%rdx                        \n\t" /* load a[0] into %rdx                                   */  \
        "xorq %%r8, %%r8                           \n\t" /* clear r10 register, we use this when we need 0          */  \
        /* front-load mul ops, can parallelize 4 of these but latency is 4 cycles */                                    \
        "mulxq 0(%%rbx), %%r13, %%r14              \n\t" /* (r[0], r[1]) <- a[0] * b[0]                             */  \
        "mulxq 8(%%rbx), %%r8, %%r9                \n\t" /* (t[0], t[1]) <- a[0] * b[1]                             */  \
        "mulxq 16(%%rbx), %%r15, %%r10             \n\t" /* (r[2] , r[3]) <- a[0] * b[2]                            */  \
        "mulxq 24(%%rbx), %%rdi, %%r12             \n\t" /* (t[2], r[4]) <- a[0] * b[3] (overwrite a[0])            */  \
        /* zero flags */                                                                                                \
                                                                                                                        \
        /* start computing modular reduction */                                                                         \
        "movq %%r13, %%rdx                         \n\t" /* move r[0] into %rdx                                     */  \
        "mulxq %[r_inv], %%rdx, %%r11              \n\t" /* (%rdx, _) <- k = r[1] * r_inv                           */  \
                                                                                                                        \
        /* start first addition chain */                                                                                \
        "adcxq %%r8, %%r14                         \n\t" /* r[1] += t[0]                                            */  \
        "adoxq %%rdi, %%r10                        \n\t" /* r[3] += t[2] + flag_o                                   */  \
        "adcxq %%r9, %%r15                         \n\t" /* r[2] += t[1] + flag_c                                   */  \
                                                                                                                        \
        /* reduce by r[0] * k */                                                                                        \
        "mulxq %[modulus_3], %%rdi, %%r11          \n\t" /* (t[2], t[3]) <- (modulus.data[3] * k)                   */  \
        "mulxq %[modulus_0], %%r8, %%r9            \n\t" /* (t[0], t[1]) <- (modulus.data[0] * k)                   */  \
        "adcxq %%rdi, %%r10                        \n\t" /* r[3] += t[2] + flag_c                                   */  \
        "adoxq %%r11, %%r12                        \n\t" /* r[4] += t[3] + flag_c                                   */  \
        "adcxq %[zero_reference], %%r12            \n\t" /* r[4] += flag_i                                          */  \
        "adoxq %%r8, %%r13                         \n\t" /* r[0] += t[0] (%r13 now free)                            */  \
        "adcxq %%r9, %%r14                         \n\t" /* r[1] += t[1] + flag_o                                   */  \
        "mulxq %[modulus_1], %%rdi, %%r11          \n\t" /* (t[0], t[1]) <- (modulus.data[1] * k)                   */  \
        "mulxq %[modulus_2], %%r8, %%r9            \n\t" /* (t[0], t[1]) <- (modulus.data[2] * k)                   */  \
        "adoxq %%rdi, %%r14                        \n\t" /* r[1] += t[0]                                            */  \
        "adcxq %%r11, %%r15                        \n\t" /* r[2] += t[1] + flag_c                                   */  \
        "adoxq %%r8, %%r15                         \n\t" /* r[2] += t[0] + flag_o                                   */  \
        "adcxq %%r9, %%r10                         \n\t" /* r[3] += t[1] + flag_o                                   */  \
                                                                                                                        \
        /* modulus = 254 bits, so max(t[3])  = 62 bits                                                              */  \
        /* b also 254 bits, so (a[0] * b[3]) = 62 bits                                                              */  \
        /* i.e. carry flag here is always 0 if b is in mont form, no need to update r[5]                            */  \
        /* (which is very convenient because we're out of registers!)                                               */  \
        /* N.B. the value of r[4] now has a max of 63 bits and can accept another 62 bit value before overflowing   */  \
                                                                                                                        \
        /* a[1] * b */                                                                                                  \
        "movq 8(%%rax), %%rdx                      \n\t" /* load a[1] into %rdx                                     */  \
        "mulxq 16(%%rbx), %%r8, %%r9               \n\t" /* (t[2], t[3]) <- (a[1] * b[2])                           */  \
        "mulxq 24(%%rbx), %%rdi, %%r13             \n\t" /* (t[6], r[5]) <- (a[1] * b[3])                           */  \
        "adoxq %%r8, %%r10                         \n\t" /* r[3] += t[0] + flag_c                                   */  \
        "adcxq %%rdi, %%r12                        \n\t" /* r[4] += t[2] + flag_o                                   */  \
        "adoxq %%r9, %%r12                         \n\t" /* r[4] += t[1] + flag_c                                   */  \
        "adcxq %[zero_reference], %%r13            \n\t" /* r[5] += flag_o                                          */  \
        "adoxq %[zero_reference], %%r13            \n\t" /* r[5] += flag_c                                          */  \
        "mulxq 0(%%rbx), %%r8, %%r9                \n\t" /* (t[0], t[1]) <- (a[1] * b[0])                           */  \
        "mulxq 8(%%rbx), %%rdi, %%r11              \n\t" /* (t[4], t[5]) <- (a[1] * b[1])                           */  \
        "adcxq %%r8, %%r14                         \n\t" /* r[1] += t[0] + flag_c                                   */  \
        "adoxq %%r9, %%r15                         \n\t" /* r[2] += t[1] + flag_o                                   */  \
        "adcxq %%rdi, %%r15                        \n\t" /* r[2] += t[0] + flag_c                                   */  \
        "adoxq %%r11, %%r10                        \n\t" /* r[3] += t[1] + flag_o                                   */  \
                                                                                                                        \
        /* reduce by r[1] * k */                                                                                        \
        "movq %%r14, %%rdx                         \n\t"  /* move r[1] into %rdx                                    */  \
        "mulxq %[r_inv], %%rdx, %%r8               \n\t"  /* (%rdx, _) <- k = r[1] * r_inv                          */  \
        "mulxq %[modulus_2], %%r8, %%r9            \n\t"  /* (t[0], t[1]) <- (modulus.data[2] * k)                  */  \
        "mulxq %[modulus_3], %%rdi, %%r11          \n\t"  /* (t[2], t[3]) <- (modulus.data[3] * k)                  */  \
        "adcxq %%r8, %%r10                         \n\t"  /* r[3] += t[0] + flag_o                                  */  \
        "adoxq %%r9, %%r12                         \n\t"  /* r[4] += t[2] + flag_c                                  */  \
        "adcxq %%rdi, %%r12                        \n\t"  /* r[4] += t[1] + flag_o                                  */  \
        "adoxq %%r11, %%r13                        \n\t"  /* r[5] += t[3] + flag_c                                  */  \
        "adcxq %[zero_reference], %%r13            \n\t"  /* r[5] += flag_o                                         */  \
        "mulxq %[modulus_0], %%r8, %%r9            \n\t"  /* (t[0], t[1]) <- (modulus.data[0] * k)                  */  \
        "mulxq %[modulus_1], %%rdi, %%r11          \n\t"  /* (t[0], t[1]) <- (modulus.data[1] * k)                  */  \
        "adoxq %%r8, %%r14                         \n\t"  /* r[1] += t[0] (%r14 now free)                           */  \
        "adcxq %%rdi, %%r15                        \n\t"  /* r[2] += t[0] + flag_c                                  */  \
        "adoxq %%r9, %%r15                         \n\t"  /* r[2] += t[1] + flag_o                                  */  \
        "adcxq %%r11, %%r10                        \n\t"  /* r[3] += t[1] + flag_c                                  */  \
                                                                                                                        \
        /* a[2] * b */                                                                                                  \
        "movq 16(%%rax), %%rdx                     \n\t" /* load a[2] into %rdx                                     */  \
        "mulxq 8(%%rbx), %%rdi, %%r11              \n\t" /* (t[0], t[1]) <- (a[2] * b[1])                           */  \
        "mulxq 16(%%rbx), %%r8, %%r9               \n\t" /* (t[0], t[1]) <- (a[2] * b[2])                           */  \
        "adoxq %%rdi, %%r10                        \n\t" /* r[3] += t[0] + flag_c                                   */  \
        "adcxq %%r11, %%r12                        \n\t" /* r[4] += t[1] + flag_o                                   */  \
        "adoxq %%r8, %%r12                         \n\t" /* r[4] += t[0] + flag_c                                   */  \
        "adcxq %%r9, %%r13                         \n\t" /* r[5] += t[2] + flag_o                                   */  \
        "mulxq 24(%%rbx), %%rdi, %%r14             \n\t" /* (t[2], r[6]) <- (a[2] * b[3])                           */  \
        "mulxq 0(%%rbx), %%r8, %%r9                \n\t" /* (t[0], t[1]) <- (a[2] * b[0])                           */  \
        "adoxq %%rdi, %%r13                        \n\t" /* r[5] += t[1] + flag_c                                   */  \
        "adcxq %[zero_reference], %%r14            \n\t" /* r[6] += flag_o                                          */  \
        "adoxq %[zero_reference], %%r14            \n\t" /* r[6] += flag_c                                          */  \
        "adcxq %%r8, %%r15                         \n\t" /* r[2] += t[0] + flag_c                                   */  \
        "adoxq %%r9, %%r10                         \n\t" /* r[3] += t[1] + flag_o                                   */  \
                                                                                                                        \
        /* reduce by r[2] * k */                                                                                        \
        "movq %%r15, %%rdx                         \n\t"  /* move r[2] into %rdx                                    */  \
        "mulxq %[r_inv], %%rdx, %%r8               \n\t"  /* (%rdx, _) <- k = r[1] * r_inv                          */  \
        "mulxq %[modulus_1], %%rdi, %%r11          \n\t"  /* (t[0], t[1]) <- (modulus.data[1] * k)                  */  \
        "mulxq %[modulus_2], %%r8, %%r9            \n\t"  /* (t[0], t[1]) <- (modulus.data[2] * k)                  */  \
        "adcxq %%rdi, %%r10                        \n\t"  /* r[3] += t[1] + flag_o                                  */  \
        "adoxq %%r11, %%r12                        \n\t"  /* r[4] += t[1] + flag_c                                  */  \
        "adcxq %%r8, %%r12                         \n\t"  /* r[4] += t[0] + flag_o                                  */  \
        "adoxq %%r9, %%r13                         \n\t"  /* r[5] += t[2] + flag_c                                  */  \
        "mulxq %[modulus_3], %%rdi, %%r11          \n\t"  /* (t[2], t[3]) <- (modulus.data[3] * k)                  */  \
        "mulxq %[modulus_0], %%r8, %%r9            \n\t"  /* (t[0], t[1]) <- (modulus.data[0] * k)                  */  \
        "adcxq %%rdi, %%r13                        \n\t"  /* r[5] += t[1] + flag_o                                  */  \
        "adoxq %%r11, %%r14                        \n\t"  /* r[6] += t[3] + flag_c                                  */  \
        "adcxq %[zero_reference], %%r14            \n\t"  /* r[6] += flag_o                                         */  \
        "adoxq %%r8, %%r15                         \n\t"  /* r[2] += t[0] (%r15 now free)                           */  \
        "adcxq %%r9, %%r10                         \n\t"  /* r[3] += t[0] + flag_c                                  */  \
                                                                                                                        \
        /* a[3] * b */                                                                                                  \
        "movq 24(%%rax), %%rdx                     \n\t"  /* load a[3] into %rdx                                    */  \
        "mulxq 0(%%rbx), %%r8, %%r9                \n\t"  /* (t[0], t[1]) <- (a[3] * b[0])                          */  \
        "mulxq 8(%%rbx), %%rdi, %%r11              \n\t"  /* (t[4], t[5]) <- (a[3] * b[1])                          */  \
        "adoxq %%r8, %%r10                         \n\t"  /* r[3] += t[0] + flag_c                                  */  \
        "adcxq %%r9, %%r12                         \n\t"  /* r[4] += t[2] + flag_o                                  */  \
        "adoxq %%rdi, %%r12                        \n\t"  /* r[4] += t[1] + flag_c                                  */  \
        "adcxq %%r11, %%r13                        \n\t"  /* r[5] += t[3] + flag_o                                  */  \
                                                                                                                        \
        "mulxq 16(%%rbx), %%r8, %%r9               \n\t"  /* (t[2], t[3]) <- (a[3] * b[2])                          */  \
        "mulxq 24(%%rbx), %%rdi, %%r15             \n\t"  /* (t[6], r[7]) <- (a[3] * b[3])                          */  \
        "adoxq %%r8, %%r13                         \n\t"  /* r[5] += t[4] + flag_c                                  */  \
        "adcxq %%r9, %%r14                         \n\t"  /* r[6] += t[6] + flag_o                                  */  \
        "adoxq %%rdi, %%r14                        \n\t"  /* r[6] += t[5] + flag_c                                  */  \
                                                                                                                        \
        /* reduce by r[3] * k */                                                                                        \
        "movq %%r10, %%rdx                         \n\t" /* move r_inv into %rdx                                    */  \
        "mulxq %[r_inv], %%rdx, %%r8               \n\t" /* (%rdx, _) <- k = r[1] * r_inv                           */  \
        "mulxq %[modulus_0], %%r8, %%r9            \n\t" /* (t[0], t[1]) <- (modulus.data[0] * k)                   */  \
        "mulxq %[modulus_1], %%rdi, %%r11          \n\t" /* (t[2], t[3]) <- (modulus.data[1] * k)                   */  \
        "adoxq %%r8, %%r10                         \n\t" /* r[3] += t[0] (%rsi now free)                            */  \
        "adcxq %%r9, %%r12                         \n\t" /* r[4] += t[2] + flag_c                                   */  \
        "adoxq %%rdi, %%r12                        \n\t" /* r[4] += t[1] + flag_o                                   */  \
        "adcxq %%r11, %%r13                        \n\t" /* r[5] += t[3] + flag_c                                   */  \
                                                                                                                        \
        "mulxq %[modulus_2], %%r8, %%r9            \n\t" /* (t[4], t[5]) <- (modulus.data[2] * k)                   */  \
        "mulxq %[modulus_3], %%rdi, %%rdx          \n\t" /* (t[6], t[7]) <- (modulus.data[3] * k)                   */  \
        "adoxq %%r8, %%r13                         \n\t" /* r[5] += t[4] + flag_o                                   */  \
        "adcxq %%r9, %%r14                         \n\t" /* r[6] += t[6] + flag_c                                   */  \
        "adoxq %%rdi, %%r14                        \n\t" /* r[6] += t[5] + flag_o                                   */  \
        "adcxq %%rdx, %%r15                        \n\t" /* r[7] += t[7] + flag_c                                   */  \
        "movq %%r12, 0(%%rcx)                      \n\t"                                                                \
        "movq %%r13, 8(%%rcx)                      \n\t"                                                                \ 
        "movq %%r14, 16(%%rcx)                     \n\t"                                                                \
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
