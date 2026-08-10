# HelpProvider

- Status: **OBSERVED: bundle 005 split and policy review; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class**
- Hierarchy: `Component → HelpProvider`
- Declaration: `include/gui_forms/components/help_provider/help_provider.hpp:60`
- Definition: `src/controls/guidance/help_provider/help_provider.cpp`

HelpProvider is a nonvisual, window-bound metadata and request router. It tokenizes per-control help strings, keywords, navigator policy, and explicit visibility; publishes semantic descriptions; owns an F1 accelerator; and emits policy-free requests that consumers or capability-gated plugins may handle. It never opens a browser, help file, or network resource itself.

## Visual evidence

![HelpProvider](../captures/tool_tip_error_provider.png)

## Declared methods

### `HelpProvider` (public)

```cpp
explicit HelpProvider(Window& window)
```

Binds help metadata and the F1 accelerator to one live Window lifetime.

### `~HelpProvider` (public)

```cpp
~HelpProvider() override
```

Disconnects accelerator, target mappings, semantics, and lifetime state before destruction.

### `can_extend` (public)

```cpp
[[nodiscard]] bool can_extend(const std::shared_ptr<Control>& target) const
```

Reports whether a shared control is live and belongs to the bound Window.

### `set_help_string` (public)

```cpp
void set_help_string(const std::shared_ptr<Control>& target, std::string text)
```

Validates a target, commits explanatory text, updates its semantic description, and removes empty mappings.

### `help_string` (public)

```cpp
[[nodiscard]] std::string help_string(const Control& target) const
```

Returns retained explanatory text for the target.

### `set_help_keyword` (public)

```cpp
void set_help_keyword(const std::shared_ptr<Control>& target, std::string keyword)
```

Commits a policy-neutral lookup keyword and refreshes target semantics.

### `help_keyword` (public)

```cpp
[[nodiscard]] std::string help_keyword(const Control& target) const
```

Returns the target's retained lookup keyword.

### `set_help_navigator` (public)

```cpp
void set_help_navigator(const std::shared_ptr<Control>& target, HelpNavigator navigator)
```

Commits the target's requested navigation mode without performing navigation.

### `help_navigator` (public)

```cpp
[[nodiscard]] HelpNavigator help_navigator(const Control& target) const
```

Returns the target's requested navigation mode.

### `set_show_help` (public)

```cpp
void set_show_help(const std::shared_ptr<Control>& target, bool show)
```

Sets an explicit per-target eligibility override and refreshes semantic publication.

### `show_help` (public)

```cpp
[[nodiscard]] bool show_help(const Control& target) const
```

Reports effective help eligibility after default and explicit override policy.

### `reset_show_help` (public)

```cpp
void reset_show_help(const Control& target)
```

Removes a target's explicit eligibility override and returns to metadata-derived behavior.

### `clear` (public)

```cpp
void clear()
```

Removes every mapping and restores affected semantic descriptions.

### `help_namespace` (public)

```cpp
[[nodiscard]] const std::string& help_namespace() const noexcept
```

Returns the provider-wide policy-neutral help namespace string.

### `set_help_namespace` (public)

```cpp
void set_help_namespace(std::string value)
```

Commits namespace metadata for future requests without opening any resource.

### `tag` (public)

```cpp
[[nodiscard]] const std::any& tag() const noexcept
```

Returns caller-owned opaque provider metadata.

### `set_tag` (public)

```cpp
void set_tag(std::any tag)
```

Replaces caller-owned opaque provider metadata without changing help routing.

### `request_help` (public)

```cpp
bool request_help(const std::shared_ptr<Control>& target, Point position, bool keyboard_initiated = false)
```

Builds and publishes a request for an eligible live target, updates counters, and returns handled state.

### `help_requested` (public)

```cpp
[[nodiscard]] Event<HelpRequestEvent&>& help_requested() noexcept
```

Returns the mutable event through which consumers handle policy-free help requests.

### `snapshot` (public)

```cpp
[[nodiscard]] HelpProviderSnapshot snapshot() const noexcept
```

Returns exact mapping and request counters for diagnostics and tests.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Executes HelpProvider's verify dispose thread operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Executes HelpProvider's on dispose operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `bound_window` (private)

```cpp
[[nodiscard]] Window* bound_window() const noexcept
```

Reports the current bound window value without mutation.

### `require_access` (private)

```cpp
void require_access(std::string_view operation) const
```

Reports the current require access value without mutation.

### `find_entry` (private)

```cpp
[[nodiscard]] Entry* find_entry(const Control& target)
```

Executes HelpProvider's find entry operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `find_entry` (private)

```cpp
[[nodiscard]] const Entry* find_entry(const Control& target) const
```

Reports the current find entry value without mutation.

### `require_entry` (private)

```cpp
[[nodiscard]] Entry& require_entry(const std::shared_ptr<Control>& target)
```

Executes HelpProvider's require entry operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `entry_effective` (private)

```cpp
[[nodiscard]] bool entry_effective(const Entry& entry) const noexcept
```

Reports the current entry effective value without mutation.

### `publish_semantics` (private)

```cpp
void publish_semantics(Entry& entry)
```

Executes HelpProvider's publish semantics operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `erase_if_empty` (private)

```cpp
void erase_if_empty(std::uint64_t runtime_id)
```

Executes HelpProvider's erase if empty operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `request_focused_help` (private)

```cpp
bool request_focused_help()
```

Executes HelpProvider's request focused help operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
