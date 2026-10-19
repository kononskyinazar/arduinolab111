"""Integration check using a pseudo-terminal, not an attached Arduino."""
import csv
import os
from pathlib import Path
import pty
import signal
import subprocess
import sys
import tempfile
import time

root = Path(__file__).resolve().parent
master, slave = pty.openpty()
try:
    with tempfile.TemporaryDirectory() as directory:
        output = Path(directory) / "readings.csv"
        process = subprocess.Popen(
            [sys.executable, str(root / "record_csv.py"), os.ttyname(slave), "--output", str(output)],
            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True,
        )
        try:
            deadline = time.monotonic() + 5
            while not output.exists() and process.poll() is None and time.monotonic() < deadline:
                time.sleep(0.02)
            assert output.exists(), "Recorder did not start"
            header = b"uptime_ms,distance_cm,echo_ok,ldr_raw,near,fault,override,lamp\n"
            os.write(master, header + b"1000,30.0,1,500,1,0,0,1\ninvalid\n2000,,0,500,0,1,0,1\n")
            os.write(master, header + b"100,100.0,1,500,0,0,0,0\n")
            rows = []
            while time.monotonic() < deadline:
                with output.open(newline="") as source:
                    rows = list(csv.DictReader(source))
                if len(rows) == 3:
                    break
                time.sleep(0.02)
            assert len(rows) == 3, rows
            assert [row["session"] for row in rows] == ["1", "1", "2"]
            assert rows[1]["distance_cm"] == "" and rows[1]["fault"] == "1"
            assert all(row["received_utc"].endswith("+00:00") for row in rows)
        finally:
            if process.poll() is None:
                process.send_signal(signal.SIGINT)
            _, errors = process.communicate(timeout=5)
            assert process.returncode == 0, errors
finally:
    os.close(master)
    os.close(slave)
print("PASS: serial CSV capture, invalid row filtering, reset sessions, UTC, clean stop")
