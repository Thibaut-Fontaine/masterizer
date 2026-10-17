#!/usr/bin/env python3
"""Flag platform-specific constructs that would break Windows/Linux portability."""

import os
import re
import sys

BANNED = [
    (r"#include\s*<unistd\.h>", "POSIX-only <unistd.h>"),
    (r"#include\s*<sys/", "POSIX-only <sys/...>"),
    (r"#include\s*<pthread\.h>", "POSIX-only <pthread.h>"),
    (r"#include\s*<windows\.h>", "Windows-only <windows.h>"),
    (r"#include\s*<malloc\.h>", "non-portable <malloc.h>"),
    (r"\b__declspec\b", "MSVC-only __declspec"),
    (r"\b__attribute__\b", "GCC-only __attribute__"),
    (r"#pragma\s+comment", "MSVC-only #pragma comment"),
    (r"\bM_PI\b", "non-standard M_PI (define a local constant)"),
    (r"\bsystem\s*\(", "system() call"),
]
ROOTS = ("include", "src", "tests", "app")
SKIP_DIRS = {"build", ".git", ".cache", "third_party", "vendor"}


def main(root):
    problems = []
    for base in ROOTS:
        for dirpath, dirnames, filenames in os.walk(os.path.join(root, base)):
            dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
            for name in filenames:
                if not name.endswith((".h", ".cpp")):
                    continue
                path = os.path.join(dirpath, name)
                with open(path, encoding="utf-8", errors="replace") as handle:
                    text = handle.read()
                for pattern, reason in BANNED:
                    if re.search(pattern, text):
                        problems.append(f"{path}: {reason}")
    for problem in problems:
        print(problem)
    print(f"check_portability: {len(problems)} problem(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "."))
