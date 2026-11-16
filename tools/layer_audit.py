#!/usr/bin/env python3
"""
Keira Layer Separation & Complexity Audit Tool
Static analysis for the Keira (lilka-dev/keira) C++ codebase: architectural
layering, complexity, code duplication, named-constant and convention rules
(see AGENTS.md). Rules are declared in tools/layer_audit_rules.json
(or pass --rules). Run from the repository root.
"""

import argparse
import collections
import json
import os
import re
import sys


class Colors:
    GREEN = "\033[92m"
    RED = "\033[91m"
    YELLOW = "\033[93m"
    CYAN = "\033[96m"
    BOLD = "\033[1m"
    RESET = "\033[0m"


DEFAULT_EXTS = (".cpp", ".cc", ".c", ".h", ".hpp")
DEFAULT_SOURCE_EXTS = (".cpp", ".cc", ".c")
DEFAULT_HEADER_EXTS = (".h", ".hpp")
DEFAULT_EXCLUDE_DIRS = [".git", ".pio", "build", "managed_components", ".board_cache"]

# Comments, string and char literals. Literals never span lines.
COMMENT_AND_STRING_RE = re.compile(
    r'//.*?$|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'',
    re.DOTALL | re.MULTILINE,
)

BRANCH_TOKEN_RE = re.compile(
    r"\b(?:if|else\s+if|for|while|case|catch)\b|\&\&|\|\||\?"
)

# Function / method header: allows C++ qualified names (Class::method),
# templates, references, destructors and constructor init lists.
FUNC_START_RE = re.compile(
    r"^(?:static\s+|inline\s+|constexpr\s+|virtual\s+|explicit\s+|const\s+|unsigned\s+|signed\s+|struct\s+|enum\s+|"
    r"__attribute__\s*\(\([^)]*\)\)\s*)*"
    r"[a-zA-Z_][\w\s\*&:<>,~]*?\b([a-zA-Z_~]\w*)\s*\([^{;]*\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?\{?\s*$"
)

NOT_FUNCTION_NAMES = ("if", "for", "while", "switch", "sizeof", "catch", "else", "return")

DEFINE_MACRO_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)\b(.*)$")

ARRAY_DECL_MAGIC_RE = re.compile(
    r"\b(?:char|bool|float|double|uint8_t|int8_t|uint16_t|int16_t|uint32_t|int32_t|uint64_t|int64_t|"
    r"int|long|short|unsigned|size_t)\s+"
    r"[a-zA-Z_]\w*\s*\[\s*(\d+|0x[0-9a-fA-F]+)\s*\]"
)

NUMERIC_LITERAL_RE = re.compile(
    r"(?<![.\w])(0x[0-9a-fA-F]+|0[0-7]{3,}|\d+)(?:[uUlL]*)\b(?![.\w])"
)

# A single-line named constant definition: its literal IS the named constant.
CONST_DEFINITION_RE = re.compile(
    r"^\s*(?:static\s+)?(?:inline\s+)?(?:constexpr|const)\b[^(){}]*=\s*[^;{}]*;\s*$"
)

NAMESPACE_OPEN_RE = re.compile(r'^\s*(?:inline\s+)?namespace\b[^;{]*\{|^\s*extern\s+""\s*\{')

NON_FUNC_PREFIXES = (
    "if", "for", "while", "switch", "#", "//", "/*", "typedef", "return",
    "namespace", "extern", "else", "do", "using", "case",
)

TRIVIAL_NORMALIZED_LINES = {
    "{", "}", "};", "break;", "continue;", "return;", "return 0;", "return -1;",
    "return 1;", "return true;", "return false;", "return NULL;", "return nullptr;",
    "else {", "} else {", "#endif", "#else", "public:", "private:", "protected:",
}

GLOBAL_EXCLUDE_KEYWORDS = (
    "typedef ", "struct ", "union ", "enum ", "class ", "namespace ", "template", "using ",
    "friend ", "const ", "constexpr ", "extern ", "return ", "if ", "for ", "while ",
    "switch ", "case ",
)

GLOBAL_VAR_RE = re.compile(
    r"^(?:static\s+)?(?:[\w:<>,\*&]+\s+)+[\w\*&]+(?:\s*\[.*\])?\s*(?:=.*;|;|\{.*\}\s*;)$"
)


def strip_comments_and_strings_keep_lines(content, keep_strings=False):
    """Blank out comments (and string/char literals unless keep_strings), preserving line numbers."""
    def _repl(match):
        text = match.group(0)
        if text.startswith(('"', "'")):
            if keep_strings:
                return text
            return '""' + "\n" * text.count("\n")
        return "".join("\n" if ch == "\n" else " " for ch in text)

    return COMMENT_AND_STRING_RE.sub(_repl, content)


def load_lines(filepath, keep_strings=False):
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        raw = f.read()
    clean = strip_comments_and_strings_keep_lines(raw, keep_strings).splitlines()
    raw_lines = raw.splitlines()
    if len(clean) < len(raw_lines):
        clean += [""] * (len(raw_lines) - len(clean))
    return raw_lines, clean


def get_exts(rule, default=DEFAULT_EXTS):
    return tuple(rule.get("file_extensions", default))


def parse_int_literal(tok):
    try:
        t = tok.rstrip("uUlL")
        if t.lower().startswith("0x"):
            return int(t, 16)
        if t.startswith("0") and len(t) > 1 and all(c in "01234567" for c in t):
            return int(t, 8)
        return int(t, 10)
    except ValueError:
        return None


def iter_target_files(paths, file_exts, exclude_dirs=None, exclude_files=None):
    exclude_dirs_set = set(DEFAULT_EXCLUDE_DIRS if exclude_dirs is None else exclude_dirs)
    exclude_files_set = set(exclude_files or [])

    def _excluded(norm_path):
        return norm_path in exclude_files_set or os.path.basename(norm_path) in exclude_files_set

    for dir_path in paths:
        if not os.path.exists(dir_path):
            continue
        if os.path.isfile(dir_path):
            if dir_path.endswith(file_exts) and not _excluded(dir_path.replace("\\", "/")):
                yield dir_path
            continue
        for root, dirs, files in os.walk(dir_path):
            dirs[:] = sorted(d for d in dirs if d not in exclude_dirs_set)
            for file in sorted(files):
                if not file.endswith(file_exts):
                    continue
                filepath = os.path.join(root, file)
                if _excluded(filepath.replace("\\", "/")):
                    continue
                yield filepath


def is_file_scope_global_variable(line):
    stripped = line.strip()
    if not stripped or stripped.startswith(("//", "/*", "*", "#")):
        return False
    if stripped in ("};", "}") or "};" == stripped[-2:] and "=" not in stripped:
        return False
    if stripped.startswith("."):
        return False
    padded = stripped + " "
    if any(kw in padded for kw in GLOBAL_EXCLUDE_KEYWORDS):
        return False
    if "(" in stripped and ")" in stripped:
        return False
    return bool(GLOBAL_VAR_RE.match(stripped))


def report_header(name):
    print(f"\n{Colors.BOLD}=== Running check: {name} ==={Colors.RESET}")


def report_footer(violations, name, what="violation(s)"):
    if violations == 0:
        print(f"  [{Colors.GREEN}PASS{Colors.RESET}] No violations found for {name}.")
    else:
        print(f"  [{Colors.RED}FAIL{Colors.RESET}] Found {violations} {what} for {name}.")
    return violations


def run_rule_forbidden_patterns(rule):
    """scan_strings=true keeps string literals (needed to match #include "path")."""
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule)
    compiled = [re.compile(p) for p in rule.get("patterns", [])]
    keep_strings = bool(rule.get("scan_strings", False))

    report_header(name)
    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            raw_lines, clean_lines = load_lines(filepath, keep_strings)
            for idx, (rl, cl) in enumerate(zip(raw_lines, clean_lines), 1):
                if not cl.strip():
                    continue
                if any(pat.search(cl) for pat in compiled):
                    print(f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}:{idx}")
                    print(f"    Match: {rl.strip()}")
                    violations += 1
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
    return report_footer(violations, name)


def run_rule_max_pattern_occurrences_per_file(rule):
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule)
    max_count = int(rule.get("max_count", 5))
    pat = re.compile(rule.get("pattern", ""))
    keep_strings = bool(rule.get("scan_strings", False))

    report_header(name)
    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            raw_lines, clean_lines = load_lines(filepath, keep_strings)
            matches = [(i, raw_lines[i - 1].strip()) for i, cl in enumerate(clean_lines, 1) if pat.search(cl)]
            if len(matches) > max_count:
                print(
                    f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}:{matches[0][0]} "
                    f"({len(matches)} occurrences > max {max_count})"
                )
                for m_idx, m_text in matches[:4]:
                    print(f"    Line {m_idx}: {m_text}")
                if len(matches) > 4:
                    print(f"    ... (+{len(matches) - 4} more occurrences)")
                violations += 1
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
    return report_footer(violations, name, "file(s) exceeding pattern threshold")


def run_rule_function_metrics(rule):
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule, DEFAULT_SOURCE_EXTS)
    max_lines = rule.get("max_lines")
    max_branches = rule.get("max_branches")
    max_indent_levels = rule.get("max_indent_levels")
    max_case_labels = rule.get("max_case_labels")

    report_header(name)
    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            _, clean_lines = load_lines(filepath)
            i = 0
            n_lines = len(clean_lines)
            while i < n_lines:
                line = clean_lines[i]
                stripped = line.strip()
                if line and not line[0].isspace() and not stripped.startswith(NON_FUNC_PREFIXES) and "=" not in line:
                    sig_text = stripped
                    sig_end = i
                    # Allow multi-line signatures (and a preceding template<> line) up to 4 lines
                    while (
                        "{" not in clean_lines[sig_end]
                        and ";" not in clean_lines[sig_end]
                        and sig_end + 1 < n_lines
                        and (sig_end - i) < 4
                    ):
                        sig_end += 1
                        sig_text += " " + clean_lines[sig_end].strip()

                    m = FUNC_START_RE.match(sig_text)
                    if m and "{" in clean_lines[sig_end] and m.group(1) not in NOT_FUNCTION_NAMES:
                        func_name = m.group(1)
                        start_idx = i
                        brace_depth = 0
                        found_open = False
                        branches = 0
                        case_count = 0
                        max_nesting = 0
                        j = i
                        while j < n_lines:
                            cl = clean_lines[j]
                            opens = cl.count("{")
                            closes = cl.count("}")
                            if opens > 0:
                                found_open = True
                            current_nesting = max(0, brace_depth - 1) if found_open else 0
                            if cl.strip():
                                max_nesting = max(max_nesting, current_nesting)
                            brace_depth += opens - closes
                            branches += len(BRANCH_TOKEN_RE.findall(cl))
                            case_count += len(re.findall(r"\bcase\b", cl))
                            j += 1
                            if found_open and brace_depth <= 0:
                                break

                        if found_open and brace_depth == 0:
                            func_len = j - start_idx
                            reasons = []
                            if max_lines is not None and func_len > max_lines:
                                reasons.append(f"lines={func_len} > {max_lines}")
                            if max_branches is not None and branches > max_branches:
                                reasons.append(f"branches={branches} > {max_branches}")
                            if max_indent_levels is not None and max_nesting > max_indent_levels:
                                reasons.append(f"nesting={max_nesting} > {max_indent_levels}")
                            if max_case_labels is not None and case_count > max_case_labels:
                                reasons.append(f"cases={case_count} > {max_case_labels}")
                            if reasons:
                                print(
                                    f"  [{Colors.RED}VIOLATION{Colors.RESET}] "
                                    f"{filepath}:{start_idx + 1} in {func_name}() [{', '.join(reasons)}]"
                                )
                                violations += 1
                            i = j
                            continue
                i += 1
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
    if violations == 0:
        print(f"  [{Colors.GREEN}PASS{Colors.RESET}] All functions meet complexity and size limits for {name}.")
        return 0
    return report_footer(violations, name, "function complexity violation(s)")


def run_rule_duplicate_blocks(rule):
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule)
    window_size = int(rule.get("min_normalized_lines", 12))
    cross_file_only = bool(rule.get("cross_file_only", True))

    report_header(name)
    chunks = collections.defaultdict(list)

    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            raw_lines, clean_lines = load_lines(filepath)
            norm = []
            for idx, (cl, rl) in enumerate(zip(clean_lines, raw_lines), 1):
                s = re.sub(r"\s+", " ", cl).strip()
                raw_s = re.sub(r"\s+", " ", rl.split("//")[0]).strip()
                if not s or s in TRIVIAL_NORMALIZED_LINES or s.startswith("#"):
                    continue
                # Skip pure data/table rows (bitmaps, designated initializers)
                if raw_s.startswith(("{0x", ".")):
                    continue
                norm.append((idx, raw_s))
            for i in range(len(norm) - window_size + 1):
                block = tuple(x[1] for x in norm[i : i + window_size])
                chunks[block].append((filepath, norm[i][0], norm[i + window_size - 1][0]))
        except Exception as e:
            print(f"Error reading {filepath}: {e}")

    covered_lines = collections.defaultdict(set)
    for block, locs in sorted(chunks.items(), key=lambda item: (len({x[0] for x in item[1]}), len(item[1])), reverse=True):
        unique_files = {x[0] for x in locs}
        if len(locs) < 2:
            continue
        if cross_file_only and len(unique_files) < 2:
            continue
        if all(any(line in covered_lines[f] for line in range(start, end + 1)) for f, start, end in locs):
            continue
        for f, start, end in locs:
            for line in range(start - 5, end + 6):
                covered_lines[f].add(line)

        loc_summary = ", ".join(f"{f}:{start}-{end}" for f, start, end in locs[:5])
        if len(locs) > 5:
            loc_summary += f" (+{len(locs) - 5} more)"
        print(
            f"  [{Colors.RED}VIOLATION{Colors.RESET}] Duplicate {window_size}-line block "
            f"({len(locs)}x across {len(unique_files)} file(s)): {loc_summary}"
        )
        for sample in block[:3]:
            print(f"    | {sample[:95]}")
        violations += 1

    if violations == 0:
        print(f"  [{Colors.GREEN}PASS{Colors.RESET}] No duplicate code blocks found for {name}.")
        return 0
    return report_footer(violations, name, "duplicate code block group(s)")


def run_rule_macros_in_headers_only(rule):
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule, DEFAULT_SOURCE_EXTS)
    allowed_macros = set(rule.get("allowed_macros", []))
    allowed_patterns = [re.compile(p) for p in rule.get("allowed_patterns", [])]

    report_header(name)
    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            raw_lines, clean_lines = load_lines(filepath)
            for idx, (cl, rl) in enumerate(zip(clean_lines, raw_lines), 1):
                m = DEFINE_MACRO_RE.match(cl)
                if not m:
                    continue
                macro_name = m.group(1)
                if macro_name in allowed_macros or any(p.search(macro_name) for p in allowed_patterns):
                    continue
                print(
                    f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}:{idx} "
                    f"defines '#define {macro_name}' inside a source file (move to a header)"
                )
                print(f"    Match: {rl.strip()}")
                violations += 1
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
    if violations == 0:
        print(f"  [{Colors.GREEN}PASS{Colors.RESET}] No unauthorized #define macros found in source files.")
        return 0
    return report_footer(violations, name, "#define macro(s) in source files that belong in headers")


def run_rule_magic_numbers(rule):
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule, DEFAULT_SOURCE_EXTS)
    ignore_const_tables = bool(rule.get("ignore_const_tables", True))
    flag_array_declarators = bool(rule.get("flag_array_declarators", True))
    allowed_array_sizes = set(rule.get("allowed_array_sizes", [1, 2, 3, 4, 6]))
    allowed_literals = set(rule.get("allowed_literals", [-1, 0, 1, 2, 3, 4, 8, 10, 16, 100]))
    flagged_literals = rule.get("flagged_literals", {})
    max_cross_file = rule.get("max_cross_file_occurrences", 4)
    min_cross_file_value = int(rule.get("min_cross_file_value", 36))

    flagged_by_str = {str(k).lower(): v for k, v in flagged_literals.items()}
    flagged_decimal_by_val = {}
    for k, v in flagged_literals.items():
        ks = str(k).strip()
        if not ks.lower().startswith("0"):
            parsed = parse_int_literal(ks)
            if parsed is not None:
                flagged_decimal_by_val[parsed] = (ks, v)

    cross_file_tracker = collections.defaultdict(list)

    report_header(name)
    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            raw_lines, clean_lines = load_lines(filepath)
            in_const_table_depth = 0
            in_enum_depth = 0

            for idx, (cl, rl) in enumerate(zip(clean_lines, raw_lines), 1):
                s = cl.strip()
                if not s or s.startswith("#"):
                    continue

                # enum constant definitions are not magic numbers
                if re.search(r"\benum\b", s) and "{" in s:
                    in_enum_depth += s.count("{") - s.count("}")
                    continue
                if in_enum_depth > 0:
                    in_enum_depth += s.count("{") - s.count("}")
                    continue

                if ignore_const_tables:
                    if in_const_table_depth == 0 and (
                        re.search(r"\bconst(?:expr)?\b.*=\s*\{", s)
                        or re.search(r"\(\s*const\s+\w+\s*\[\s*\d+\s*\]\s*\)\s*\{", s)
                    ):
                        in_const_table_depth = s.count("{") - s.count("}")
                        continue
                    elif in_const_table_depth > 0:
                        in_const_table_depth += s.count("{") - s.count("}")
                        continue

                # `constexpr int W = 280;` is the definition of the named constant itself
                if CONST_DEFINITION_RE.match(cl):
                    continue

                flagged_spans = []
                if flag_array_declarators:
                    for m_arr in ARRAY_DECL_MAGIC_RE.finditer(s):
                        arr_tok = m_arr.group(1)
                        arr_val = parse_int_literal(arr_tok)
                        if arr_val is not None and arr_val not in allowed_array_sizes:
                            flagged_spans.append(m_arr.span(1))
                            print(
                                f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}:{idx} "
                                f"hardcoded array size [{arr_tok}] (use a named constant)"
                            )
                            print(f"    Match: {rl.strip()}")
                            violations += 1

                for m_num in NUMERIC_LITERAL_RE.finditer(s):
                    if any(start <= m_num.start(1) and m_num.end(1) <= end for start, end in flagged_spans):
                        continue
                    tok = m_num.group(1)
                    val = parse_int_literal(tok)
                    if val is None:
                        continue

                    tok_lower = tok.lower()
                    is_hex_or_oct = tok_lower.startswith("0x") or (tok_lower.startswith("0") and len(tok_lower) > 1)
                    reason = None
                    if tok_lower in flagged_by_str:
                        reason = flagged_by_str[tok_lower]
                    elif not is_hex_or_oct and val in flagged_decimal_by_val:
                        reason = flagged_decimal_by_val[val][1]

                    if reason:
                        print(
                            f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}:{idx} "
                            f"magic constant '{tok}': {reason}"
                        )
                        print(f"    Match: {rl.strip()}")
                        violations += 1
                    elif val not in allowed_literals and (is_hex_or_oct or abs(val) >= min_cross_file_value):
                        cross_file_tracker[tok].append((filepath, idx, rl.strip()))
        except Exception as e:
            print(f"Error reading {filepath}: {e}")

    if max_cross_file is not None:
        for tok, locs in sorted(
            cross_file_tracker.items(),
            key=lambda item: (len({x[0] for x in item[1]}), len(item[1])),
            reverse=True,
        ):
            unique_files = sorted({x[0] for x in locs})
            if len(unique_files) < int(max_cross_file):
                continue
            loc_preview = ", ".join(f"{f}:{l}" for f, l, _ in locs[:5])
            if len(locs) > 5:
                loc_preview += f" (+{len(locs) - 5} more)"
            print(
                f"  [{Colors.RED}VIOLATION{Colors.RESET}] Repeated magic literal '{tok}' "
                f"({len(locs)}x across {len(unique_files)} files — move to a shared header constant): {loc_preview}"
            )
            violations += 1

    if violations == 0:
        print(f"  [{Colors.GREEN}PASS{Colors.RESET}] No magic number violations found for {name}.")
        return 0
    return report_footer(violations, name, "magic number / array bound violation(s)")


def run_rule_no_calls_between_patterns(rule):
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule)
    lock_patterns = [re.compile(p) for p in rule.get("lock_patterns", [])]
    unlock_patterns = [re.compile(p) for p in rule.get("unlock_patterns", [])]
    forbidden_calls = [re.compile(p) for p in rule.get("forbidden_calls", [])]

    report_header(name)
    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            raw_lines, clean_lines = load_lines(filepath)
            lock_stack = []
            for idx, (rl, cl) in enumerate(zip(raw_lines, clean_lines), 1):
                if not cl.strip():
                    continue
                for lpat in lock_patterns:
                    if lpat.search(cl):
                        lock_stack.append(idx)
                if lock_stack:
                    for fpat in forbidden_calls:
                        if fpat.search(cl):
                            print(f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}:{idx}")
                            print(f"    Forbidden call while lock held (acquired at line {lock_stack[-1]}): {rl.strip()}")
                            violations += 1
                for upat in unlock_patterns:
                    if upat.search(cl) and lock_stack:
                        lock_stack.pop()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
    return report_footer(violations, name)


def run_rule_no_globals(rule):
    """Mutable file-scope variables. Braces of namespace / extern "C" blocks do not count as nesting."""
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule)

    report_header(name)
    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            raw_lines, clean_lines = load_lines(filepath)
            stack = []  # "ns" for namespace-like blocks, "code" for everything else
            for idx, (rl, cl) in enumerate(zip(raw_lines, clean_lines), 1):
                s = cl.strip()
                if not s or s.startswith("#"):
                    continue
                if "code" not in stack and is_file_scope_global_variable(cl):
                    print(f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}:{idx}")
                    print(f"    Match: {rl.strip()}")
                    violations += 1
                first_open_is_ns = bool(NAMESPACE_OPEN_RE.match(cl))
                for ch in cl:
                    if ch == "{":
                        stack.append("ns" if first_open_is_ns else "code")
                        first_open_is_ns = False
                    elif ch == "}" and stack:
                        stack.pop()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
    return report_footer(violations, name)


def run_rule_include_guards(rule):
    """Headers must have #pragma once (require_pragma_once=true) or #pragma once / #ifndef."""
    violations = 0
    name = rule.get("name", rule.get("id"))
    exts = get_exts(rule, DEFAULT_HEADER_EXTS)
    require_pragma = bool(rule.get("require_pragma_once", False))

    report_header(name)
    for filepath in iter_target_files(rule.get("paths", []), exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        try:
            _, clean_lines = load_lines(filepath)
            text = "\n".join(clean_lines)
            has_pragma = re.search(r"^\s*#\s*pragma\s+once\b", text, re.MULTILINE) is not None
            has_ifndef = re.search(r"^\s*#\s*ifndef\b", text, re.MULTILINE) is not None
            if not (has_pragma or (has_ifndef and not require_pragma)):
                print(f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}: missing #pragma once" +
                      ("" if require_pragma else " / include guard"))
                violations += 1
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
    return report_footer(violations, name)


def run_rule_sibling_isolation(rule):
    """Modules under base_dir/<name>/ must not include another sibling module (except 'shared' ones)."""
    violations = 0
    name = rule.get("name", rule.get("id"))
    base_dir = rule.get("base_dir", "src/apps")
    prefix = rule.get("include_prefix", "apps/")
    shared = set(rule.get("shared", []))
    exts = get_exts(rule)
    inc_re = re.compile(r'^\s*#\s*include\s*"' + re.escape(prefix) + r'([^/"]+)/')

    report_header(name)
    for filepath in iter_target_files([base_dir], exts, rule.get("exclude_dirs"), rule.get("exclude_files")):
        parts = os.path.relpath(filepath, base_dir).replace("\\", "/").split("/")
        if len(parts) < 2 or parts[0] in shared:
            continue
        owner = parts[0]
        try:
            raw_lines, clean_lines = load_lines(filepath, keep_strings=True)
            for idx, (rl, cl) in enumerate(zip(raw_lines, clean_lines), 1):
                m = inc_re.match(cl)
                if m and m.group(1) != owner and m.group(1) not in shared:
                    print(f"  [{Colors.RED}VIOLATION{Colors.RESET}] {filepath}:{idx} "
                          f"'{owner}' depends on sibling '{m.group(1)}'")
                    print(f"    Match: {rl.strip()}")
                    violations += 1
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
    return report_footer(violations, name)


def run_rule_allowed_directory(rule):
    violations = 0
    name = rule.get("name", rule.get("id"))
    base_dir = rule.get("base_dir", ".")
    file_exts = tuple(rule.get("file_extensions", []))
    allowed_prefix = os.path.abspath(rule.get("allowed_prefix", ""))
    exclude_dirs = rule.get("exclude_dirs", DEFAULT_EXCLUDE_DIRS)

    report_header(name)
    for root, dirs, files in os.walk(base_dir):
        dirs[:] = [d for d in dirs if d not in exclude_dirs]
        for file in files:
            if file.endswith(file_exts):
                filepath = os.path.abspath(os.path.join(root, file))
                if not filepath.startswith(allowed_prefix):
                    print(f"  [{Colors.RED}VIOLATION{Colors.RESET}] File found outside {rule.get('allowed_prefix')}: {filepath}")
                    violations += 1
    return report_footer(violations, name, "misplaced file(s)")


DISPATCHER = {
    "forbidden_patterns": run_rule_forbidden_patterns,
    "max_pattern_occurrences_per_file": run_rule_max_pattern_occurrences_per_file,
    "function_metrics": run_rule_function_metrics,
    "duplicate_blocks": run_rule_duplicate_blocks,
    "macros_in_headers_only": run_rule_macros_in_headers_only,
    "magic_numbers": run_rule_magic_numbers,
    "no_calls_between_patterns": run_rule_no_calls_between_patterns,
    "no_globals": run_rule_no_globals,
    "mandatory_include_guards": run_rule_include_guards,
    "sibling_isolation": run_rule_sibling_isolation,
    "allowed_directory": run_rule_allowed_directory,
}


def load_rules(rules_file):
    if not os.path.exists(rules_file):
        print(f"{Colors.RED}Rules definition file not found: {rules_file}{Colors.RESET}")
        sys.exit(2)
    with open(rules_file, "r", encoding="utf-8") as f:
        data = json.load(f)
    return data.get("rules", [])


def main():
    parser = argparse.ArgumentParser(description="Keira Declarative Layer Separation & Complexity Audit Tool")
    parser.add_argument("-r", "--rules", default="tools/layer_audit_rules.json",
                        help="Path to JSON rules definition (default: tools/layer_audit_rules.json)")
    parser.add_argument("--rule-id", action="append", dest="rule_ids",
                        help="Run only specific rule ID(s) (repeatable)")
    parser.add_argument("--list", action="store_true", help="List rule IDs and exit")
    args = parser.parse_args()

    rules = load_rules(args.rules)
    if args.list:
        for rule in rules:
            print(f"{rule.get('id'):36} {rule.get('type'):32} {rule.get('name', '')}")
        sys.exit(0)

    print(f"{Colors.CYAN}{Colors.BOLD}Running Keira Layer Separation Audit ({args.rules})...{Colors.RESET}")
    total_violations = 0
    executed_rules = 0
    for rule in rules:
        if args.rule_ids and rule.get("id") not in args.rule_ids:
            continue
        handler = DISPATCHER.get(rule.get("type"))
        if not handler:
            print(f"{Colors.YELLOW}[WARN] Unknown rule type '{rule.get('type')}' for rule '{rule.get('id')}'; skipping.{Colors.RESET}")
            continue
        total_violations += handler(rule)
        executed_rules += 1

    print(f"\n{Colors.BOLD}========================================{Colors.RESET}")
    print(f"Executed {executed_rules} architectural rules.")
    if total_violations == 0:
        print(f"{Colors.GREEN}{Colors.BOLD}>>> ALL LAYER AUDIT CHECKS PASSED! <<<{Colors.RESET}\n")
        sys.exit(0)
    print(f"{Colors.RED}{Colors.BOLD}>>> AUDIT FAILED: {total_violations} architectural violation(s) found! <<<{Colors.RESET}\n")
    sys.exit(1)


if __name__ == "__main__":
    main()