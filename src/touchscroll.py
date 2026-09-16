#!/usr/bin/env python3
import argparse
import os
import re
import signal
import subprocess
import sys

parser = argparse.ArgumentParser(description="Touch drag to Responsa WM_VSCROLL bridge")
parser.add_argument("--bottle", default=os.environ.get("AAG_RESPONSA_BOTTLE", "/mnt/data/Bottles/BarIlan__197"))
parser.add_argument("--runner", default=os.environ.get("AAG_RESPONSA_RUNNER", os.path.expanduser("~/.var/app/com.usebottles.bottles/data/bottles/runners/soda-11.0-10")))
parser.add_argument("--pixels-per-step", type=float, default=float(os.environ.get("AAG_TOUCH_PIXELS_PER_STEP", "65")))
parser.add_argument("--deadzone", type=float, default=float(os.environ.get("AAG_TOUCH_DEADZONE", "6")))
args = parser.parse_args()

BOTTLE = args.bottle
RUNNER = os.path.expanduser(args.runner)
WORK = os.path.dirname(os.path.abspath(__file__))
BRIDGE = os.path.join(WORK, "scrollbridge.exe")
PIXELS_PER_STEP = args.pixels_per_step
DEADZONE = args.deadzone

env = os.environ.copy()
env["DISPLAY"] = env.get("DISPLAY") or ":0"
env["WINEPREFIX"] = BOTTLE
env["WINEDEBUG"] = "-all"

bridge = subprocess.Popen(
    [os.path.join(RUNNER, "bin", "wine"), BRIDGE],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.DEVNULL,
    text=True,
    bufsize=1,
    env=env,
)

if bridge.stdout:
    print(bridge.stdout.readline().strip())

xi = subprocess.Popen(
    ["xinput", "test-xi2", "--root"],
    stdout=subprocess.PIPE,
    stderr=subprocess.DEVNULL,
    text=True,
    bufsize=1,
    env={**os.environ, "DISPLAY": env["DISPLAY"]},
)

event_re = re.compile(r"EVENT type\s+\d+\s+\(([^)]+)\)")
root_re = re.compile(r"root:\s+[-0-9.]+/([-0-9.]+)")
active = False
event = None
last_y = None
accum = 0.0

def send(cmd):
    if bridge.poll() is None and bridge.stdin:
        bridge.stdin.write(cmd + "\n")
        bridge.stdin.flush()

def cleanup(*_):
    try:
        xi.terminate()
    except Exception:
        pass
    try:
        send("Q")
        bridge.terminate()
    except Exception:
        pass
    sys.exit(0)

signal.signal(signal.SIGINT, cleanup)
signal.signal(signal.SIGTERM, cleanup)

if not xi.stdout:
    cleanup()

for raw in xi.stdout:
    line = raw.strip()
    m = event_re.search(line)
    if m:
        event = m.group(1)
        if event == "TouchBegin":
            active = True
            last_y = None
            accum = 0.0
        elif event == "TouchEnd":
            active = False
            last_y = None
            accum = 0.0
        continue

    if not active or event not in ("TouchBegin", "TouchUpdate"):
        continue

    m = root_re.search(line)
    if not m:
        continue

    y = float(m.group(1))
    if last_y is None:
        last_y = y
        continue

    dy = y - last_y
    last_y = y
    if abs(dy) < DEADZONE:
        continue

    accum += dy
    if accum <= -PIXELS_PER_STEP:
        send("D")
        accum += PIXELS_PER_STEP
    elif accum >= PIXELS_PER_STEP:
        send("U")
        accum -= PIXELS_PER_STEP

cleanup()
