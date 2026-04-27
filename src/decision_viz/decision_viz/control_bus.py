from __future__ import annotations

import json
import time
from pathlib import Path
from typing import Any


def append_command(path: Path, command: str, payload: dict[str, Any] | None = None) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    body: dict[str, Any] = {
        "ts": time.time(),
        "command": command,
    }
    if payload:
        body.update(payload)
    with path.open("a", encoding="utf-8") as stream:
        stream.write(json.dumps(body, ensure_ascii=True, separators=(",", ":")))
        stream.write("\n")
        stream.flush()


def read_commands(path: Path, offset: int) -> tuple[list[dict[str, Any]], int]:
    if not path.exists():
        return ([], offset)
    try:
        with path.open("rb") as stream:
            stream.seek(max(0, int(offset)))
            chunk = stream.read()
    except OSError:
        return ([], offset)
    if not chunk:
        return ([], offset)

    commands: list[dict[str, Any]] = []
    consumed = 0
    for line in chunk.splitlines(keepends=True):
        if not line.endswith(b"\n"):
            break
        consumed += len(line)
        raw = line.strip()
        if not raw:
            continue
        try:
            payload = json.loads(raw.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            continue
        if isinstance(payload, dict):
            commands.append(payload)
    return (commands, offset + consumed)
