# Malkuth mission and intent framing

Date: 2026-08-06.

Status: **DECIDED mission core; supporting copy remains editable**.

## Name

**Malkuth** is the suite/distribution identity because it governs the physical
existence of files and reveals their provenance and location.

The name describes the suite's concern with the tangible file world. It does
not replace the filesystem as authority and does not imply surveillance,
automatic cloud ingestion, or metaphysical claims in technical interfaces.

## Mission

> **Curious, Concise, Friendly.**
>
> Malkuth is curious about the files that physically exist: where they are, how
> they arrived, what can act on them, and which evidence supports that account.
> It is concise in the commands, explanations, and machinery it places between
> a person and their work. It is friendly by being quick, legible, accessible,
> reversible where promised, and honest about what it knows. It reveals
> provenance and location without turning the desktop into a feed, a cloud
> account, or a browser.

## Three promises

### Curious

- reveal location, provenance, handler, availability, and search evidence;
- let users inspect why a result or state exists;
- ask before admitting roots, remote machines, plugins, or durable derived data;
- preserve ambiguity and uncertainty instead of inventing certainty;
- make help contextual and discoverable without making the application noisy.

### Concise

- one clear probable path before optional depth;
- bounded commands, labels, explanations, dialogs, settings, and APIs;
- no tabs/feeds/cloud dashboards merely because competitors have them;
- no duplicate service, index, picker, settings, or presentation authorities;
- performance work removes measured machinery rather than adding clever layers.

### Friendly

- fast input response and low idle intrusion;
- ordinary desktop behavior with authored, legible appearance;
- accessible controls, keyboard paths, contextual help, and honest errors;
- reversibility only where genuinely promised;
- local-first privacy, explicit consent, clean installation, and clean removal;
- degraded capability is named and useful rather than frozen or deceptive.

## Audience framing

Malkuth is for people who want the computer to feel like a place they inhabit,
not a storefront or an activity feed. Power users receive direct paths and
inspectable evidence without requiring every ordinary user to understand an
index, ABI, hive, plugin supervisor, or file identity model.

## Public voice

- direct, specific, warm, and technically literate;
- never twee, mystical, breathless, sarcastic, or enterprise-anonymous;
- demonstrate rather than call the product revolutionary;
- prefer “this result came from…” over “AI-powered”;
- prefer “works without indexing” over vague offline slogans;
- name limits, unavailable systems, supported platforms, and migration paths.

## Concise capability shape

ADR-013 applies the mission to suite growth:

- make a separate application only when a capability owns a durable primary
  interaction or document model;
- make an owned popup dialog when a tool exists to assist one application;
- make a typed provider when a capability contributes structured local
  knowledge;
- make a trusted contextual command when the operation is small, first-party,
  and filesystem-adjacent;
- make a supervised plugin when hostile formats or external engines need reef
  containment.

This lets Malkuth remain curious without becoming a miscellaneous-utilities
drawer. Persistent side panels remain specific to File Manager and its embedded
picker/browser; other applications keep one clear primary surface.

## Short copy candidates

These remain **CANDIDATE** website/installer lines:

- “Your files, where they are, and how they got there.”
- “A local desktop suite that stays curious and gets out of the way.”
- “See what exists. Know where it came from. Act without the clutter.”

## Claims law

“Fast,” “stable,” “cross-platform,” “secure,” “private,” “accessible,”
“verified,” and “compatible” appear publicly only beside the release artifact
and evidence scope that support them. Malkuth's friendly voice never excuses an
unsupported claim.
