#!/usr/bin/env python3
"""Verify the inventory of fenced code blocks in the C notes.

The parser deliberately has no Markdown dependency.  It implements the part of
CommonMark needed here: a fenced block starts with up to three spaces followed
by at least three backticks or tildes, and ends with a fence made from the same
character and at least the same length.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Sequence, Tuple

SCHEMA_VERSION = 1
ALLOWED_C_MODES = {
    "run",
    "compile_pass",
    "compile_fail",
    "platform",
    "ub_demo",
    "fragment",
    "document_only",
}

_OPEN_FENCE = re.compile(r"^( {0,3})(`{3,}|~{3,})(.*)$")
_CLOSE_FENCE = re.compile(r"^ {0,3}(`{3,}|~{3,})[ \t]*$")

Block = Dict[str, Any]


def _normalise_code(lines: Sequence[str]) -> str:
    """Return the hash representation of a block.

    Markdown is read as logical UTF-8 lines.  Code line endings are normalised
    to LF and the newline immediately before the closing fence is not part of
    the representation.  This keeps hashes independent of checkout line
    endings while preserving every code line and all indentation.
    """

    return "\n".join(lines)


def _stable_id(relative_file: str, fence_number: int, language: str) -> str:
    identity = f"{relative_file}\0{fence_number}\0{language}".encode("utf-8")
    digest = hashlib.sha256(identity).hexdigest()[:16]
    prefix = "c" if language == "c" else "block"
    return f"{prefix}-{digest}"


def _language(info: str) -> str:
    token = info.strip().split(None, 1)[0] if info.strip() else "text"
    # The notes use plain language names.  Keep the parser useful for common
    # aliases without changing the original info string in the manifest.
    aliases = {"C": "c", "shell": "bash", "sh": "bash", "console": "text"}
    return aliases.get(token, token.lower())


def parse_markdown_file(path: Path, relative_file: str) -> List[Block]:
    lines = path.read_text(encoding="utf-8").splitlines()
    blocks: List[Block] = []
    active: Optional[Dict[str, Any]] = None
    fence_number = 0

    for line_number, line in enumerate(lines, start=1):
        if active is not None:
            closing = _CLOSE_FENCE.match(line)
            if (
                closing is not None
                and closing.group(1)[0] == active["marker_char"]
                and len(closing.group(1)) >= active["marker_length"]
            ):
                code_lines = active["code_lines"]
                code = _normalise_code(code_lines)
                info = active["info"]
                language = _language(info)
                block: Block = {
                    "id": _stable_id(relative_file, active["fence_number"], language),
                    "file": relative_file,
                    "language": language,
                    "info_string": info,
                    "fence": active["fence"],
                    "fence_number": active["fence_number"],
                    "fence_start_line": active["fence_start_line"],
                    "fence_end_line": line_number,
                    "start_line": active["fence_start_line"] + 1,
                    "end_line": line_number - 1,
                    "sha256": hashlib.sha256(code.encode("utf-8")).hexdigest(),
                }
                blocks.append(block)
                active = None
            else:
                active["code_lines"].append(line)
            continue

        opening = _OPEN_FENCE.match(line)
        if opening is None:
            continue
        fence = opening.group(2)
        info = opening.group(3).strip()
        fence_number += 1
        active = {
            "marker_char": fence[0],
            "marker_length": len(fence),
            "fence": fence,
            "info": info,
            "fence_number": fence_number,
            "fence_start_line": line_number,
            "code_lines": [],
        }

    if active is not None:
        raise ValueError(
            f"unterminated fenced block: {relative_file}:"
            f"{active['fence_start_line']}"
        )
    return blocks


def discover_blocks(source_dir: Path) -> List[Block]:
    blocks: List[Block] = []
    for path in sorted(source_dir.glob("*.md"), key=lambda p: p.name):
        blocks.extend(parse_markdown_file(path, path.name))
    return blocks


def _nearby_context(lines: Sequence[str], block: Block, radius: int = 3) -> str:
    start = max(0, int(block["fence_start_line"]) - 1 - radius)
    end = min(len(lines), int(block["fence_end_line"]) + radius)
    return "\n".join(lines[start:end]).lower()


def classify_c_block(block: Block, source_lines: Sequence[str]) -> str:
    """Conservative first-pass classification for a C block.

    This is an intent label, not a compiler result.  Dangerous examples are
    deliberately classified before the runnable/compilable heuristics.
    """

    code = _normalise_code(block["_code_lines"]).lower()
    context = _nearby_context(source_lines, block)
    combined = f"{context}\n{code}"

    # These examples are meant to show a fault or undefined behaviour.  Never
    # let the presence of main() turn them into an ordinary run candidate.
    ub_markers = (
        "未定义行为",
        "undefined behavior",
        "越界",
        "空指针",
        "野指针",
        "悬空指针",
        "悬空",
        "use after free",
        "use-after-free",
        "double-free",
        "重复释放",
        "内存泄漏",
        "缓冲区溢出",
        "栈溢出",
        "无限递归",
        "无限循环",
        "segmentation fault",
        "段错误",
        "危险",
        "崩溃",
    )
    if any(marker in combined for marker in ub_markers):
        return "ub_demo"

    # A few unmistakable invalid snippets are compile-failure demonstrations.
    # This is intentionally narrower than searching for the word “错误”, which
    # occurs next to many correct replacement examples.
    platform_markers = (
        "static_assert(sizeof(void*)",
        "static_assert(sizeof(void *)",
        "sizeof(void*)",
        "sizeof(void *)",
        "__asm__",
        "__attribute__",
        "__builtin_",
        "__thread",
        "immintrin.h",
        "pthread_",
        "pthread.h",
        "arpa/inet.h",
        "unistd.h",
        "sys/",
        "windows.h",
        "winsock",
        "_win32",
        "__linux__",
        "__apple__",
        "x86",
        "x86-64",
        "amd64",
        "r8",
        "rsp",
        "rdtsc",
        "simd",
        "##__va_args__",
    )
    if any(marker in combined for marker in platform_markers):
        return "platform"

    # Keep this list specific.  Nearby prose often says “编译错误” while the
    # code block itself is a valid replacement example.
    compile_fail_markers = (
        "int 2nd_place",
        "int my-var",
        "int int;",
        "int if;",
        "void wrong(...)",
        "取模不能用于浮点数",
        "5 = x",
        "#define foo(x) do_something(x);",
        "log(\"hello\");",
        "cannot compile",
    )
    if any(marker in combined for marker in compile_fail_markers):
        return "compile_fail"

    # A main function is the clearest available signal for a runnable example.
    # It still does not assert that headers, libraries, or behaviour were
    # tested; this distinction is documented in tools/README.md.
    if re.search(r"\bmain\s*\(", code):
        return "run"

    # Complete-looking function/type definitions are useful compile units, but
    # generally have no entry point and may rely on neighbouring snippets.
    if re.search(r"\b(struct|union|enum|typedef)\b", code) or re.search(
        r"\b(?:void|char|short|int|long|float|double|size_t)\s+\w+\s*\([^;{}]*\)\s*\{",
        code,
    ):
        return "compile_pass"

    # Short declarations, expressions, and control-flow fragments are intended
    # to be combined with surrounding code.  Prose-like or signature-only
    # blocks are kept document_only so no checker will try to execute them.
    if "..." in code or "省略" in combined or "伪代码" in combined:
        return "document_only"
    if re.search(r"[;{}]|\b(if|for|while|switch|return)\b", code):
        return "fragment"
    return "document_only"


def make_manifest(source_dir: Path) -> Dict[str, Any]:
    blocks = discover_blocks(source_dir)
    # Keep private parser data out of the public records while making source
    # context available to the heuristic classifier.
    by_file: Dict[str, List[str]] = {
        path.name: path.read_text(encoding="utf-8").splitlines()
        for path in sorted(source_dir.glob("*.md"), key=lambda p: p.name)
    }
    public_blocks: List[Block] = []
    for block in blocks:
        path_lines = by_file[block["file"]]
        code_lines = path_lines[block["start_line"] - 1 : block["end_line"]]
        block["_code_lines"] = code_lines
        public = dict(block)
        public.pop("_code_lines", None)
        public["category"] = "c" if block["language"] == "c" else block["language"]
        if block["language"] == "c":
            public["mode"] = classify_c_block(block, path_lines)
        else:
            public["mode"] = "not_applicable"
        public_blocks.append(public)

    language_counts = collections.Counter(block["language"] for block in public_blocks)
    mode_counts = collections.Counter(
        block["mode"] for block in public_blocks if block["language"] == "c"
    )
    return {
        "schema_version": SCHEMA_VERSION,
        "source": "../*.md (the parent C notes directory)",
        "hash_normalization": "UTF-8 logical lines joined with LF; no terminal newline",
        "classification": "heuristic-v1; labels are intent, not compile/test results",
        "counts": {
            "files": len(by_file),
            "blocks": len(public_blocks),
            "c_blocks": language_counts.get("c", 0),
            "languages": dict(sorted(language_counts.items())),
            "c_modes": dict(sorted(mode_counts.items())),
        },
        "blocks": public_blocks,
    }


def _identity(block: Block) -> Tuple[Any, ...]:
    return tuple(block.get(key) for key in (
        "id",
        "file",
        "language",
        "info_string",
        "fence",
        "fence_number",
        "fence_start_line",
        "fence_end_line",
        "start_line",
        "end_line",
        "sha256",
        "category",
        "mode",
    ))


def _validate_manifest_shape(manifest: Dict[str, Any]) -> List[str]:
    errors: List[str] = []
    if manifest.get("schema_version") != SCHEMA_VERSION:
        errors.append(
            f"unsupported schema_version: {manifest.get('schema_version')!r}"
        )
    blocks = manifest.get("blocks")
    if not isinstance(blocks, list):
        return errors + ["manifest.blocks is not a list"]

    ids = [block.get("id") for block in blocks if isinstance(block, dict)]
    locations = [
        (block.get("file"), block.get("fence_number"))
        for block in blocks
        if isinstance(block, dict)
    ]
    for value, label in ((ids, "id"), (locations, "file/fence_number")):
        duplicates = [key for key, count in collections.Counter(value).items() if count > 1]
        if duplicates:
            errors.append(f"duplicate manifest {label}: {duplicates[:3]}")

    required = {
        "id", "file", "language", "info_string", "fence", "fence_number",
        "fence_start_line", "fence_end_line", "start_line", "end_line",
        "sha256", "category", "mode",
    }
    for index, block in enumerate(blocks):
        if not isinstance(block, dict):
            errors.append(f"block {index} is not an object")
            continue
        missing = sorted(required - set(block))
        if missing:
            errors.append(f"block {index} missing fields: {', '.join(missing)}")
        if block.get("language") == "c" and block.get("mode") not in ALLOWED_C_MODES:
            errors.append(
                f"C block {block.get('id', index)} has invalid mode "
                f"{block.get('mode')!r}"
            )
    return errors


def _public_current_blocks(
    current: List[Block], source_dir: Optional[Path]
) -> List[Block]:
    public_blocks: List[Block] = []
    source_lines: Dict[str, List[str]] = {}
    for block in current:
        public = dict(block)
        language = block["language"]
        public["category"] = "c" if language == "c" else language
        if language != "c":
            public["mode"] = "not_applicable"
        elif source_dir is None:
            public["mode"] = "not_checked"
        else:
            if block["file"] not in source_lines:
                source_lines[block["file"]] = (
                    (source_dir / block["file"])
                    .read_text(encoding="utf-8")
                    .splitlines()
                )
            private = dict(block)
            lines = source_lines[block["file"]]
            private["_code_lines"] = lines[
                block["start_line"] - 1 : block["end_line"]
            ]
            public["mode"] = classify_c_block(private, lines)
        public_blocks.append(public)
    return public_blocks


def verify(
    manifest: Dict[str, Any],
    current: List[Block],
    source_file_count: Optional[int] = None,
    source_dir: Optional[Path] = None,
) -> List[str]:
    errors = _validate_manifest_shape(manifest)
    recorded = manifest.get("blocks")
    if not isinstance(recorded, list):
        return errors

    current_public = _public_current_blocks(current, source_dir)

    recorded_by_id = {
        block.get("id"): block for block in recorded if isinstance(block, dict)
    }
    current_ids = [block["id"] for block in current_public]
    current_locations = [
        (block["file"], block["fence_number"]) for block in current_public
    ]
    for values, label in ((current_ids, "id"), (current_locations, "file/fence_number")):
        duplicates = [key for key, count in collections.Counter(values).items() if count > 1]
        if duplicates:
            errors.append(f"duplicate current {label}: {duplicates[:3]}")
    current_by_id = {block["id"]: block for block in current_public}
    missing = sorted(set(current_by_id) - set(recorded_by_id))
    extra = sorted(set(recorded_by_id) - set(current_by_id))
    if missing:
        errors.append(f"manifest omissions ({len(missing)}): {', '.join(missing[:5])}")
    if extra:
        errors.append(f"manifest extras ({len(extra)}): {', '.join(extra[:5])}")

    for block_id in sorted(set(current_by_id) & set(recorded_by_id)):
        expected = current_by_id[block_id]
        actual = recorded_by_id[block_id]
        if _identity(expected) != _identity(actual):
            errors.append(
                f"metadata/hash drift for {block_id}: "
                f"manifest={_identity(actual)!r}, current={_identity(expected)!r}"
            )

    expected_counts = _current_counts_with_manifest(
        current, manifest, source_file_count=source_file_count
    )
    actual_counts = manifest.get("counts")
    if not isinstance(actual_counts, dict):
        errors.append("manifest.counts is not an object")
    else:
        for key in ("files", "blocks", "c_blocks", "languages", "c_modes"):
            if actual_counts.get(key) != expected_counts.get(key):
                errors.append(
                    f"count drift for {key}: manifest={actual_counts.get(key)!r}, "
                    f"current={expected_counts.get(key)!r}"
                )
    return errors


def make_counts(blocks: Iterable[Block]) -> Dict[str, Any]:
    blocks = list(blocks)
    languages = collections.Counter(block["language"] for block in blocks)
    # Current blocks do not carry modes; this is filled from the manifest in
    # summary/check where needed.
    return {
        "files": len({block["file"] for block in blocks}),
        "blocks": len(blocks),
        "c_blocks": languages.get("c", 0),
        "languages": dict(sorted(languages.items())),
        "c_modes": {},
    }


def _current_counts_with_manifest(
    current: List[Block],
    manifest: Dict[str, Any],
    source_file_count: Optional[int] = None,
) -> Dict[str, Any]:
    counts = make_counts(current)
    if source_file_count is not None:
        counts["files"] = source_file_count
    by_id = {
        block.get("id"): block
        for block in manifest.get("blocks", [])
        if isinstance(block, dict)
    }
    modes = collections.Counter(
        by_id[block["id"]].get("mode")
        for block in current
        if block["language"] == "c" and block["id"] in by_id
    )
    counts["c_modes"] = dict(sorted(modes.items()))
    return counts


def _default_paths() -> Tuple[Path, Path]:
    tools_dir = Path(__file__).resolve().parent
    c_doc_test_dir = tools_dir.parent
    source_dir = c_doc_test_dir.parent
    return source_dir, c_doc_test_dir / "manifest.json"


def _compile_fail_examples(
    manifest: Dict[str, Any], current: List[Block], source_dir: Path,
    compiler_name: Optional[str],
) -> int:
    errors = verify(
        manifest,
        current,
        source_file_count=len(list(source_dir.glob("*.md"))),
        source_dir=source_dir,
    )
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    compiler = compiler_name or shutil.which("cc")
    if not compiler:
        print("SKIP: 未找到 C 编译器，无法验证 compile_fail 示例")
        return 0

    failures = 0
    checked = 0
    for block in manifest["blocks"]:
        if block["language"] != "c" or block["mode"] != "compile_fail":
            continue
        source_lines = (source_dir / block["file"]).read_text(
            encoding="utf-8"
        ).splitlines()
        code = "\n".join(
            source_lines[block["start_line"] - 1 : block["end_line"]]
        ) + "\n"
        with tempfile.TemporaryDirectory(prefix="c-doc-negative-") as raw:
            source = Path(raw) / "example.c"
            source.write_text(code, encoding="utf-8")
            result = subprocess.run(
                [
                    compiler,
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Wpedantic",
                    "-Werror",
                    "-fsyntax-only",
                    str(source),
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
        checked += 1
        if result.returncode == 0:
            failures += 1
            print(
                f"FAIL: compile_fail 示例意外通过 "
                f"{block['file']}:{block['start_line']}",
                file=sys.stderr,
            )

    if failures:
        print(f"compile_fail 检查失败：{failures}/{checked}", file=sys.stderr)
        return 1
    print(f"OK: {checked} compile_fail 示例均按预期失败")
    return 0


def _print_summary(
    manifest: Dict[str, Any], current: List[Block], source_file_count: int
) -> None:
    counts = _current_counts_with_manifest(
        current, manifest, source_file_count=source_file_count
    )
    print(f"files: {counts['files']}")
    print(f"fenced blocks: {counts['blocks']}")
    print(f"C blocks: {counts['c_blocks']}")
    print("languages:")
    for language, count in counts["languages"].items():
        print(f"  {language}: {count}")
    print("C modes (manifest labels):")
    for mode, count in counts["c_modes"].items():
        print(f"  {mode}: {count}")


def main(argv: Optional[Sequence[str]] = None) -> int:
    default_source, default_manifest = _default_paths()
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--check", action="store_true", help="verify manifest exactly")
    group.add_argument(
        "--compile-fail",
        action="store_true",
        help="compile C blocks classified as compile_fail and expect failure",
    )
    group.add_argument("--summary", action="store_true", help="print inventory counts")
    group.add_argument(
        "--update",
        action="store_true",
        help="regenerate manifest.json using the conservative classifier",
    )
    parser.add_argument("--source-dir", type=Path, default=default_source)
    parser.add_argument("--manifest", type=Path, default=default_manifest)
    parser.add_argument(
        "--compiler", help="C compiler for --compile-fail (default: cc)"
    )
    args = parser.parse_args(argv)

    source_dir = args.source_dir.resolve()
    manifest_path = args.manifest.resolve()
    try:
        current = discover_blocks(source_dir)
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"error: cannot parse Markdown: {exc}", file=sys.stderr)
        return 2

    if args.update:
        manifest = make_manifest(source_dir)
        manifest_path.write_text(
            json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        print(f"wrote {manifest_path} ({len(manifest['blocks'])} blocks)")
        return 0

    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        print(f"error: cannot read manifest {manifest_path}: {exc}", file=sys.stderr)
        return 2
    if not isinstance(manifest, dict):
        print("error: manifest root is not an object", file=sys.stderr)
        return 2

    if args.summary:
        _print_summary(
            manifest, current, source_file_count=len(list(source_dir.glob("*.md")))
        )
        return 0

    if args.compile_fail:
        return _compile_fail_examples(
            manifest, current, source_dir, args.compiler
        )

    # --check is the CTest-facing mode.  With no explicit option, being strict
    # is more useful for CI and makes a bare invocation safe.
    errors = verify(
        manifest,
        current,
        source_file_count=len(list(source_dir.glob("*.md"))),
        source_dir=source_dir,
    )
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1
    print(
        f"OK: {len(current)} fenced blocks verified; "
        f"{sum(block['language'] == 'c' for block in current)} C blocks; "
        "no omissions, duplicates, or hash drift"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
