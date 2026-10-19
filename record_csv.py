#!/usr/bin/env python3
"""Record Uno serial CSV on macOS using only the Python standard library."""
import argparse
import csv
from datetime import datetime, timezone
import os
from pathlib import Path
import select
import termios
import tty

FIELDS = "uptime_ms,distance_cm,echo_ok,ldr_raw,near,fault,override,lamp".split(",")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", help="For example /dev/cu.usbmodem1101")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    output = args.output or Path("greentech-" + datetime.now().strftime("%Y%m%d-%H%M%S") + ".csv")
    fd = os.open(args.port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    previous = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        settings = termios.tcgetattr(fd)
        settings[4] = settings[5] = termios.B9600
        settings[2] |= termios.CLOCAL | termios.CREAD
        termios.tcsetattr(fd, termios.TCSANOW, settings)
        # Exclusive creation prevents accidental overwriting of a prior run.
        with output.open("x", newline="", encoding="utf-8") as target:
            writer = csv.writer(target)
            writer.writerow(["received_utc", "session"] + FIELDS)
            target.flush()
            print(f"Recording {args.port} to {output.resolve()}", flush=True)
            print("Close Serial Monitor. Press Uno RESET once. Ctrl+C stops recording.", flush=True)
            pending = b""
            session = 1
            last_uptime = None
            while True:
                if not select.select([fd], [], [], 1)[0]:
                    continue
                chunk = os.read(fd, 4096)
                if not chunk:
                    raise OSError("Serial device disconnected")
                pending += chunk
                if len(pending) > 8192:
                    pending = b""  # Discard noise without unbounded buffering.
                    continue
                while b"\n" in pending:
                    raw, pending = pending.split(b"\n", 1)
                    row = raw.decode("ascii", errors="replace").strip().split(",")
                    if row == FIELDS:
                        if last_uptime is not None:
                            session += 1
                        last_uptime = None
                        continue
                    if len(row) != len(FIELDS):
                        continue
                    try:
                        uptime = int(row[0])
                        ldr = int(row[3])
                        flags = [int(row[i]) for i in (2, 4, 5, 6, 7)]
                        if uptime < 0 or not 0 <= ldr <= 1023 or any(v not in (0, 1) for v in flags):
                            continue
                        if row[2] == "1":
                            if not 2 <= float(row[1]) <= 400:
                                continue
                        elif row[1] != "":
                            continue
                    except ValueError:
                        continue
                    if last_uptime is not None and uptime < last_uptime:
                        session += 1  # Reset or uptime rollover; never join silently.
                    last_uptime = uptime
                    writer.writerow([datetime.now(timezone.utc).isoformat(), session] + row)
                    target.flush()
                    print(",".join(row), flush=True)
    except KeyboardInterrupt:
        print("\nRecording stopped.")
    finally:
        try:
            termios.tcsetattr(fd, termios.TCSANOW, previous)
        finally:
            os.close(fd)


if __name__ == "__main__":
    main()
