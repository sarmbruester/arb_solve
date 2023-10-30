# Solving Large Linear Equations in Arbitrary Precision

There are a few Python libraries that let you do math computations in arbitrary precision like
`mpmath`. Though there are also slightly faster libraries like `flamp` or Python versions of
`flint` around, I personally could never come close to the speed of the C libraries Arb and Flint
when solving larger systems of (complex-valued) linear equations A * x = b in Python.

Therefore, I decided to come up with a workaround Python integration where you have a Python
script that dumps in some directory (e.g. the entry of an environment variable SOLVE_DIR) the
real- or complex-valued matrix A to a file `a.mtx` in matrix-market format via

`scipy.io.mmwrite('a', a, field='complex', precision=18, symmetry='general')`

and the complex-valued resultant vector b to a file `b.txt` via

`numpy.savetxt('b.txt', b, fmt='%.18e %.18e', header=str(b.shape[0]))`

then call the compiled `arb_solve.c` that natively uses C libraries Arb and Flint, as a process,

finally reading the complex-valued solution vector x from a file `x.txt` via

```x = numpy.loadtxt('x.txt')```

followed by

```x = x[:, 0] + 1j * x[:, 1]```.

Though the overhead for file IO is significant for smaller systems, it becomes insignificant for
larger ones. In case you have troubles installing Arb and Flint, there are ready-made Conda
packages available.
