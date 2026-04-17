# agents/

Agent and AI assistant configuration for CASMcode_global. These files are designed to be shared across the CASM developer team via symlinks rather than tool-specific config committed directly to each repo.

## Files

| File | Purpose |
|------|---------|
| `AGENTS.md` | Repo-specific dev workflow for CASMcode_global |
| `CASM_overview.md` | CASM suite overview: package list, structure, general notes |
| `skills/casm-version/` | On-demand skill: versioning checklist |
| `skills/casm-release/` | On-demand skill: release procedure |

Release and workflow scripts live in `../dev/` (relative to this file):

| Script | Purpose |
|--------|---------|
| `dev/release.py` | Full release workflow (run from package root as `python ../CASMcode_global/dev/release.py`) |
| `dev/download_release.py` | Download build artifacts from GitHub Actions |
| `dev/update_workflow_versions.py` | Update CASM dependency versions in `.github/workflows/*.yml` |

## Symlink setup (Claude Code example)

Claude Code loads context from files named `CLAUDE.md` (repo-level) and `CLAUDE.local.md` (parent directory). Skills are loaded from `~/.claude/skills/`.

From the repo root:

```bash
# Repo-level context
ln -sf agents/AGENTS.md CLAUDE.md
```

From the parent directory (`CASMcode_global/..`):

```bash
# Suite-level context (symlink once; covers all CASM repos in this directory)
ln -sf CASMcode_global/agents/CASM_overview.md CLAUDE.local.md
```

For skills (invoke with `/casm-version` and `/casm-release`):

```bash
mkdir -p ~/.claude/skills
ln -sf "$(pwd)/agents/skills/casm-version" ~/.claude/skills/casm-version
ln -sf "$(pwd)/agents/skills/casm-release" ~/.claude/skills/casm-release
```

## Notes

- `AGENTS.md` is also read by some other agents (e.g. OpenAI Codex, Google Jules) — symlink as `AGENTS.md` at the repo root if needed.
- Other CASM repos should maintain their own `agents/AGENTS.md` with the same structure.
- `CASM_overview.md` and the skills live here in `CASMcode_global` as the canonical source; other repos symlink to them.
