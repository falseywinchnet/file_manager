# T-QUANT-CERT-1: sharp interval certificate for coarse quantized sign

Status: **HYPOTHESIS with complete paper proof; concrete quantizer absent**.

## Exact certificate

Let a coarse coefficient be a positive contraction

```text
s = sum_i alpha_i z_i,  alpha_i>=0.
```

Suppose the quantizer reports only that `z_i` lies in interval `[l_i,u_i]`.
Then the exact attainable coarse interval is

```text
[L,U] = [sum_i alpha_i l_i, sum_i alpha_i u_i].
```

The coarse sign is certified positive iff `L>0`, negative iff `U<0`, and is
otherwise uncertified. This criterion is sharp.

### Proof

Positive linearity makes every feasible sum lie in `[L,U]`; choosing all lower
or all upper endpoints attains its endpoints. If the interval crosses or
touches zero, feasible amplitudes do not force one strict sign. QED.

For reconstructions `zhat_i` with errors `|z_i-zhat_i|<=delta_i`, the familiar
sufficient-and-necessary interval test is

```text
|sum_i alpha_i zhat_i| > sum_i alpha_i delta_i.
```

## Least sign-only obstruction

For every `eta>0`, the two fine vectors

```text
z+ = (1+eta, -1),
z- = (1, -1-eta)
```

have the same fine sign pattern `(+,-)` but their coarse sums have signs `+`
and `-`. Hence no sign-only parent rule can be exact on unrestricted
amplitudes.

One fine coordinate is not an obstruction: a positive one-coordinate
contraction preserves its strict sign. Two coordinates are therefore least
support.

## Quantized-flow rule

Quantize only after the pre-quantized contraction/flow has been fixed. Every
derived coarse bit carries one of three states:

```text
positive, negative, uncertified.
```

Reporting an uncertified bit as stable is a theorem violation, even if aggregate
retrieval is unchanged. Serialized support must count any retained interval or
margin certificate.

## Falsification gate

A quantizer fails scale consistency on the first block whose feasible interval
contains both signs while the representation emits a definite inherited coarse
bit. A margin scheme fails economically when its certificates cost at least as
much support as storing the required coarse coefficients directly.
