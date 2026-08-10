------------------------- MODULE atomic_batch_pool -------------------------
EXTENDS Naturals, FiniteSets, TLC

CONSTANT N, Workers, K
ASSUME N \in Nat /\ N > 0 /\ Workers \in Nat /\ Workers > 0
       /\ K \in Nat /\ K > 0

VARIABLES remaining, pending, claimed, completed
vars == <<remaining, pending, claimed, completed>>

Init ==
    /\ remaining = N
    /\ pending = N
    /\ claimed = [w \in 1..Workers |-> {}]
    /\ completed = {}

Min(a, b) == IF a < b THEN a ELSE b

Claim(w, amount) ==
    /\ remaining > 0
    /\ amount \in 1..K
    /\ LET take == Min(amount, remaining)
           tickets == (remaining - take + 1)..remaining IN
       /\ \A other \in 1..Workers : tickets \cap claimed[other] = {}
       /\ claimed' = [claimed EXCEPT ![w] = @ \cup tickets]
       /\ remaining' = remaining - take
    /\ UNCHANGED <<pending, completed>>

Complete(w, ticket) ==
    /\ ticket \in claimed[w]
    /\ ticket \notin completed
    /\ completed' = completed \cup {ticket}
    /\ pending' = pending - 1
    /\ UNCHANGED <<remaining, claimed>>

Next ==
    \/ \E w \in 1..Workers, amount \in 1..K : Claim(w, amount)
    \/ \E w \in 1..Workers, ticket \in 1..N : Complete(w, ticket)

TypeOK ==
    /\ remaining \in 0..N
    /\ pending \in 0..N
    /\ completed \subseteq 1..N

UniqueClaims ==
    \A a, b \in 1..Workers : a # b => claimed[a] \cap claimed[b] = {}

CompletionAccounting == pending = N - Cardinality(completed)

FullCoverageAfterClaims == remaining = 0 =>
    UNION {claimed[w] : w \in 1..Workers} = 1..N

Spec == Init /\ [][Next]_vars
=============================================================================
