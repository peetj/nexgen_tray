# PROJECT_MEMORY

## Purpose
- System tray hub. Owns hotkeys and controls utilities via IPC (`QLocalSocket`).

## Build & run
- Windows (Qt6 + MinGW + CMake):
  - Configure: `cmake -S . -B build`
  - Build: `cmake --build build -j`
  - Run: `build\\nexgen_tray.exe`

## Repo structure / key modules
- `src/NotifyListener.{h,cpp}`: minimal HTTP server (LAN) for Alfred integration

## Decisions

## Gotchas
### Notify listener settings (Windows)
- Uses `QSettings("Nexgen", "Utilities")`.
- Registry location:
  - `HKCU\\Software\\Nexgen\\Utilities\\NotifyListener` (key)
  - `ApiKey` (REG_SZ) — required
  - `Port` (REG_DWORD) — default 17321
- Note: Qt uses `/` as group separator, so setting a value named `NotifyListener\\ApiKey` will NOT be read by `settings.value("NotifyListener/ApiKey")`.

### Notify listener API
- Health check: `GET http://<host>:17321/health` → `healthy`
- Notify: `POST http://<host>:17321/notify`
  - Header: `X-API-Key: <secret>`
  - JSON: `{ "title": "...", "message": "...", "timeoutMs": 5000 }`

Example curl (from Alfred):
```bash
curl -i -X POST "http://192.168.137.1:17321/notify" \
  -H "Content-Type: application/json" \
  -H "X-API-Key: <ApiKey>" \
  -d '{"title":"Done","message":"Task finished","timeoutMs":5000}'
```

## Current bug / hypothesis

## Next steps

## Session notes
- 2026-05-28: Added minimal HTTP notify listener to `nexgen_tray` so Alfred can POST completion notifications to Hermes PC; endpoints `/health` and `/notify` with `X-API-Key` auth.
