# T-APPROX-CLOSURE-1: weighted closure and its least rare-state obstruction

Status: **HYPOTHESIS with complete paper proof; task calibration absent**.

## Weighted residual

For fine state `s`, let `q_s` be the true distribution of the next coarse cell
under one action and let `qbar_[s]` be the proposed closed coarse law for its
current cell. Under fine-state distribution `pi`, define

```text
epsilon_pi = sum_s pi(s) TV(q_s,qbar_[s]).
```

For several actions, retain the residual separately by action or take a
predeclared weighted maximum. Do not average action failures without a policy
that assigns those weights.

## Positive bound

If a next-state task functional `f` is `L`-Lipschitz in total variation, then

```text
E_pi |f(q_s)-f(qbar_[s])| <= L epsilon_pi.
```

### Proof

Apply the Lipschitz inequality statewise, multiply by `pi(s)`, and sum. QED.

This controls an average under the same distribution and only for a Lipschitz
readout. It does not give worst-state or rare-class recall.

## Least rare-state obstruction

Three fine states suffice, and two cannot. Let

```text
S={a,b,c},  cells B={a,b}, C={c},
T(a)=a, T(b)=c, T(c)=c,
pi(a)=1-epsilon, pi(b)=epsilon, pi(c)=0,
0<epsilon<1/2.
```

The optimal coarse law from `B` predicts `B`: choosing probability `r` of `C`
has weighted residual

```text
(1-epsilon)r + epsilon(1-r),
```

minimized at `r=0`, with value `epsilon`. This can be arbitrarily small, while
the conditional prediction for state `b` is wrong with probability one. A hard
negative concentrated on `b` therefore has zero conditional recall.

The obstruction needs at least three fine states: two states must be merged to
create hidden within-cell behavior, and at least one state in a different
observable cell is required for those behaviors to have different coarse
targets. The construction attains that bound.

## Minimal support curve

For this system:

| Coarse cells | Weighted dynamic residual | Conditional recall on `b` |
|---:|---:|---:|
| 2: `{a,b}`, `{c}` | `epsilon` | 0 |
| 3: `{a}`, `{b}`, `{c}` | 0 | 1 |

One cell is inadmissible because it erases the required `B/C` observation.

## Required guard

An approximate support claim must pair average residual with at least one of:

- a supremum or high-quantile statewise residual;
- minimum protected-class mass and a conditional bound;
- an explicit hard-negative set;
- a task margin making the decision readout stable.

Without such a guard, small average flow residual is compatible with complete
failure on the epistemically important state.
