# T-ENERGY-INFORMATION-1: least separation of detail energy and relevance

Status: **HYPOTHESIS with complete paper obstruction; probabilistic bridge
model absent**.

## Statement

Neither detail energy nor even the complete marginal law of a detail coefficient
determines its relevant information about a target.

Let target `Y` be uniform on `{-1,+1}` and let the coarse state be constant.
Compare two detail channels:

```text
informative:    Z=Y,
uninformative:  Z=N, where N is independent uniform {-1,+1}.
```

Both have the same marginal distribution, all the same marginal moments, and

```text
E[Z^2]=1.
```

But their relevant informations are

```text
I(Y;Z)=1 bit
I(Y;Z)=0 bits,
```

respectively.

Conversely, `Z=aY` has one bit of relevant information for every nonzero `a`,
while its energy is `a^2` and can be arbitrarily small or large.

## Proof

In the informative channel, `Y` is exactly decoded from `Z`, so conditional
entropy is zero. In the independent channel, observing `Z` leaves the uniform
law of `Y` unchanged. Nonzero scalar multiplication is bijective on the two
detail values and therefore preserves the exact decoding while changing energy.
QED.

## Minimality

Nonzero mutual information requires at least two target states and at least two
observable detail states. The construction uses exactly two of each, so it is a
least-support obstruction.

## Consequence

Orthogonal detail energy is an exact geometric accounting quantity; conditional
mutual information is a joint target/distribution quantity. Neither may be used
as a proxy for the other.

Any bridge must additionally declare the conditional observation laws, target
prior, noise mechanism, and task readout. SNR can enter such a model, but total
energy and a noise scalar alone do not identify the joint coupling.

## Falsification gate

Reject every pruning rule that removes a block solely because its energy is
small while making a relevance claim, unless a proved observation model maps
that energy bound to the declared information or task loss. Report the energy
and relevant-information ledgers independently until that bridge exists.
