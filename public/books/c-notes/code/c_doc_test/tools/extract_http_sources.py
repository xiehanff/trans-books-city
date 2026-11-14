#!/usr/bin/env python3
"""Extract the HTTP source files embedded in S09_简易HTTP协议.md."""

from __future__ import annotations

import argparse
import re
from pathlib import Path
from typing import Dict, List

FILE_MARKER = re.compile(r"^/\* file: ([A-Za-z0-9_.-]+) \*/$")
EXPECTED_FILES = {"http_protocol.h", "http_protocol.c", "http_server.c"}


def extract(source: Path) -> Dict[str, str]:
    lines = source.read_text(encoding="utf-8").splitlines()
    files: Dict[str, str] = {}
    in_fence = False
    language = ""
    code: List[str] = []

    for line in lines:
        if not in_fence:
            if line.startswith("```"):
                in_fence = True
                language = line[3:].strip().lower()
                code = []
            continue

        if line == "```":
            if language == "c" and code:
                marker = FILE_MARKER.fullmatch(code[0])
                if marker is not None:
                    name = marker.group(1)
                    if name in files:
                        raise ValueError(f"重复的 HTTP 源文件标记: {name}")
                    files[name] = "\n".join(code) + "\n"
            in_fence = False
            language = ""
            code = []
        else:
            code.append(line)

    if in_fence:
        raise ValueError("S09 Markdown 存在未闭合的代码围栏")
    if set(files) != EXPECTED_FILES:
        missing = sorted(EXPECTED_FILES - set(files))
        extra = sorted(set(files) - EXPECTED_FILES)
        raise ValueError(f"HTTP 源文件不完整，缺少={missing} 多余={extra}")
    return files


def main() -> int:
    default_source = Path(__file__).resolve().parents[1].parent / "S09_简易HTTP协议.md"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=default_source)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()

    files = extract(args.source.resolve())
    output_dir = args.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    for name, content in files.items():
        (output_dir / name).write_text(content, encoding="utf-8")
    print(f"已提取 {len(files)} 个 HTTP 源文件到 {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
