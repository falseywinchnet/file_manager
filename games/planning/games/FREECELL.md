# FreeCell plan

Status: **GIVEN module; exact move-assistance policy open**.

## Purpose

Provide a highly inspectable open-information solitaire game and test precise
multi-card movement, card animation, keyboard planning and deterministic deal
replay without hidden stock state.

## Initial rules candidate

- standard 52-card deck, eight cascades, four free cells and four foundations;
- alternating-color descending cascades;
- multi-card movement is legal only when equivalent sequential single-card
  moves exist given empty cells/cascades;
- classic numbered-deal compatibility is optional and must not be claimed until
  its shuffle is reproduced exactly.

## Model and oracle

The model stores piles and card identity only. A transparent capacity function
explains movable-run length. Reference fixtures exhaustively compare legal
moves on bounded subpositions and verify that assisted supermoves decompose into
legal primitive moves.

## Primary surface and dialogs

One card table, compact menu/status, no panels. New Deal/seed, Rules,
Hint/explanation and result/statistics are owned dialogs. Hint explanations may
name blocked free cells and required empty cascades.

## Interaction and animation

- drag or source/destination keyboard moves;
- optional supermove animation shows the legal decomposition without making the
  user perform every primitive move;
- reduced motion commits the same move instantly with a short destination cue;
- undo scope and auto-foundation policy require interview closure.

## Dogfood and evidence

- deterministic solvable/unsolved/prepared deals;
- legal capacity edge cases with zero through four free cells;
- interrupted supermove and focus restoration;
- keyboard-only sequence and screen-reader pile/card descriptions;
- solver may be used offline to label fixtures, but no claim that arbitrary
  deals are solved optimally without a certificate.

## Exclusions

No online deal of the day, competitive timing service, solver-as-attraction, or
hidden dynamic difficulty.
