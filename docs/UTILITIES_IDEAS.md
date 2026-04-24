# Nexgen Utilities — Ideas & Platform Notes

This is the **shared scratchpad** for Nexgen utilities.

It lives in `nexgen_tray` because the tray/hub is the platform centre:
- owns global hotkeys
- owns theming/tokens
- controls utilities via IPC

If an idea turns into a project, create a dedicated repo and keep *implementation details* in that project’s `docs/PROJECT_MEMORY.md`.

---

## Platform principles (north star)
- **Minimal**: nothing you don’t need.
- **Quick**: hotkey → action → you’re back.
- **Unobtrusive**: small footprint; reliable focus restore.
- **Transparent-capable**: translucency/blur where it helps you stay in-context.
- **Beautiful**: calm typography/spacing; consistent theming.

---

## Ideas backlog

### Folder colouring in Windows Explorer (via icon library)
**Problem:** folders all look the same; hard to visually group projects/contexts.

**v1 approach (recommended):** per-folder **custom icon** (desktop.ini + folder attributes).
- This is “native enough” and resilient compared to Explorer injection.
- The real value is a **great icon library** (palette + shapes + optional labels), because finding good icons is the usual blocker.

**Nice UX:**
- Right-click a folder → `Nexgen Color` → pick a colour (and optionally an icon style).
- Also offer: `Clear colour` (restore default icon).
- Bonus: `Nexgen Icons…` to browse/search the library.

**Design bar:** this should feel like a first-party feature: fast, clean, no fuss.

**Open questions:**
- Context menu integration vs hotkey-driven picker UI (or both).
- Where to store the icon library (per-user app data vs shipped assets).
- Whether to support labels/badges on icons (e.g. “FIN”, “URGENT”, “2026”).
