# Games — Orchestrator interface negotiation

Status: **intake only; no runtime contract admitted**.

## Expected existing families

- `ORC-APP-001`: Games package/module identities and truthful availability;
- `ORC-SET-001`: namespaced local preferences and optional factual statistics;
- `ORC-HLP-001`: bundled rules/help topic registration and locale fallback.

No new game-specific service family is proposed. Rules, boards, solvers,
opponents, seeds, saves and animation stay inside Games. No Engine, Kolmogrov,
hive, plugin, federation, network or Document Picker dependency is inferred.

## Questions for later reconciliation

1. Whether modules are separate `ApplicationId`s or capabilities beneath one
   Games application identity.
2. Whether save files use ordinary handler registration or application-private
   state only.
3. Whether local statistics are settings values or a bounded application-owned
   document.
4. How package manifests represent independently available game modules.

The Games project replies only after GA0 closes.
