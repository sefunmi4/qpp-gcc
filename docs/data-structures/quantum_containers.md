# Learning Quantum Collections: `qint`, `qvector`, and `entangled_set`

The quantum data-structure helpers in **QPP** layer domain-specific semantics on
top of familiar standard-library containers. This guide explains how the core
primitives—[`qint`](#qint-quantum-orientation-state),
[`qvector`](#qvector-quantum-friendly-dynamic-arrays), and
[`entangled_set`](#entangled_set-context-aware-hash-sets)—fit together inside
the *Data Structures and Algorithms* examples so you can extend or adapt them in
your own learning projects.

## `qint`: quantum orientation state

`qint` represents a four-axis quantum orientation. Each axis stores a discrete
`amplitude` that records polarity as either `negative` (−1) or `positive` (+1).
The fields map to the following interpretations:

| Axis | Field | Negative (`-1`) meaning | Positive (`+1`) meaning |
| ---- | ----- | ----------------------- | ----------------------- |
| X    | `x`   | logical `0` / northbound step | logical `1` / southbound step |
| Y    | `y`   | symbol `-` / eastward step | symbol `+` /  westward step |
| Z    | `z`   | `-45°` rotation (back flip) | `+45°` rotation (front flip) |
| T    | `t`   | `-45i` imaginary rotation (backward time) | `+45i` imaginary rotation (forward time) |

Constructors accept either explicit `amplitude` values or ordinary integers,
which are normalised so that any non-positive number becomes `negative`.
Convenience accessors—`x_step()`, `y_step()`, `z_step()`, and `t_step()`—expose
axis-specific encodings when you need to visualise, log, or convert the state.

`qint` also defines equality and provides a `std::hash` specialisation. This
makes it a drop-in key for hashed containers such as `std::entangled_set` and
means algorithms can rely on value semantics when deduplicating or comparing
states.

## `qvector`: quantum-friendly dynamic arrays

`std::qvector<T>` is a straightforward alias for `std::vector<T>` that resides in
the standard namespace. The alias lets educational material refer to "quantum
vectors" without sacrificing the familiarity or performance characteristics of
the standard dynamic array. Any API that works with `std::vector`—iterators,
capacity management, algorithms—works identically with `std::qvector`.

When storing `qint` values, `std::qvector<qint>` becomes the natural building
block for quantum sequences. You can reserve capacity, push states, and iterate
with range-based `for` loops exactly as you would with classical data.

## `entangled_set`: context-aware hash sets

`std::entangled_set<Key>` wraps an `std::unordered_set` but injects a
cryptographically inspired *entanglement context* into its hashing pipeline. By
default, each set instance seeds a shared `context` with two secret keys, a
"domain" tag, and an epoch counter. The generated salt feeds into the
`hasher` functor so that identical logical sets can produce different hash
sequences across contexts—helping demonstrate how quantum systems decorrelate
shared states.

Key features to be aware of:

- **Context control:** You can pass a custom `context` (via `make_context` or
  `make_default_context`) to share entropy between sets or to rotate epochs when
  demonstrating time-evolving behaviour.
- **Transparent equality:** `transparent_equal` allows heterogeneous lookups so
  string-like keys can be compared without constructing temporary `std::string`
  objects. Combined with the hasher's `is_transparent` marker, this mirrors the
  ergonomics of `std::unordered_set` while reinforcing how entanglement can span
  related representations.
- **Broad key support:** The hashing utility handles integrals, floating-point
  numbers, tuples, string-like types, and pointers by delegating to specialised
  helpers. That makes `std::entangled_set` a flexible drop-in replacement for
  quantum-flavoured collections built on top of the STL.

## Putting the pieces together

The *Arrays and Hashing* example uses these helpers to reframe classic interview
questions in a quantum context. The duplicate-check routine below highlights the
interplay between all three types:

```cpp
std::qvector<qint> states = {/* ... prepare quantum inputs ... */};
std::entangled_set<qint> visited;
visited.reserve(states.size());
for (const auto& state : states) {
    if (visited.find(state) != visited.end())
        return true; // entangled collision detected
    visited.insert(state);
}
return false;
```

- `qvector` provides contiguous storage for a sequence of `qint` states.
- Each `qint` carries multi-axis orientation data while remaining hashable.
- `entangled_set` tracks which states have been observed using its salted hash
  function.

Try modifying the `context` epoch between runs or mixing `qint` constructions
(from both integer and `amplitude` inputs) to experiment with how the data
structures react. Because these helpers are thin abstractions over standard
containers, they are safe playgrounds for bridging classical algorithm drills
and quantum intuition.
