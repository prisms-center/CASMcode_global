#!/usr/bin/env python3
"""
CASM release workflow script.

Run from the root directory of the package repository after all changes are
committed on the version branch and all GitHub Actions workflows have passed.

Usage:
    python ../release.py <version> --dev-branch <2.X|3.X> [options]

Steps:
    1 - Download artifacts, label wheels (libcasm- only), upload to PyPI
    2 - Install from PyPI and run tests
    3 - Create and push version tag
    4 - Create GitHub release
    5 - Merge tag into main, then main into dev-branch
    6 - Build and push documentation

Examples:
    python ../release.py 2.4.0 --dev-branch 2.X --pydocs ../CASMcode_pydocs
    python ../release.py 2.4.0 --dev-branch 2.X --pydocs ../CASMcode_pydocs --start-step 3
    python ../release.py 2.4.0 --dev-branch 2.X --pydocs ../CASMcode_pydocs --steps 1,2
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import textwrap

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


# ---------------------------------------------------------------------------
# Utility functions
# ---------------------------------------------------------------------------


def run(cmd, cwd=None):
    """Run a command, return (returncode, stdout, stderr)."""
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd)
    return result.returncode, result.stdout.strip(), result.stderr.strip()


def run_interactive(cmd, cwd=None):
    """Run a command interactively (stdout/stderr go to terminal)."""
    result = subprocess.run(cmd, cwd=cwd)
    return result.returncode


def confirm(msg, default=True):
    """Prompt for yes/no confirmation. Returns True if confirmed."""
    suffix = " [Y/n]: " if default else " [y/N]: "
    try:
        response = input(msg + suffix).strip().lower()
    except (EOFError, KeyboardInterrupt):
        print()
        abort("Interrupted.")
    if not response:
        return default
    return response in ("y", "yes")


def abort(msg=""):
    if msg:
        print(f"\nAborted: {msg}", file=sys.stderr)
    else:
        print("\nAborted.", file=sys.stderr)
    sys.exit(1)


def header(title):
    bar = "=" * 60
    print(f"\n{bar}")
    print(title)
    print(bar)


def get_package_name():
    """Read the package name from pyproject.toml in the current directory."""
    if not os.path.exists("pyproject.toml"):
        print("Error: pyproject.toml not found. Run from the package root directory.")
        sys.exit(1)
    with open("pyproject.toml") as f:
        content = f.read()
    # Find name under [project]
    m = re.search(
        r"^\[project\].*?^name\s*=\s*\"([^\"]+)\"",
        content,
        re.MULTILINE | re.DOTALL,
    )
    if not m:
        print("Error: Could not find package name in pyproject.toml.")
        sys.exit(1)
    return m.group(1)


def get_changelog_entry(version):
    """Extract the CHANGELOG.md section for the given version."""
    if not os.path.exists("CHANGELOG.md"):
        print("Error: CHANGELOG.md not found.")
        sys.exit(1)
    with open("CHANGELOG.md") as f:
        content = f.read()
    pattern = rf"(## \[{re.escape(version)}\].*?)(?=\n## \[|\Z)"
    m = re.search(pattern, content, re.DOTALL)
    if not m:
        print(f"Error: No CHANGELOG.md entry found for version {version}.")
        sys.exit(1)
    return m.group(1).strip()


def is_libcasm_package(package_name):
    return package_name.startswith("libcasm-")


def short_name(package_name):
    """Return the short name used in docs paths (strip libcasm- or casm- prefix)."""
    for prefix in ("libcasm-", "casm-"):
        if package_name.startswith(prefix):
            return package_name[len(prefix):]
    return package_name


def docs_namespace(package_name):
    """Return 'libcasm' or 'casm' for the pydocs namespace."""
    return "libcasm" if is_libcasm_package(package_name) else "casm"


def docs_version_folder(dev_branch):
    """Return the docs subdirectory name for the given dev branch.

    '2.X' -> '2.0'  (legacy layout)
    '3.X' -> '3'
    '4.X' -> '4'  etc.
    """
    m = re.match(r"^(\d+)\.X$", dev_branch)
    if m:
        major = int(m.group(1))
        return "2.0" if major == 2 else str(major)
    return "2.0"


# ---------------------------------------------------------------------------
# Git helpers
# ---------------------------------------------------------------------------


def check_git_branch(version):
    """Warn if not on the version branch."""
    rc, branch, _ = run(["git", "branch", "--show-current"])
    if rc != 0:
        abort("Failed to get current git branch.")
    if branch != version:
        print(f"Warning: current branch is '{branch}', expected '{version}'.")
        if not confirm("Continue anyway?", default=False):
            abort()
    return branch


def check_git_clean():
    """Warn if there are uncommitted changes."""
    rc, out, _ = run(["git", "status", "--porcelain"])
    if out:
        print("Warning: there are uncommitted changes:")
        print(out)
        if not confirm("Continue anyway?", default=False):
            abort()


# ---------------------------------------------------------------------------
# Step 1: Download artifacts and upload to PyPI
# ---------------------------------------------------------------------------


def _download_and_label_libcasm(version):
    """Use download_release.py + label_wheels.py for C++ extension packages."""
    download_script = os.path.join(SCRIPT_DIR, "download_release.py")
    if not os.path.exists(download_script):
        abort(f"download_release.py not found at: {download_script}")

    print(f"\nRunning: python {download_script} {version}")
    rc = run_interactive([sys.executable, download_script, version])
    if rc != 0:
        abort("download_release.py failed.")

    label_script = "label_wheels.py"
    if not os.path.exists(label_script):
        abort(f"label_wheels.py not found in current directory.")

    print(f"\nRunning: python {label_script} {version}")
    rc = run_interactive([sys.executable, label_script, version])
    if rc != 0:
        abort("label_wheels.py failed.")


def _download_pure_python(version):
    """
    For pure Python packages: verify all workflows pass and download artifacts
    from the 'Build' workflow. No wheel relabeling needed.
    """
    branch = version
    print(f"\nChecking workflow runs on branch '{branch}'...")
    rc, out, err = run(
        [
            "gh",
            "run",
            "list",
            "--branch",
            branch,
            "--json",
            "workflowName,status,conclusion,databaseId",
            "--limit",
            "50",
        ]
    )
    if rc != 0:
        print(err)
        abort("gh run list failed.")

    runs = json.loads(out)
    if not runs:
        abort(f"No workflow runs found on branch '{branch}'.")

    # Most recent run per workflow
    latest = {}
    for r in runs:
        wf = r["workflowName"]
        if wf not in latest:
            latest[wf] = r

    all_ok = True
    build_run_id = None
    for wf, r in latest.items():
        ok = r["status"] == "completed" and r["conclusion"] == "success"
        mark = "OK" if ok else "FAIL"
        print(f"  [{mark}] {wf}: {r['status']} / {r['conclusion']}")
        if not ok:
            all_ok = False
        if wf == "Build":
            build_run_id = r["databaseId"]

    if not all_ok:
        abort("Not all workflows passed.")
    if build_run_id is None:
        abort("Could not find a 'Build' workflow run.")

    print("\nAll workflows passed.")

    dist_dir = f"dist/{version}"
    if os.path.exists(dist_dir):
        abort(f"'{dist_dir}' already exists. Remove it and try again.")

    print(f"\nDownloading artifacts from 'Build' run {build_run_id}...")
    rc = run_interactive(
        ["gh", "run", "download", str(build_run_id), "--name", "dist", "--dir", dist_dir]
    )
    if rc != 0:
        abort("Failed to download artifacts.")

    print(f"\nArtifacts written to '{dist_dir}/'.")


def step_download_and_upload(version, package_name):
    """Step 1: Download artifacts, label wheels if needed, upload to PyPI."""
    header("Step 1: Download artifacts and upload to PyPI")

    if is_libcasm_package(package_name):
        _download_and_label_libcasm(version)
    else:
        _download_pure_python(version)

    dist_dir = f"dist/{version}"
    print(f"\nReady to upload contents of {dist_dir}/:")
    for f in sorted(os.listdir(dist_dir)):
        print(f"  {f}")

    if not confirm("\nUpload to PyPI with twine?"):
        abort("Upload cancelled.")

    rc = run_interactive(
        [sys.executable, "-m", "twine", "upload", f"{dist_dir}/*"]
    )
    if rc != 0:
        # twine may need shell glob expansion
        import glob
        files = glob.glob(f"{dist_dir}/*")
        rc = run_interactive([sys.executable, "-m", "twine", "upload"] + files)
        if rc != 0:
            abort("twine upload failed.")

    print("\nUpload complete.")


# ---------------------------------------------------------------------------
# Step 2: Install from PyPI and run tests
# ---------------------------------------------------------------------------


def step_install_and_test(version, package_name):
    """Step 2: Install from PyPI and run tests."""
    header("Step 2: Install from PyPI and run tests")

    if not confirm(f"\nInstall {package_name}=={version} from PyPI and run tests?"):
        print("Skipping.")
        return

    print(f"\nRunning: pip install --upgrade {package_name}=={version}")
    rc = run_interactive(
        [sys.executable, "-m", "pip", "install", "--upgrade", f"{package_name}=={version}"]
    )
    if rc != 0:
        if not confirm("pip install failed. Continue anyway?", default=False):
            abort("pip install failed.")

    test_dir = "python/tests" if is_libcasm_package(package_name) else "tests"
    print(f"\nRunning: pytest -rsap -x {test_dir}")
    rc = run_interactive(["pytest", "-rsap", "-x", test_dir])
    if rc != 0:
        if not confirm("Tests failed. Continue anyway?", default=False):
            abort("Tests failed.")


# ---------------------------------------------------------------------------
# Step 3: Create and push version tag
# ---------------------------------------------------------------------------


def step_tag_and_push(version, remote):
    """Step 3: Create and push version tag."""
    header("Step 3: Create and push version tag")

    tag = f"v{version}"
    print(f"\nWill create tag '{tag}' and push to remote '{remote}'.")

    if not confirm("Proceed?"):
        abort()

    rc, _, err = run(["git", "tag", tag])
    if rc != 0:
        print(f"  {err}")
        abort(f"Failed to create tag '{tag}'.")

    print(f"Pushing {tag} to {remote}...")
    rc = run_interactive(["git", "push", remote, tag])
    if rc != 0:
        abort(f"Failed to push tag '{tag}' to '{remote}'.")

    print(f"\nTag {tag} created and pushed.")


# ---------------------------------------------------------------------------
# Step 4: Create GitHub release
# ---------------------------------------------------------------------------


def _find_tarball(version):
    """Return path to the source tarball in dist/<version>/, or None."""
    import glob

    patterns = [
        f"dist/{version}/*.tar.gz",
    ]
    for pat in patterns:
        matches = glob.glob(pat)
        if matches:
            return matches[0]
    return None


def step_create_release(version, package_name, changelog_entry):
    """Step 4: Create GitHub release."""
    header("Step 4: Create GitHub release")

    tag = f"v{version}"
    tarball = _find_tarball(version)

    print(f"\nRelease tag: {tag}")
    if tarball:
        print(f"Tarball: {tarball}")
    else:
        print("Tarball: not found (will create release without attachment)")
    print(f"\nRelease notes:\n{changelog_entry}\n")

    if not confirm("Create GitHub release?"):
        abort()

    cmd = [
        "gh",
        "release",
        "create",
        tag,
        "--title",
        tag,
        "--notes",
        changelog_entry,
    ]
    if tarball:
        cmd.append(tarball)

    rc = run_interactive(cmd)
    if rc != 0:
        abort("Failed to create GitHub release.")

    print("\nGitHub release created.")


# ---------------------------------------------------------------------------
# Step 5: Merge branches
# ---------------------------------------------------------------------------


def step_merge_branches(version, dev_branch, remote):
    """Step 5: Merge tag into main, then main into dev-branch."""
    header("Step 5: Merge branches")

    tag = f"v{version}"
    print(f"\nPlan:")
    print(f"  1. git checkout main && git pull")
    print(f"  2. git merge --ff-only {tag}")
    print(f"  3. git push {remote} main")
    print(f"  4. git checkout {dev_branch} && git pull {remote} {dev_branch}")
    print(f"  5. git merge main --no-edit")
    print(f"  6. git push {remote} {dev_branch}")

    if not confirm("\nProceed?"):
        abort()

    print("\nFetching all remotes and tags...")
    run_interactive(["git", "fetch", "--all", "--tags"])

    # Show state before merging
    for ref in [f"{remote}/main", f"{remote}/{dev_branch}", tag]:
        rc, out, _ = run(["git", "log", "--oneline", "-3", ref])
        if rc == 0:
            print(f"\n  {ref}:")
            for line in out.splitlines():
                print(f"    {line}")

    if not confirm("\nLooks good? Proceed with merge?"):
        abort()

    # Merge tag into main
    print(f"\nChecking out main...")
    rc = run_interactive(["git", "checkout", "main"])
    if rc != 0:
        abort("Failed to checkout main.")

    rc = run_interactive(["git", "pull"])
    if rc != 0:
        print("Warning: git pull failed on main (continuing).")

    print(f"Merging {tag} into main (fast-forward)...")
    rc = run_interactive(["git", "merge", "--ff-only", tag])
    if rc != 0:
        abort(f"Fast-forward merge of {tag} into main failed.")

    print(f"Pushing main to {remote}...")
    rc = run_interactive(["git", "push", remote, "main"])
    if rc != 0:
        abort(f"Failed to push main to {remote}.")

    # Merge main into dev branch
    print(f"\nChecking out {dev_branch}...")
    rc = run_interactive(["git", "checkout", dev_branch])
    if rc != 0:
        abort(f"Failed to checkout {dev_branch}.")

    rc = run_interactive(["git", "pull", remote, dev_branch])
    if rc != 0:
        print(f"Warning: git pull failed on {dev_branch} (continuing).")

    print(f"Merging main into {dev_branch}...")
    rc = run_interactive(["git", "merge", "main", "--no-edit"])
    if rc != 0:
        abort(f"Failed to merge main into {dev_branch}.")

    print(f"Pushing {dev_branch} to {remote}...")
    rc = run_interactive(["git", "push", remote, dev_branch])
    if rc != 0:
        abort(f"Failed to push {dev_branch} to {remote}.")

    print(f"\nBranches merged and pushed.")


# ---------------------------------------------------------------------------
# Step 6: Build and push documentation
# ---------------------------------------------------------------------------


def _update_overview_rst(rst_path, package_name, version, docs_folder):
    """
    Update the version number for package_name in the overview RST file.
    Returns True if a replacement was made.
    """
    ns = docs_namespace(package_name)
    sn = short_name(package_name)
    link_path = f"../../{ns}/{sn}/{docs_folder}/"

    with open(rst_path) as f:
        content = f.read()

    # Pattern: `[OLD_VERSION] <../../libcasm/clexmonte/2.0/>`_
    pattern = rf"(`\[)[^\]]+(] <{re.escape(link_path)}>`_)"
    replacement = rf"\g<1>{version}\g<2>"
    new_content, count = re.subn(pattern, replacement, content)

    if count == 0:
        return False

    with open(rst_path, "w") as f:
        f.write(new_content)
    return True


def step_build_docs(version, package_name, pydocs_path, dev_branch):
    """Step 6: Build and push documentation."""
    header("Step 6: Build and push documentation")

    ns = docs_namespace(package_name)
    sn = short_name(package_name)
    docs_folder = docs_version_folder(dev_branch)

    if is_libcasm_package(package_name):
        doc_src = "python/doc"
        autosummary_dir = "python/doc/reference/libcasm/_autosummary"
    else:
        doc_src = "doc"
        autosummary_dir = "doc/reference/casm/_autosummary"

    docs_output = os.path.join(pydocs_path, "docs", ns, sn, docs_folder)
    docs_git_rel = os.path.join("docs", ns, sn, docs_folder)
    overview_rst = os.path.join(pydocs_path, "src", "overview", "latest", "index.rst")
    overview_src = os.path.join(pydocs_path, "src", "overview", "latest")
    overview_dst = os.path.join(pydocs_path, "docs", "overview", "latest")
    overview_dst_rel = os.path.join("docs", "overview", "latest")
    overview_rst_rel = os.path.join("src", "overview", "latest", "index.rst")

    print(f"\nPackage docs source : {doc_src}")
    print(f"Docs output         : {docs_output}")
    print(f"Overview RST        : {overview_rst}")

    if not confirm("\nBuild documentation?"):
        print("Skipping.")
        return

    # Remove stale autosummary files
    if autosummary_dir and os.path.exists(autosummary_dir):
        print(f"\nRemoving stale autosummary: {autosummary_dir}")
        shutil.rmtree(autosummary_dir)

    # Build package docs
    print(f"\nRunning: sphinx-build -b html {doc_src} {docs_output}")
    rc = run_interactive(["sphinx-build", "-b", "html", doc_src, docs_output])
    if rc != 0:
        if not confirm("sphinx-build failed. Continue anyway?", default=False):
            abort("sphinx-build failed.")

    # Update version in overview RST
    print(f"\nUpdating version to [{version}] in overview RST...")
    updated = _update_overview_rst(overview_rst, package_name, version, docs_folder)
    if updated:
        print(f"  Updated {overview_rst}")
    else:
        print(f"  Warning: could not auto-update version in overview RST.")
        print(f"  Please manually update {overview_rst}")
        print(f"  (set version for {package_name} to [{version}])")
        input("  Press Enter when done... ")

    # Rebuild overview docs
    print(f"\nRunning: sphinx-build -b html {overview_src} {overview_dst}")
    rc = run_interactive(
        ["sphinx-build", "-b", "html", overview_src, overview_dst],
        cwd=pydocs_path,
    )
    if rc != 0:
        if not confirm("Overview sphinx-build failed. Continue?", default=False):
            abort("Overview sphinx-build failed.")

    # Commit and push
    commit_msg = f"{package_name} v{version}"
    print(f"\nWill commit in {pydocs_path} with message: '{commit_msg}'")
    print(f"  git add {docs_git_rel} {overview_dst_rel} {overview_rst_rel}")

    if not confirm("Commit and push docs?"):
        print("Skipping docs commit/push.")
        return

    rc = run_interactive(
        ["git", "add", docs_git_rel, overview_dst_rel, overview_rst_rel],
        cwd=pydocs_path,
    )
    if rc != 0:
        abort("git add failed in pydocs repository.")

    rc = run_interactive(["git", "commit", "-m", commit_msg], cwd=pydocs_path)
    if rc != 0:
        abort("git commit failed in pydocs repository.")

    rc = run_interactive(["git", "push"], cwd=pydocs_path)
    if rc != 0:
        abort("git push failed in pydocs repository.")

    print("\nDocumentation built and pushed.")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def parse_steps(s):
    """Parse a steps string like '1,3,5-6' into a set of ints."""
    steps = set()
    for part in s.split(","):
        part = part.strip()
        if "-" in part:
            lo, hi = part.split("-", 1)
            steps.update(range(int(lo), int(hi) + 1))
        else:
            steps.add(int(part))
    return steps


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("version", help="Version to release (e.g. 2.4.0)")
    parser.add_argument(
        "--dev-branch",
        required=True,
        metavar="BRANCH",
        help="Development branch to merge into after release (e.g. '2.X' or '3.X')",
    )
    parser.add_argument(
        "--pydocs",
        metavar="PATH",
        default=None,
        help="Path to CASMcode_pydocs repository (required for step 6)",
    )
    parser.add_argument(
        "--remote",
        default="public",
        metavar="REMOTE",
        help="Git remote to push to (default: 'public')",
    )
    parser.add_argument(
        "--package",
        metavar="NAME",
        default=None,
        help="Package name (auto-detected from pyproject.toml if not given)",
    )
    parser.add_argument(
        "--start-step",
        type=int,
        default=1,
        metavar="N",
        help="Start from step N (default: 1)",
    )
    parser.add_argument(
        "--end-step",
        type=int,
        default=6,
        metavar="N",
        help="Stop after step N (default: 6)",
    )
    parser.add_argument(
        "--steps",
        default=None,
        metavar="LIST",
        help=(
            "Comma-separated list or range of steps to run, e.g. '1,2' or '3-5'."
            " Overrides --start-step/--end-step."
        ),
    )

    args = parser.parse_args()

    # Resolve which steps to run
    if args.steps:
        try:
            steps = parse_steps(args.steps)
        except ValueError:
            parser.error(f"Invalid --steps value: {args.steps!r}")
    else:
        steps = set(range(args.start_step, args.end_step + 1))

    # Auto-detect package name
    package_name = args.package or get_package_name()

    print(f"\nPackage  : {package_name}")
    print(f"Version  : {args.version}")
    print(f"Dev branch: {args.dev_branch}")
    print(f"Remote   : {args.remote}")
    print(f"Steps    : {sorted(steps)}")
    if args.pydocs:
        print(f"Pydocs   : {os.path.abspath(args.pydocs)}")

    # Pre-flight checks (only if starting from step 1 or 2)
    if steps & {1, 2}:
        check_git_branch(args.version)
        check_git_clean()

    # Extract changelog entry (needed for step 4)
    changelog_entry = get_changelog_entry(args.version)

    pydocs_path = os.path.abspath(args.pydocs) if args.pydocs else None

    # Run steps
    if 1 in steps:
        step_download_and_upload(args.version, package_name)

    if 2 in steps:
        step_install_and_test(args.version, package_name)

    if 3 in steps:
        step_tag_and_push(args.version, args.remote)

    if 4 in steps:
        step_create_release(args.version, package_name, changelog_entry)

    if 5 in steps:
        step_merge_branches(args.version, args.dev_branch, args.remote)

    if 6 in steps:
        if not pydocs_path:
            print(
                "\nStep 6 skipped: --pydocs not specified.",
                "(Re-run with --steps 6 --pydocs <path> to build docs.)",
            )
        else:
            step_build_docs(args.version, package_name, pydocs_path, args.dev_branch)

    header("Release complete!")


if __name__ == "__main__":
    main()
