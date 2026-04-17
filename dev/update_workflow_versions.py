#!/usr/bin/env python3
"""Update CASM dependency versions in GitHub workflow files.

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

Usage:
    python update_workflow_versions.py --libcasm-global 2.4.1 --libcasm-xtal 2.3.0
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


def version_to_cache_key(version):
    """Convert '2.4.1' -> 'v2-4-1'."""
    return "v" + version.replace(".", "-")


def update_file(path, updates):
    """Apply version updates to one workflow file.

    updates: dict mapping pip package name -> new version string

    Returns True if the file was changed.
    """
    original = content = path.read_text()

    for pkg, new_ver in updates.items():
        repo_path = PACKAGES[pkg]
        new_key = version_to_cache_key(new_ver)

        # 1. Cache keys: libcasm-global-v2-2-0 -> libcasm-global-v2-4-1
        content = re.sub(
            rf"{re.escape(pkg)}-v\d+-\d+-\d+",
            f"{pkg}-{new_key}",
            content,
        )

        # 2. Checkout refs: use "path: CASMcode_xxx" as context (unique per package)
        #      path: CASMcode_global
        #      ref: v2.2.0
        content = re.sub(
            rf"(path: {re.escape(repo_path)}\n\s+ref: )v[\d.]+",
            rf"\g<1>v{new_ver}",
            content,
        )

        # 3. Pip install versions: libcasm-global==2.2.0 -> libcasm-global==2.4.1
        content = re.sub(
            rf"{re.escape(pkg)}==[\d.]+",
            f"{pkg}=={new_ver}",
            content,
        )

    if content != original:
        path.write_text(content)
        return True
    return False


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
        if args.dry_run:
            # Simulate update to check if it would change
            content = original
            for pkg, new_ver in updates.items():
                repo_path = PACKAGES[pkg]
                new_key = version_to_cache_key(new_ver)
                content = re.sub(
                    rf"{re.escape(pkg)}-v\d+-\d+-\d+", f"{pkg}-{new_key}", content
                )
                content = re.sub(
                    rf"(path: {re.escape(repo_path)}\n\s+ref: )v[\d.]+",
                    rf"\g<1>v{new_ver}",
                    content,
                )
                content = re.sub(
                    rf"{re.escape(pkg)}==[\d.]+", f"{pkg}=={new_ver}", content
                )
            if content != original:
                changed.append(yml)
        else:
            if update_file(yml, updates):
                changed.append(yml)

    if changed:
        label = "Would update" if args.dry_run else "Updated"
        for f in changed:
            print(f"{label}: {f}")
    else:
        print("No changes needed.")


if __name__ == "__main__":
    main()
