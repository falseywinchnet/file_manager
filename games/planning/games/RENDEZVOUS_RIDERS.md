# Rendezvous Riders plan

Status: **GIVEN two-motorcycle route puzzle; public name and objective details
open**.

## Purpose

Make an animation-rich shortest-route fidget game in which two tiny motorcycles
advance alternately from an origin and destination until they meet. The player
tries to equal the optimal move/cost certificate. This is a bidirectional
shortest-path puzzle, not a traveling-salesman problem.

## Candidate rules

- a finite connected graph is drawn as roads, junctions and positive edge
  costs;
- one rider starts at origin and one at destination;
- turns alternate; the player selects one adjacent edge for the active rider;
- riders meet at a vertex, or on an edge only if that rule is explicitly
  admitted;
- score compares total travelled cost first and perhaps move count second;
- revisits, one-way roads, equal-cost edges, hidden costs and dead ends require
  explicit profiles.

## Reference solution

- bidirectional breadth-first search certifies equal-cost boards;
- bidirectional Dijkstra certifies positive weighted boards;
- a simple single-source Dijkstra reference cross-checks optimal distance;
- generator retains only connected boards with a recorded optimum and declared
  uniqueness/multiple-optimum state;
- “optimal” appears only for the exact displayed cost objective, never by visual
  route length or an uncertified heuristic.

The player's alternating two-ended constraint may differ from ordinary
shortest-path distance. If so, define the joint state `(left, right, turn)` and
solve that exact state graph rather than claiming Dijkstra alone proves it.

## Surface and dialogs

One map, two riders and compact cost/move status, no panels. New Map/difficulty,
Rules, Route Explanation and result-comparison dialogs are owned popups. The
optimal route remains hidden until requested or the game ends.

## Animation

Riders traverse selected edges with bounded, cancellable motion. Logic commits
the destination junction first. Reduced motion moves instantly with a route
highlight. Animation duration is presentation-only and never affects score.

## Dogfood and evidence

- fixed equal-cost, weighted, multiple-optimum, disconnected-rejection and
  alternating-constraint fixtures;
- generator seed replay and certificate verification;
- keyboard junction selection and non-color rider distinction;
- undo/new-game/close during travel animation;
- graph/path drawing damage, scaling, labels and overlap;
- exact result explanation showing player cost versus certified optimum.

## Exclusions

No misleading traveling-salesman terminology, real-world map/network service,
GPS, online races, physics simulation, fuel purchases, leaderboard or heuristic
optimality claim without a certificate.
