#!/usr/bin/env python3
"""Safely smoke-check platform examples for C chapters 17-22 and S09."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import platform
import selectors
import shutil
import socket
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "platform"
C_FLAGS = ["-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]


class CheckFailure(Exception):
    """A fixture or command did not pass its check."""


class SkipCheck(Exception):
    """The current host cannot provide a meaningful check."""


def run_command(command: list[str], cwd: Path, timeout: float = 10.0) -> str:
    """Run a fixed argument vector and return combined output."""
    try:
        result = subprocess.run(
            command,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=timeout,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        raise CheckFailure(
            "命令超时：{}\n{}".format(" ".join(command), exc.stdout or "")
        ) from exc
    except OSError as exc:
        raise CheckFailure("无法执行 {}：{}".format(command[0], exc)) from exc

    if result.returncode != 0:
        output = result.stdout.strip()
        if len(output) > 2000:
            output = output[-2000:]
        detail = "\n{}".format(output) if output else ""
        raise CheckFailure(
            "命令退出码 {}：{}{}".format(result.returncode, " ".join(command), detail)
        )
    return result.stdout


def require_files(*names: str) -> None:
    missing = [name for name in names if not (FIXTURES / name).is_file()]
    if missing:
        raise CheckFailure("缺少验证夹具：{}".format(", ".join(missing)))


def compiler() -> str:
    for name in ("cc", "gcc", "clang"):
        found = shutil.which(name)
        if found:
            return found
    raise SkipCheck("未找到 C 编译器（cc/gcc/clang）")


def gnu_make() -> str:
    found = shutil.which("make")
    if not found:
        raise SkipCheck("未找到 make")
    version = run_command([found, "--version"], ROOT)
    if not version.startswith("GNU Make"):
        raise SkipCheck("make 不是 GNU Make")
    return found


def copy_fixtures(directory: Path, *names: str) -> None:
    for name in names:
        shutil.copy2(FIXTURES / name, directory / name)


def check_c_runtime() -> None:
    require_files("byte_order_sizeof.c")
    cc = compiler()
    with tempfile.TemporaryDirectory(prefix="c-doc-platform-") as raw:
        directory = Path(raw)
        source = directory / "byte_order_sizeof.c"
        copy_fixtures(directory, source.name)
        executable = directory / "byte_order_sizeof"
        run_command([cc, *C_FLAGS, str(source), "-o", str(executable)], directory)
        output = run_command([str(executable)], directory).strip()
        if (not output.startswith("byte_order=") or
                "sizeof_int=" not in output or
                "sizeof_pointer=" not in output):
            raise CheckFailure("字节序/sizeof 输出格式不正确：{}".format(output))


def check_preprocessor() -> None:
    require_files("preprocessor_macros.c")
    cc = compiler()
    with tempfile.TemporaryDirectory(prefix="c-doc-preprocessor-") as raw:
        directory = Path(raw)
        source = directory / "preprocessor_macros.c"
        copy_fixtures(directory, source.name)
        preprocessed = run_command(
            [cc, "-E", "-P", "-DDOC_FEATURE=21", str(source)], directory
        )
        if 'feature_text = "21"' not in preprocessed:
            raise CheckFailure("预处理结果没有展开 DOC_FEATURE=21")
        executable = directory / "preprocessor_macros"
        run_command(
            [cc, *C_FLAGS, "-DDOC_FEATURE=21", str(source), "-o", str(executable)],
            directory,
        )
        run_command([str(executable)], directory)


def check_two_file_link() -> None:
    require_files("link_main.c", "link_value.c")
    cc = compiler()
    with tempfile.TemporaryDirectory(prefix="c-doc-link-") as raw:
        directory = Path(raw)
        copy_fixtures(directory, "link_main.c", "link_value.c")
        main_object = directory / "link_main.o"
        value_object = directory / "link_value.o"
        executable = directory / "linked_program"
        run_command(
            [cc, *C_FLAGS, "-c", "link_main.c", "-o", str(main_object)], directory
        )
        run_command(
            [cc, *C_FLAGS, "-c", "link_value.c", "-o", str(value_object)], directory
        )
        run_command(
            [cc, str(main_object), str(value_object), "-o", str(executable)], directory
        )
        run_command([str(executable)], directory)


def check_gnu_make() -> None:
    require_files("Makefile", "link_main.c", "link_value.c")
    if os.name != "posix":
        raise SkipCheck("Makefile 夹具使用 POSIX rm，仅在 POSIX 主机验证")
    cc = compiler()
    make = gnu_make()
    with tempfile.TemporaryDirectory(prefix="c-doc-make-") as raw:
        directory = Path(raw)
        copy_fixtures(directory, "Makefile", "link_main.c", "link_value.c")
        make_args = [
            make,
            "-f",
            "Makefile",
            "CC=" + cc,
            "CFLAGS=" + " ".join(C_FLAGS),
        ]
        run_command(make_args, directory)
        run_command([str(directory / "linked_make")], directory)
        run_command(make_args, directory)
        run_command([make, "-f", "Makefile", "clean"], directory)
        leftovers = [
            directory / "linked_make",
            directory / "link_main.o",
            directory / "link_value.o",
        ]
        if any(path.exists() for path in leftovers):
            raise CheckFailure("make clean 没有清理全部构建产物")


def check_x86_assembly() -> None:
    require_files("asm_add.S")
    machine = platform.machine().lower()
    if machine not in {"x86_64", "amd64"}:
        raise SkipCheck("当前架构为 {}，asm_add.S 仅匹配 x86-64".format(machine or "未知"))
    cc = compiler()
    with tempfile.TemporaryDirectory(prefix="c-doc-asm-") as raw:
        directory = Path(raw)
        source = directory / "asm_add.S"
        copy_fixtures(directory, source.name)
        # 只生成目标文件检查语法/架构；不链接、不运行，避免执行平台相关汇编。
        run_command([cc, "-c", str(source), "-o", str(directory / "asm_add.o")], directory)


def check_http_server() -> None:
    if os.name != "posix":
        raise SkipCheck("简易 HTTP 服务器使用 POSIX socket")
    cc = compiler()
    note_source = ROOT.parent / "S09_简易HTTP协议.md"
    extractor = ROOT / "tools" / "extract_http_sources.py"
    if not note_source.is_file() or not extractor.is_file():
        raise CheckFailure("缺少 S09 HTTP 笔记或源码提取脚本")

    cases = [
        (
            b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",
            b"HTTP/1.1 200 OK",
            b"Hello from C HTTP server\n",
        ),
        (
            b"GET /missing HTTP/1.1\r\nHost: localhost\r\n\r\n",
            b"HTTP/1.1 404 Not Found",
            b"Not Found\n",
        ),
        (
            b"POST / HTTP/1.1\r\nHost: localhost\r\n\r\n",
            b"HTTP/1.1 405 Method Not Allowed",
            b"Method Not Allowed\n",
        ),
        (b"not an HTTP request\r\n\r\n", b"HTTP/1.1 400 Bad Request", b"Bad Request\n"),
    ]

    with tempfile.TemporaryDirectory(prefix="c-doc-http-") as raw:
        directory = Path(raw)
        generated = directory / "generated"
        run_command(
            [
                sys.executable,
                str(extractor),
                "--source",
                str(note_source),
                "--output-dir",
                str(generated),
            ],
            directory,
        )
        executable = directory / "http_server"
        run_command(
            [
                cc,
                *C_FLAGS,
                "-I",
                str(generated),
                str(generated / "http_server.c"),
                str(generated / "http_protocol.c"),
                "-o",
                str(executable),
            ],
            directory,
        )
        for request, status, body in cases:
            process = subprocess.Popen(
                [str(executable), "--port", "0", "--once"],
                cwd=directory,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            try:
                if process.stdout is None:
                    raise CheckFailure("HTTP 服务器没有 stdout")
                selector = selectors.DefaultSelector()
                selector.register(process.stdout, selectors.EVENT_READ)
                if not selector.select(timeout=3.0):
                    raise CheckFailure("HTTP 服务器启动超时")
                line = process.stdout.readline().strip()
                selector.close()
                parts = line.split()
                if len(parts) != 2 or parts[0] != "PORT":
                    raise CheckFailure("HTTP 服务器没有报告端口：{}".format(line))
                port = int(parts[1])
                with socket.create_connection(("127.0.0.1", port), timeout=3.0) as client:
                    client.sendall(request)
                    chunks = []
                    while True:
                        chunk = client.recv(4096)
                        if not chunk:
                            break
                        chunks.append(chunk)
                response = b"".join(chunks)
                if status not in response or body not in response:
                    raise CheckFailure(
                        "HTTP 响应不符合预期：{}".format(response[:300].decode(errors="replace"))
                    )
                process.wait(timeout=3.0)
                if process.returncode != 0:
                    raise CheckFailure("HTTP 服务器退出码：{}".format(process.returncode))
            finally:
                if process.poll() is None:
                    process.terminate()
                    try:
                        process.wait(timeout=1.0)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()


def check_bash() -> None:
    require_files("safe_commands.sh")
    bash = shutil.which("bash")
    if not bash:
        raise SkipCheck("未找到 bash")
    run_command([bash, "-n", str(FIXTURES / "safe_commands.sh")], ROOT)
    output = run_command([bash, str(FIXTURES / "safe_commands.sh")], ROOT).strip()
    if output != "bash_smoke=safe":
        raise CheckFailure("bash smoke 输出不正确：{}".format(output))


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="验证 C 笔记第 17-22 章和 S09 的安全平台夹具"
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="执行检查；不提供此选项时不执行任何命令",
    )
    args = parser.parse_args(argv)
    if not args.check:
        parser.error("必须显式传入 --check")

    checks = [
        ("字节序与 sizeof", check_c_runtime),
        ("预处理宏", check_preprocessor),
        ("两源文件链接", check_two_file_link),
        ("GNU make", check_gnu_make),
        ("x86-64 汇编语法", check_x86_assembly),
        ("简易 HTTP 服务器", check_http_server),
        ("安全 bash 命令", check_bash),
    ]
    failures = 0
    for name, check in checks:
        try:
            check()
        except SkipCheck as exc:
            print("[SKIP] {}: {}".format(name, exc))
        except CheckFailure as exc:
            failures += 1
            print("[FAIL] {}: {}".format(name, exc), file=sys.stderr)
        else:
            print("[PASS] {}".format(name))

    if failures:
        print("检查失败：{} 项".format(failures), file=sys.stderr)
        return 1
    print("检查完成：没有失败项")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        print("检查被中断", file=sys.stderr)
        raise SystemExit(2)
