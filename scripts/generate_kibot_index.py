#!/usr/bin/env python3

import argparse
from datetime import datetime, timezone
from pathlib import Path
import sys
import yaml


def load_config(config_file: Path) -> dict:
    with config_file.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def render_template(
    template: str, project: str, artifacts: list[str], timestamp: str
) -> str:
    # preserve order, drop duplicates
    seen = set()
    uniq = [
        a.strip()
        for a in artifacts
        if a.strip() and (a.lower() not in seen and not seen.add(a.lower()))
    ]
    links_block = "\n".join(f"- [{a}](./{a})" for a in uniq)
    return (
        template.replace("$PROJECT_NAME", project)
        .replace("$LINKS", links_block)
        .replace("$DATE", timestamp)
    )


def main():
    parser = argparse.ArgumentParser(description="Generate index.md for KiBot outputs")
    parser.add_argument(
        "-c", "--config", type=Path, help="Path to config.kibot.site.yml"
    )
    parser.add_argument("--project", type=str, help="Override project name")
    parser.add_argument(
        "--artifacts", type=str, help="Comma-separated list of artifacts"
    )
    parser.add_argument(
        "--template", type=Path, default=Path("docs/kibot/index_template.md")
    )
    parser.add_argument("--out-md", type=Path, default=Path("docs/kibot/index.md"))
    args = parser.parse_args()

    project = args.project
    artifacts: list[str] = []

    if args.artifacts:
        artifacts = [a.strip() for a in args.artifacts.split(",") if a.strip()]
    elif args.config:
        cfg = load_config(args.config)
        project = project or cfg.get("project_name", "")
        artifacts = cfg.get("artifacts", []) or []
    else:
        print(
            "❌ Either --config or both --project and --artifacts must be provided.",
            file=sys.stderr,
        )
        sys.exit(1)

    timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d at %H:%M:%S UTC")
    template_str = args.template.read_text(encoding="utf-8")
    index_md = render_template(template_str, project or "", artifacts, timestamp)

    args.out_md.parent.mkdir(parents=True, exist_ok=True)
    args.out_md.write_text(index_md.rstrip() + "\n", encoding="utf-8")
    print(f"✔ Wrote {args.out_md}")


if __name__ == "__main__":
    main()
