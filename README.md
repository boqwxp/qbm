# qbm - Quantified Boolean Matching
qbm is an exploration tool for the low-level digit design.
It performs boolean matching on the basis of quantified boolean formulae
to compute or to disprove the existence of a configuration for a configurable
hardware structure that implements a specific user function.

## Features
qbm features a generic, i.e. parameterized, specification of typically configurable logic structures that are compiled into a concrete component
hierarchy for evaluation purposes. The compilation computes a set of
[clauses](https://en.wikipedia.org/wiki/Clause_%28logic%29)
reflecting the functional behavior of the built circuitry. qbm then utilizes
the [QBF](https://en.wikipedia.org/wiki/True_quantified_Boolean_formula)
solver [Quantor](http://fmv.jku.at/quantor/) to determine an assignment to the available configuration variables that implements the desired user function if this is, at all, possible. If qbm cannot implement the desired function within the available circuit, this is, indeed, impossible. qbm thus implements a formal synthesis flow for combinational logic.

The fastest way to specify mapping tasks is by using the included description language QDL.

Basic Example:
```C
// Generic description of a LUT with K inputs
component LUT<K>(x[K] -> y)
  config c[2**K];
  y = c[x];
end;

// The top-level component instantiating a LUT and
// specifying the desired user function.
component top(x[3] -> y)
  lut_0 : LUT<3>(x -> y);
  y = x[2] ^ x[1] ^ x[0];
end;
```

## Code Structure
* lib/       - the place where Quantor and PicoSat are downloaded and compiled
* src/model/ - the circuit representation
* src/qdl/   - the QDL frontend supporting a command-line work flow
* models/    - example QDL models
* bin/       - directory created when building binaries

## Quick Start
```bash
> git clone https://github.com/preusser/qbm.git
> make
> bin/qdlsolve < models/test.qdl
Compiling <top> ...
Compiling lut_s0 ...
Compiling lut_s1 ...
Compiling lut_s2 ...
Compiling fct ...
SAT
/fct:ADD
/lut_s0:LUT
        c = "0101101001011010";
/lut_s1:LUT
        c = "1001001101101100";
/lut_s2:LUT
        c = "1110110010000000";
```
In essence, the
[provided example](https://github.com/preusser/qbm/blob/master/models/test.qdl)
computes the truth tables for the individual
output bits of an adder for two 2-bit operands.
