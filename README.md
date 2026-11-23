# MK1 AI Core — first implementation milestone

This is a **portable, read-only-from-the-game-side decision core prototype**. It is not yet an MK1Hook plugin and does not issue game inputs. It establishes testable data structures and learning/scoring logic before connecting to undocumented game internals.

## Included now
- Opponent action frequency tracking and punished-action tracking.
- Per-character combo route catalog.
- Combo attempt/success/failure statistics.
- Combo selection based on situation, distance, meter, estimated damage, and learned reliability.
- A bounded round-performance signal for a future adaptive-difficulty controller.
- 300+ damage routes receive a small preference bonus, not an unconditional override.

## Build check

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -c MK1AI_Core.cpp -o MK1AI_Core.o
```

## Current limitations (intentional)
- The game-side telemetry adapter is not implemented yet.
- `distance` units must be calibrated from MK1 runtime coordinates.
- `inputSequence` is metadata only; actual move execution must be mapped to verified MK1 move/script functions.
- Combo route data must be populated from verified character-specific routes; no invented route inputs are included.
- No game memory is written and no MKO file is modified.

## Next integration milestone
1. Verify what combat state can be read safely from MK1Hook's `MKCharacter`, `PlayerInfo`, `CharacterDefinitionV2`, and runtime MKScript interfaces.
2. Add narrow, validated exports to MK1Hook for snapshots and AI decisions.
3. Build a `.ehp` plugin that displays telemetry and decision logs before enabling any automated action.
4. Add one character and one verified route for an end-to-end test.
