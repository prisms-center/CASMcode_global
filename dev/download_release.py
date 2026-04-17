"""
Check that all GitHub Actions workflows completed successfully on the release
branch, then download build_wheels.yml artifacts into dist/<version>_raw/.

Run from within the package repository directory.

Usage:
    python ../download_release.py <version>

Example:
    python ../download_release.py 2.3.1

After running, continue with:
    python label_wheels.py <version>
    python -m twine upload dist/<version>/*
"""

import json
import os
import shutil
import subprocess
import sys


def gh(*args):
    cmd = ["gh", *args]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running: {' '.join(cmd)}")
        print(result.stderr)
        sys.exit(1)
    return result.stdout


def main():
    if len(sys.argv) != 2:
        print("Usage: python ../download_release.py <version>")
        sys.exit(1)

    version = sys.argv[1]
    branch = version  # release branch has the same name as the version

    # 1. Check all workflow runs on the release branch
    print(f"Checking workflow runs on branch '{branch}'...")
    output = gh(
        "run", "list",
        "--branch", branch,
        "--json", "workflowName,status,conclusion,databaseId",
        "--limit", "50",
    )
    runs = json.loads(output)

    if not runs:
        print(f"No workflow runs found on branch '{branch}'.")
        sys.exit(1)

    # Take the most recent run per workflow (list is ordered newest-first)
    latest = {}
    for r in runs:
        wf = r["workflowName"]
        if wf not in latest:
            latest[wf] = r

    all_ok = True
    build_wheels_run_id = None
    for wf, r in latest.items():
        ok = r["status"] == "completed" and r["conclusion"] == "success"
        mark = "OK" if ok else "FAIL"
        print(f"  [{mark}] {wf}: {r['status']} / {r['conclusion']}")
        if not ok:
            all_ok = False
        if wf == "Build wheels":
            build_wheels_run_id = r["databaseId"]

    if not all_ok:
        print("\nNot all workflows passed. Aborting.")
        sys.exit(1)

    if build_wheels_run_id is None:
        print("\nCould not find a 'Build wheels' workflow run. Aborting.")
        sys.exit(1)

    print(f"\nAll workflows passed.")

    # 2. Download artifacts into a temporary directory, then flatten
    raw_dir = f"dist/{version}_raw"
    if os.path.exists(raw_dir):
        print(f"Error: '{raw_dir}' already exists. Remove it and try again.")
        sys.exit(1)

    tmp_dir = f"dist/{version}_raw_tmp"
    if os.path.exists(tmp_dir):
        shutil.rmtree(tmp_dir)
    os.makedirs(tmp_dir)

    print(f"Downloading artifacts from 'Build wheels' run {build_wheels_run_id}...")
    gh("run", "download", str(build_wheels_run_id), "--dir", tmp_dir)

    os.makedirs(raw_dir)
    for artifact_name in sorted(os.listdir(tmp_dir)):
        artifact_path = os.path.join(tmp_dir, artifact_name)
        if not os.path.isdir(artifact_path):
            continue
        for filename in sorted(os.listdir(artifact_path)):
            src = os.path.join(artifact_path, filename)
            dst = os.path.join(raw_dir, filename)
            shutil.move(src, dst)
            print(f"  {filename}")

    shutil.rmtree(tmp_dir)
    print(f"\nArtifacts written to '{raw_dir}/'.")
    print(f"\nNext steps:")
    print(f"  python label_wheels.py {version}")
    print(f"  python -m twine upload dist/{version}/*")


if __name__ == "__main__":
    main()
