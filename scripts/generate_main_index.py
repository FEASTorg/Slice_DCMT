#!/usr/bin/env python3

from pathlib import Path
import fnmatch


def load_ignore(base_dir: Path):
    ignore_file = base_dir / ".indexignore"
    patterns = []
    if ignore_file.exists():
        for line in ignore_file.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            patterns.append(line)
    # always ignore existing indexes and Pages marker
    patterns += ["index.md"]
    return patterns


def should_ignore(path: Path, patterns, base: Path):
    rel_path = path.relative_to(base).as_posix()
    name = path.name
    return any(
        fnmatch.fnmatch(name, pat) or fnmatch.fnmatch(rel_path, pat) for pat in patterns
    )


def make_index(dir_path: Path, base_dir: Path):
    patterns = load_ignore(base_dir)
    readme = dir_path / "README.md"
    index = dir_path / "index.md"
    parts = []

    # 1) README or fallback title
    if readme.exists() and not should_ignore(readme, patterns, base_dir):
        parts.append(readme.read_text(encoding="utf-8"))
        parts.append("\n")
    else:
        title = dir_path.name.replace("_", " ").title() or "Documentation"
        parts.append(f"# {title}\n\n")

    parts.append("## Contents\n\n")

    # 2) List markdown files (excluding index/readme)
    for md in sorted(dir_path.glob("*.md")):
        if should_ignore(md, patterns, base_dir) or md.name.lower() in (
            "readme.md",
            "index.md",
        ):
            continue
        title = md.stem.replace("_", " ").title()
        parts.append(f"- [{title}]({md.name})\n")
    parts.append("\n")

    # 3) Recurse into subdirectories
    for sub in sorted([d for d in dir_path.iterdir() if d.is_dir()]):
        if should_ignore(sub, patterns, base_dir):
            continue
        make_index(sub, base_dir)
        title = sub.name.replace("_", " ").title()
        parts.append(f"### [{title}]({sub.name})\n")

    index.write_text("".join(parts), encoding="utf-8")
    print(f"Generated {index}")


if __name__ == "__main__":
    make_index(Path("docs"), Path("docs"))
