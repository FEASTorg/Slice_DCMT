#!/usr/bin/env python3

import os
import sys
import yaml
import argparse
from datetime import datetime
from pathlib import Path

try:
    import markdown2
except ImportError:
    markdown2 = None


def load_config(config_file: Path):
    with config_file.open("r") as f:
        return yaml.safe_load(f)


def generate_markdown(project_name: str, artifacts: list[str], timestamp: str) -> str:
    lines = [
        f"# Docs & KiBot Outputs for {project_name}",
        "",
        "The following files were generated from the latest PCB build:",
        "",
    ]
    for artifact in artifacts:
        lines.append(f"- [{artifact}](./{artifact})")
    lines.append("")
    lines.append(f"_Last updated automatically on {timestamp} by GitHub Actions._")
    lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Generate index.md and index.html from KiBot config."
    )
    parser.add_argument(
        "-c", "--config", type=Path, default=Path("docs/site_config.yml")
    )
    parser.add_argument("-o", "--output-dir", type=Path, default=Path("docs"))
    parser.add_argument(
        "--html", action="store_true", help="Also generate index.html using markdown2"
    )

    args = parser.parse_args()
    cfg = load_config(args.config)

    timestamp = datetime.now().strftime("%Y-%m-%d at %H:%M:%S %Z")
    md = generate_markdown(cfg["project_name"], cfg["artifacts"], timestamp)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / "index.md").write_text(md, encoding="utf-8")

    print("✔ index.md generated.")

    if args.html:
        if markdown2:
            html = markdown2.markdown(md)
            (args.output_dir / "index.html").write_text(html, encoding="utf-8")
            print("✔ index.html generated.")
        else:
            print("⚠ markdown2 not installed; skipping HTML generation.")


if __name__ == "__main__":
    main()
