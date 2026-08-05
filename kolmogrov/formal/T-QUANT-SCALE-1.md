# T-QUANT-SCALE-1: sign quantization obstructs scale contraction

Status: **HYPOTHESIS with complete impossibility proof and margin-qualified
fuzzy replacement; independent review and formal encoding absent**.

## Impossibility statement

Let a fine block contain at least two unrestricted nonzero real coefficients,
let its coarse coefficient be their positive weighted sum, and let the fine
hash retain only coefficient signs. No deterministic function of those fine
signs can recover the coarse sign for every fine coefficient vector.

## Proof

It suffices to use two positive weights; absorb them into coefficient
magnitudes. The vectors `(100,-99)` and `(1,-100)` have identical fine signs
`(+,-)` but their sums have opposite signs. Any deterministic function of the
fine signs returns the same answer on both and is wrong on one. Extra fine
coordinates may be fixed identically, so the obstruction holds for larger
blocks. QED.

Therefore exact pre-quantized consistency `Phi_m=A_m Phi_(m+1)` does not imply a
binary contraction `h_m=C_m h_(m+1)`.

## Margin-qualified fuzzy statement

Let `A` be an `l_infinity` contraction, let `z` be the fine state, and let
`z_hat` satisfy `||z-z_hat||_infinity<=epsilon`. Then

```text
||Az-Az_hat||_infinity <= epsilon.
```

For any coarse coordinate `j`, if `|(Az_hat)_j|>epsilon`, its sign equals the
sign of `(Az)_j`.

### Proof

The contraction gives the error bound. If the approximate value is more than
`epsilon` from zero, an error of at most `epsilon` cannot cross zero. QED.

## Minimum extra support question

Sign-only fine state is insufficient. Exact recovery over unrestricted real
amplitudes requires additional magnitude/order information or an independently
stored coarse sign. Over a bounded quantized amplitude alphabet, the minimum
certificate becomes a finite coding problem; it is not solved here. The fuzzy
version needs only enough support to certify an interval excluding zero, but
the required bits depend on amplitude range and error tolerance.
