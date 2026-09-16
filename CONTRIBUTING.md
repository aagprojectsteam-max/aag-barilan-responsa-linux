# Contributing

Contributions are welcome for documentation, portability improvements, safer detection, and compatibility testing.

Please do not submit proprietary application binaries, installers, data/content files, or third-party runtime DLLs.

When changing the executable patch logic, preserve fail-closed hash and byte validation. When changing touch handling, avoid grabbing input devices or globally moving the pointer unless the behavior is explicitly opt-in and documented.
