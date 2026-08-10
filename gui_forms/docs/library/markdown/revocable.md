# Revocable

- Status: **OBSERVED: bundle 010 revocation contract split; focused M4 lifecycle tests pass**
- Kind: **class**
- Hierarchy: `Revocable`
- Declaration: `include/gui_forms/component/revocable/revocable.hpp:5`
- Definition: `inline/header-only`

Revocable is the minimal lifetime authority owned weakly by Component for subscriptions, timers, and queued work.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `~Revocable` (public)

```cpp
virtual ~Revocable() = default
```

Provides polymorphic destruction.

### `disconnect` (public)

```cpp
virtual void disconnect() noexcept = 0
```

Idempotently revokes the underlying work or observation.

### `connected` (public)

```cpp
[[nodiscard]] virtual bool connected() const noexcept = 0
```

Reports whether revocable work remains active.
