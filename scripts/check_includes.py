#!/usr/bin/env python3
"""Verify each C++ file directly includes the standard headers it uses.

MSVC does not pull in <vector>/<string>/<map>/... transitively the way GCC/Clang
often do, so a file that compiles on Linux can fail to build on Windows. This
check catches that class of bug early.
"""

import os
import re
import sys

RULES = {
    "std::vector": "vector",
    "std::string": "string",
    "std::map": "map",
    "std::complex": "complex",
    "std::pair": "utility",
    "std::move": "utility",
    "std::shared_ptr": "memory",
    "std::unique_ptr": "memory",
    "std::optional": "optional",
    "std::filesystem": "filesystem",
    "std::size_t": "cstddef",
    "std::numeric_limits": "limits",
    "std::sort": "algorithm",
    "std::clamp": "algorithm",
    "std::min": "algorithm",
    "std::max": "algorithm",
    "std::all_of": "algorithm",
    "std::any_of": "algorithm",
    "std::find": "algorithm",
    "std::count": "algorithm",
    "std::fill": "algorithm",
    "std::sin": "cmath",
    "std::cos": "cmath",
    "std::exp": "cmath",
    "std::log": "cmath",
    "std::log10": "cmath",
    "std::pow": "cmath",
    "std::fabs": "cmath",
    "std::sqrt": "cmath",
    "std::isfinite": "cmath",
    "std::tanh": "cmath",
    "std::round": "cmath",
    "std::lround": "cmath",
    "std::fmod": "cmath",
}
ROOTS = ("include", "src", "tests", "app")
SKIP_DIRS = {"build", ".git", ".cache"}


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


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
                body = strip_comments(text)
                missing = set()
                for token, header in RULES.items():
                    if token in body and f"#include <{header}>" not in text:
                        missing.add(header)
                if missing:
                    headers = ", ".join(f"<{h}>" for h in sorted(missing))
                    problems.append(f"{path}: missing {headers}")
    for problem in problems:
        print(problem)
    print(f"check_includes: {len(problems)} problem(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "."))
