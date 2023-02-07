## EC-FFT Test

Hi there! Welcome to the `ec_fft` test that you are about to take. We will guide you through the concept of EC-FFT before you begin crushing this exercise.

#### Monomial Reference String

Universal zk-SNARKs like PlonK need to run a one-time trusted setup ceremony to generate a Structured Reference String (SRS). Any number of participants can participate in this ceremony but only _one_ of all the participants need to be _honest_. As a part of this ceremony, each participant contributes to the setup with their own _secret_ which they are free to choose. Even if one of the participants generates this secret randomly and destroy it successfully, the setup ceremony is considered to be successful. The output of the setup ceremony is a structure reference string of the form:

$$
\begin{aligned}
\mathbb{G}_1 \text{ points: }
\big\{[1]_1,[x]_1, [x^2]_1, [x^3]_1, \dots, [x^{N-1}]_1\big\}, \\
\mathbb{G}_2 \text{ points: }
\big\{[1]_2,[x]_2, [x^2]_2, [x^3]_2, \dots, [x^{M-1}]_2\big\}.
\end{aligned}
$$

Here, $x \in \mathbb{F}$ is the combined secret of all the participants and is assumed to be unknown to anyone in the world. Further, the notation $[a]_1 := aG_1$ such that $G_1\in \mathbb{G}_1$ is generator for the first group $\mathbb{G}_1$.

The monomial reference string is used to commit to polynomials in their coefficient/monomial form. For example, given a polynomial $f(X) = f_0 + f_1X + f_2X^2 + \dots + f_{n-1}X^{n-1}$ for $n < N$, we can compute its commitment as:

$$
\textsf{commit}(f) := [f(x)]_1 = \sum_{i=0}^{n-1} f_i \cdot [x^i]_1.
$$

#### Lagrange Representation

An alternative way to represent a polynomial is in its Lagrange form. Let $\{\omega^0, \omega^1, \omega^2, \dots, \omega^{n-1}\}$ be the $n$-th roots of unity and assume $n$ is a power of two. Then we can write the same polynomial $f(X)$ as:

$$
f(X) = \sum_{i=0}^{n-1} f(\omega^i) \cdot L_{n,i}(X)
$$

where $L_i(X)$ is the $n$-th Lagrange basis polynomial defined as:

$$
L_{n,i}(\omega^j) =
\begin{cases}
    1 & \text{if }j = i \\
    0 & \text{otherwise}
\end{cases}.
$$

In other words, the Lagrange basis polynomial $L_{n,i}(X)$ is $1$ on $\omega^i$ and $0$ on the other roots $\{\omega^j\}_{j \neq i}$. It is sometimes useful to work with the Lagrange form of a polynomial than its coefficient form.

#### Fast Fourier Transform

Given the coefficent form of a polynomial, it is possible to convert it to the Lagrange form using the Discrete Fourier Transform (DFT) operation. Similarly, we can take an inverse DFT to convert the Lagrange form to its coefficient form.

$$
\begin{aligned}
    \{f_i\}_{i=0}^{n} \xrightarrow{\textsf{FFT}} \{f(\omega^i)\}_{i=0}^{n}, \\
\{f(\omega^i)\}_{i=0}^{n} \xrightarrow{\textsf{iFFT}} \{f_i\}_{i=0}^{n}.
\end{aligned}
$$

Note that this $\textsf{FFT}$ operation is defined on scalars in the field $\mathbb{F}$.

#### EC-FFT

Suppose we are given a bunch of elliptic curve points:

$$
\{a_0G_1, a_1G_1, \dots, a_{n-1}G_1\} \in \mathbb{G}_1^{n}
$$

for some scalars $\{a_i\}_{i=0}^{n-1}\in \mathbb{F}^n$. You can think of these scalars $\{a_i\}_{i=0}^{n-1}$ as coefficients of some polynomial $A(X)$. Note that you only have access to the given curve points and not the actual coefficients $\{a_i\}_{i=0}^{n-1}$. The question is: can you convert this set of curve points to another set of curve points defined as:

$$
\{A(\omega^0)\cdot G_1, \ A(\omega^1)\cdot G_1, \ \dots,  \ A(\omega^{n-1})\cdot G_1\} \in \mathbb{G}_1^{n}.
$$

Since you do not have access to the coefficient $\{a_i\}_{i=0}^{n-1}$ you cannot just compute its FFT. But instead, if we can do an FFT operation on the curve points, that will give us the desried result. EC-FFT is exactly that: FFT on Elliptic Curve points!

In this exercise, you have to implement the function `ec_fft` that takes in a set of points `g1_elements` and modifies the same points to get the FFT form.

#### Application of EC-FFT

Recall that our monomial SRS (of size $n$) was of the form:

$$
\mathbb{G}_1 \text{ monomial points: }
\big\{[1]_1,[x]_1, [x^2]_1, [x^3]_1, \dots, [x^{n-1}]_1\big\}.
$$

Note that we are interested only in the SRS of the first group $\mathbb{G}_1$. Lets say we want to convert this monomial SRS to the Lagrange SRS:

$$
\mathbb{G}_1 \text{ lagrange points: }
\big\{[L_0(x)]_1,[L_1(x)]_1, [L_2(x)]_1, \dots, [L_{n-1}(x)]_1\big\}.
$$

without knowing the scalar $x\in \mathbb{F}$. We can do this by using EC-FFT functionality. Define a polynomial with coefficients $\{1, x, x^2, \dots, x^{n-1}\}$:

$$
P(Y) := 1 + xY + x^2Y^2 + \dots + x^{n-1}Y^{n-1}.
$$

Now, we can write the Lagrange polynomial $L_{n,i}(X)$ as:

$$
L_{n,i}(X) := \frac{1}{n}\left( \left(\frac{X}{\omega^i}\right)^0 + \left(\frac{X}{\omega^i}\right)^1 + \dots + \left(\frac{X}{\omega^i}\right)^{n-1} \right)
$$

This is because: when $X=\omega^i$ all of the terms would be 1 and so $L_{n,i}(\omega^i)=\frac{1 + 1 + \dots + 1}{n} = 1$. On the other hand, if $X = \omega^j$ s.t. $j\neq i$, the term in the numerator would just be the sum of all $n$-th roots of unity, i.e. $L_{n,i}(\omega^j)=\frac{\sum_k\omega^k}{n} = 0$. Using this definition of the Lagrange polynomial, we have:

$$
L_{n,i}(X) := \frac{1}{n}\sum_{j=0}^{n-1}\left(\frac{X}{\omega^i}\right)^j = \frac{1}{n}\sum_{j=0}^{n-1}\left(\omega^{-i}X\right)^j.
$$

By the definition of $P(Y)$, we can write:

$$
\begin{aligned}
P(Y) &= \sum_{j=0}^{n-1} (x \cdot Y)^j \\
\implies P(\omega^{-i}) &= \sum_{j=0}^{n-1} (\omega^{-i} \cdot x)^j =: n \cdot L_{n,i}(x) \\
\therefore \quad L_{n,i}(x) &:= \frac{1}{n} \cdot P(\omega^{-i}) \qquad \text{(1)}
\end{aligned}
$$

Therefore, we can compute the Lagrange SRS from the monomial SRS by first taking the EC-FFT on the monomial SRS and applying the transform shown in equation $(1)$.

As a part of this exercise, you will also implement the function `convert_srs` that takes in a `monomial_srs` and converts it to a `lagrange_srs` for a given size.