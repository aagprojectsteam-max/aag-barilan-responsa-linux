#!/usr/bin/env python3
import argparse
import hashlib
import shutil
from pathlib import Path

ORIGINAL_SHA256 = "7987ad1c1da58f8f4a949451688e987fac44a3c873fe83c1fce4ec5e6a439dcb"
PATCHED_SHA256 = "6fcc0381376e5a3a0e57a4b643df08d9a4241141bd8fd3307193d238fede46fe"
OFFSET = 0x240100
EXPECTED = bytes.fromhex("04 20")
PATCHED = bytes.fromhex("2c 20")

def sha256(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

parser = argparse.ArgumentParser(description="Version-locked patch for user-supplied RESPONSA.exe")
parser.add_argument("exe", type=Path)
parser.add_argument("--no-backup", action="store_true")
args = parser.parse_args()
p = args.exe.expanduser().resolve()
if not p.is_file():
    raise SystemExit(f"File not found: {p}")

before = sha256(p)
if before == PATCHED_SHA256:
    print("ALREADY_PATCHED=YES")
    raise SystemExit(0)
if before != ORIGINAL_SHA256:
    raise SystemExit(f"Refusing patch: SHA256 mismatch: {before}")

data = bytearray(p.read_bytes())
actual = bytes(data[OFFSET:OFFSET+2])
if actual != EXPECTED:
    raise SystemExit(f"Refusing patch: expected {EXPECTED.hex()} at 0x{OFFSET:x}, got {actual.hex()}")

if not args.no_backup:
    backup = p.with_name(p.name + ".AAG-ORIGINAL")
    if not backup.exists():
        shutil.copy2(p, backup)
        print(f"BACKUP={backup}")

data[OFFSET:OFFSET+2] = PATCHED
p.write_bytes(data)
after = sha256(p)
if after != PATCHED_SHA256:
    raise SystemExit(f"Patch written but final SHA256 unexpected: {after}")
print("PATCH_APPLIED=YES")
print(f"SHA256={after}")
