# Malkuth release manifest and acceptance

Status: **CANDIDATE schema; DECIDED evidence procedure**.

## Manifest purpose

One release object states exactly what “Malkuth X.Y” contains and supports. It
does not make every component use the suite version.

## Required manifest sections

```text
suite identity/version/channel/date
mission/narrative revision
applications and component artifacts
independently available game modules, lexical corpus packages, and first-party plugins
component semantic/ABI/protocol/schema ranges
platform/architecture/minimum-version matrix
feature/capability availability by artifact
installer/package revisions and lifecycle behavior
documentation and website revisions
migrations/rebuilds/downgrade policy
signatures, hashes, provenance, license inventory, SBOM
native dogfood/conformance/benchmark/security evidence
known issues, carried risks, exclusions, recovery paths
release owner approval
```

## Evidence classes

- **native release evidence:** shipping-candidate artifact on supported native
  hardware/OS/filesystem;
- **compatibility evidence:** Wine, VM translation, emulation, cross-build or
  alternate filesystem—useful but not native support;
- **fixture evidence:** deterministic fake/provider/headless behavior;
- **planning evidence:** design/contract intent only.

Public support and stability claims use the first class unless the claim is
explicitly narrower.

For Games, the manifest names each available module plus rules/save/generator
versions. For Lexicon, it names each exact corpus/source package and license.
For Archive Viewer/Image Converter, it names plugin, external-engine/decoder,
grant profile and disabled-by-default state independently.

## Release blocker classes

Always blocking absent explicit architect exception:

- wrong-object mutation, content loss, unrecoverable corruption, privilege
  escalation, authentication/capability bypass, secret/signing-key exposure;
- installer targets the wrong path/user/system service or uninstall destroys
  retained user data without consent;
- ordinary input deadlock/hang, irrecoverable service loop, incompatible upgrade
  without recovery, or supported-platform launch failure;
- website/download points to an unmanifested or mismatched artifact;
- documentation directs a destructive/privileged action with wrong scope;
- advertised privacy/security/accessibility/platform claim contradicted by the
  shipping artifact.

Other defects receive severity, frequency, affected matrix, workaround,
visibility, and owner disposition. “As many bugs as possible” becomes a bounded
burn-down with no unowned severe defect, not a claim of zero bugs.

## Release-candidate sequence

1. Freeze candidate component/artifact revisions and compatibility ranges.
2. Build through the declared clean/reproducible lanes.
3. Generate manifest, SBOM, licenses, hashes, signatures and provenance.
4. Install on the clean native matrix.
5. Run daily workflows, fault/security/accessibility/performance/upgrade suites.
6. Build docs and website from the same candidate manifest.
7. Re-run download/install documentation literally as written.
8. Triage every difference, rebuild if artifact bytes change, and restart the
   affected evidence lanes.
9. Grand architect approves the final manifest, exclusions and carried risks.
10. Publish artifacts, docs, site and release record atomically or through a
    rehearsed staged order with rollback.

## Compatibility law

Component ranges are explicit. A suite installer refuses an incompatible mix or
offers a named repair/migration path; it does not guess. 2.0 internal tightening
must keep existing calls stable or ship tested compatibility adapters as
required by ADR-012.

## Open schema choices

- serialization/format and signing envelope;
- artifact transparency/reproducibility service;
- channel model: nightly/development/beta/stable/LTS;
- supported previous release and rollback windows;
- issue/advisory publication and incident response;
- exact quantitative 1.0 stability and dogfood thresholds.
