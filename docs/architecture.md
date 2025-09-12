# Architecture Overview (Refactor Stage 1)

## Goals of This Refactor (Stage 1)
- Reduce orchestration duplication (Engine -> GameService -> SessionController).
- Make `SessionController` a thin UI/session adapter over `GameService`.
- Remove duplicated board state from `GameBoardRenderer` (render directly from `IBoardView`).
- Prepare ground for future separation of rule validation and AI statistics exposure.

## Layering
```
Presentation (SFML Scenes)
  └─ SessionController (UI/session adapter)
       └─ GameService (Application Service)
            ├─ Board (Domain Model)
            ├─ MinimaxSearchEngine (AI via ISearchEngine)
            └─ Repository (IBoardRepository -> MemoryBoardRepository)
```

Engine façade has been removed (Stage 1.1 follow-up). All previous usages migrated to `SessionController` (GUI) or lightweight adapters in tests.

## Changes Introduced
1. `SessionController` now owns a `std::unique_ptr<GameService>` instead of an `Engine` instance.
2. AI moves & human moves flow directly through `GameService` methods (`makeMove`, `getAIMove`).
3. `GameBoardRenderer` removed its internal 19x19 `CellState` cache; it renders on-demand from a bound `IBoardView` pointer supplied via `setBoardView`.
4. `GameScene` updated to bind renderer to snapshot view instead of copying board contents.
5. Introduced documentation file (this file) to describe transitional architecture.

## Benefits
- Single source of truth for board state (`Board` inside `GameService`).
- Eliminates risk of renderer desynchronization.
- Simplifies future rule engine extraction (validation no longer duplicated via Engine wrapper logic).

## Technical Debt Remaining / Next Steps
1. **Force Side / Starting Player**: `SessionController::reset` white-start hack not implemented properly. Add a method `forceSide(Player)` to `GameService` or have `Board` expose a controlled API.
2. **Expose AI Statistics**: Extend `ISearchEngine` / `GameService` to retain last search stats, return them in `getAIMove`.
3. **Rule Validation Extraction**: Introduce `IRuleValidator` so `Board::tryPlay` logic (pattern rules / captures) can be modular and testable.
4. **CLI/Test Adaptation Post-Engine**: CLI can now construct a `SessionController` directly (or a thin wrapper if isolation desired). Tests already migrated via `TestEngine` shim.
5. **Context / Scene Booleans**: Replace multiple boolean flags with a state machine or enum-driven navigator.
6. **Testing Expansion**: Add tests for `SessionController` scenarios (undo multi half-moves, AI integration) and rendering invariants (no crash with null board view, etc.).
7. **Statistics & Telemetry**: Provide deeper AI stats (nodes/s, PV) into HUD.

## Proposed Stage 2 Roadmap
| Stage | Action | Outcome |
|-------|--------|---------|
| 2.1 | Add `forceSide` to `GameService` | Clean player-start semantics |
| 2.2 | Add `lastSearchStats()` accessor | HUD display of AI performance |
| 2.3 | Introduce `IRuleValidator` | Pluggable rule variants |
| 2.4 | (DONE) Remove/Adapt `Engine` usages | Simplified public API |
| 2.5 | Expand unit tests | Higher confidence & regression safety |
| 2.6 | Refactor `Context` | Clear navigation state |

## Migration Notes
Existing code using `SessionController::engine()` must be updated; that accessor was removed. Instead, call `snapshot()` for read access or (if absolutely needed) extend `SessionController` to expose selective `GameService` methods.

---
Generated date: 2025-09-12
