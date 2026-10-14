#!/usr/bin/env python3
"""Enforce the project's three size rules on C++ sources.

  1. No file may define more than 5 functions.
  2. No function body may exceed 60 lines.
  3. No line may exceed 110 characters.

A function definition is detected when it starts at column 0 and its opening brace
sits on the same line as the closing parenthesis of its signature. Declarations (no
body) are not counted. Braces inside comments, strings and character literals are
ignored.
"""

import os
import re
import sys

MAX_FUNCS = 5
MAX_FUNC_LINES = 60
MAX_LINE_LEN = 110
SOURCE_DIRS = ("include", "src", "tests")
SKIP_DIRS = {"build", ".git", ".cache", "vendor", "third_party"}
CONTROL = {
    "if", "for", "while", "switch", "catch", "else", "do", "return",
    "namespace", "class", "struct", "union", "enum", "try", "using", "case",
}
FUNC_RE = re.compile(
    r"^(?:[\w:<>,\*&\s~]+\s+)?(?P<name>[\w:~]+)\s*\([^;{]*\)\s*[^;{]*\{"
)


def strip_line(code, state):
    """Remove comments and string/char literal contents from one line."""
    out = []
    i = 0
    while i < len(code):
        two = code[i:i + 2]
        if state["block"]:
            if two == "*/":
                state["block"] = False
                i += 2
            else:
                i += 1
        elif state["str"] is not None:
            if code[i] == "\\":
                i += 2
            elif code[i] == state["str"]:
                state["str"] = None
                i += 1
            else:
                i += 1
        elif two == "//":
            break
        elif two == "/*":
            state["block"] = True
            i += 2
        elif code[i] in "\"'":
            state["str"] = code[i]
            i += 1
        else:
            out.append(code[i])
            i += 1
    return "".join(out)


def analyze(path):
    """Return (violations, function_count) for one file."""
    with open(path, "r", encoding="utf-8", errors="replace") as handle:
        raw = handle.readlines()

    stripped = []
    state = {"block": False, "str": None}
    for line in raw:
        stripped.append(strip_line(line, state))

    violations = []
    for number, line in enumerate(raw, start=1):
        if len(line.rstrip("\n")) > MAX_LINE_LEN:
            violations.append(f"{path}:{number}: line exceeds {MAX_LINE_LEN} cols")

    starts = []
    for index, code in enumerate(stripped):
        if not code or code[0].isspace():
            continue  # only top-level definitions start at column 0
        match = FUNC_RE.match(code)
        if match and match.group("name") not in CONTROL:
            starts.append(index)

    for start in starts:
        depth = 0
        end = None
        for index in range(start, len(stripped)):
            depth += stripped[index].count("{") - stripped[index].count("}")
            if depth <= 0 and index > start:
                end = index
                break
            if depth <= 0 and index == start:
                end = index
                break
        if end is None:
            end = len(stripped) - 1
        length = end - start + 1
        if length > MAX_FUNC_LINES:
            violations.append(
                f"{path}:{start + 1}: function body is {length} lines (max {MAX_FUNC_LINES})"
            )

    count = len(starts)
    if count > MAX_FUNCS:
        violations.append(f"{path}: defines {count} functions (max {MAX_FUNCS})")
    return violations, count


def main(argv):
    root = argv[1] if len(argv) > 1 else "."
    violations = []
    total_files = 0
    total_funcs = 0
    for base in SOURCE_DIRS:
        for dirpath, dirnames, filenames in os.walk(os.path.join(root, base)):
            dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
            for name in filenames:
                if not name.endswith((".h", ".hpp", ".cpp")):
                    continue
                found, count = analyze(os.path.join(dirpath, name))
                violations.extend(found)
                total_files += 1
                total_funcs += count
    for line in violations:
        print(line)
    print(f"style_rules: {total_files} files, {total_funcs} functions, "
          f"{len(violations)} violation(s)")
    return 1 if violations else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
