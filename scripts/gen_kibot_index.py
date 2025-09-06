#!/usr/bin/env python3

import argparse
from datetime import datetime
from pathlib import Path
import sys
import yaml
import markdown2


def load_config(config_file: Path) -> dict:
    with config_file.open("r") as f:
        return yaml.safe_load(f)


def render_template(
    template: str, project: str, artifacts: list[str], timestamp: str
) -> str:
    lines = []
    for a in artifacts:
        a = a.strip()
        if a:
            lines.append(f"- [{a}](./{a})")
    links_block = "\n".join(lines)

    return (
        template.replace("$PROJECT_NAME", project)
        .replace("$LINKS", links_block)
        .replace("$DATE", timestamp)
    )


def main():
    parser = argparse.ArgumentParser(
        description="Generate index.md and index.html for KiBot outputs"
    )
    parser.add_argument("-c", "--config", type=Path, help="Path to site_config.yml")
    parser.add_argument("--project", type=str, help="Override project name")
    parser.add_argument(
        "--artifacts", type=str, help="Comma-separated list of artifacts"
    )
    parser.add_argument("--template", type=Path, default=Path("docs/index_template.md"))
    parser.add_argument("--out-md", type=Path, default=Path("docs/index.md"))
    parser.add_argument("--out-html", type=Path, default=Path("docs/index.html"))
    parser.add_argument("--html", action="store_true", help="Also generate index.html")
    args = parser.parse_args()

    # Load config unless everything is overridden
    project = args.project
    artifacts = []
    if args.artifacts:
        artifacts = [a.strip() for a in args.artifacts.split(",") if a.strip()]
    elif args.config:
        cfg = load_config(args.config)
        project = project or cfg["project_name"]
        artifacts = artifacts or cfg["artifacts"]
    else:
        print(
            "❌ Either --config or both --project and --artifacts must be provided.",
            file=sys.stderr,
        )
        sys.exit(1)

    # Prepare timestamp and read template
    timestamp = datetime.now().strftime("%Y-%m-%d at %H:%M:%S %Z")
    template_str = args.template.read_text(encoding="utf-8")

    # Generate and write index.md
    index_md = render_template(template_str, project, artifacts, timestamp)
    args.out_md.parent.mkdir(parents=True, exist_ok=True)
    args.out_md.write_text(index_md, encoding="utf-8")
    print(f"✔ Wrote {args.out_md}")

    if args.html:
        html = markdown2.markdown(index_md)
        args.out_html.write_text(html, encoding="utf-8")
        print(f"✔ Wrote {args.out_html}")


if __name__ == "__main__":
    main()
