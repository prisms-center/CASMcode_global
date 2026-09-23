#!/usr/bin/env python3
r"""Update CASM dependency versions in GitHub workflow files.

Handles three patterns in .github/workflows/*.yml files:

  1. Cache keys (all build/test workflows):
       key: ${{ runner.os }}-libcasm-global-v2-2-0
       ->   ${{ runner.os }}-libcasm-global-v2-4-1

  2. Checkout refs (dependency build workflows):
       path: CASMcode_global
       ref: v2.2.0
       ->   ref: v2.4.1

  3. Pip install versions (pip-install workflow):
       pip install libcasm-global==2.2.0
       ->           libcasm-global==2.4.1

Release (2.4.1) and pre-release (3.0a1, 3.0.0b2, 3.0rc1) versions are
supported, both as the current and as the new version.

Usage (from the package repository root):
    python ../CASMcode_global/dev/update_workflow_versions.py \
        --libcasm-global 2.4.1 --libcasm-xtal 3.0a1
"""

import argparse
import re
import sys
from pathlib import Path

# Maps pip package name -> checkout path used in the workflow files
PACKAGES = {
    "libcasm-global": "CASMcode_global",
    "libcasm-xtal": "CASMcode_crystallography",
    "libcasm-composition": "CASMcode_composition",
    "libcasm-mapping": "CASMcode_mapping",
    "libcasm-clexulator": "CASMcode_clexulator",
    "libcasm-configuration": "CASMcode_configuration",
    "libcasm-monte": "CASMcode_monte",
}


# Release or pre-release version, e.g. 2.4.1, 3.0a1, 3.0.0b2, 3.0rc1
VERSION = r"\d+(?:\.\d+)*(?:(?:a|b|rc)\d+)?"

# Cache key form of VERSION, e.g. v2-4-1, v3-0a1
CACHE_KEY = r"v\d+(?:-\d+)*(?:(?:a|b|rc)\d+)?"


def version_to_cache_key(version):
    """Convert '2.4.1' -> 'v2-4-1', '3.0a1' -> 'v3-0a1'."""
    return "v" + version.replace(".", "-")


def apply_updates(content, updates):
    """Return workflow file content with version updates applied.

    updates: dict mapping pip package name -> new version string
    """
    for pkg, new_ver in updates.items():
        repo_path = PACKAGES[pkg]
        new_key = version_to_cache_key(new_ver)

        # 1. Cache keys: libcasm-global-v2-2-0 -> libcasm-global-v2-4-1
        content = re.sub(
            rf"{re.escape(pkg)}-{CACHE_KEY}\b",
            f"{pkg}-{new_key}",
            content,
        )

        # 2. Checkout refs: use "path: CASMcode_xxx" as context (unique per package)
        #      path: CASMcode_global
        #      ref: v2.2.0
        content = re.sub(
            rf"(path: {re.escape(repo_path)}\n\s+ref: )v{VERSION}\b",
            rf"\g<1>v{new_ver}",
            content,
        )

        # 3. Pip install versions: libcasm-global==2.2.0 -> libcasm-global==2.4.1
        content = re.sub(
            rf"{re.escape(pkg)}=={VERSION}\b",
            f"{pkg}=={new_ver}",
            content,
        )

    return content


def main():
    parser = argparse.ArgumentParser(
        description="Update CASM dependency versions in .github/workflows/*.yml files.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    for pkg in PACKAGES:
        parser.add_argument(
            f"--{pkg}",
            metavar="VERSION",
            help=f"New version for {pkg} (e.g. 2.4.1)",
        )
    parser.add_argument(
        "--workflows-dir",
        default=".github/workflows",
        metavar="DIR",
        help="Workflows directory (default: .github/workflows)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show which files would change without modifying them",
    )
    args = parser.parse_args()

    updates = {}
    for pkg in PACKAGES:
        ver = getattr(args, pkg.replace("-", "_"))
        if ver:
            if not re.fullmatch(VERSION, ver):
                parser.error(
                    f"invalid version for --{pkg}: {ver!r} "
                    "(expected e.g. 2.4.1 or 3.0a1, without a leading 'v')"
                )
            updates[pkg] = ver

    if not updates:
        parser.print_help()
        sys.exit(1)

    workflows_dir = Path(args.workflows_dir)
    if not workflows_dir.is_dir():
        print(f"Error: {workflows_dir} not found", file=sys.stderr)
        sys.exit(1)

    changed = []
    for yml in sorted(workflows_dir.glob("*.yml")):
        original = yml.read_text()
        content = apply_updates(original, updates)
        if content != original:
            if not args.dry_run:
                yml.write_text(content)
            changed.append(yml)

    if changed:
        label = "Would update" if args.dry_run else "Updated"
        for f in changed:
            print(f"{label}: {f}")
    else:
        print("No changes needed.")


if __name__ == "__main__":
    main()
