#!/usr/bin/python
##################################################################################
#                                                                                #
## Generates keira_firmware.json manifest for GitHub releases.                  ##
## Used by firmware workflow and by Keira OTA updater on device.                ##
#                                                                                #
##################################################################################
#
# Manifest layout:
# {
#   "name": "KeiraOS",
#   "repo": "lilka-dev/keira",
#   ...fields of current version entry...,
#   "versions": [ <current entry>, <up to N previous entries> ]
# }
#
# Version entry:
# {
#   "version": "2.6.12", "tag": "v2.6.12", "prerelease": false, "date": "...",
#   "languages": ["LANG_UK", "LANG_EN"],
#   "firmware": {"LANG_UK": {"url": "...", "size": 123, "sha256": "..."}, ...},
#   "merged":   {"LANG_UK": {"url": "...", "size": 123, "sha256": "..."}, ...}
# }
#
# Info about previous versions is taken from their own keira_firmware.json if it
# exists, otherwise it is generated from release assets list.

import argparse, hashlib, json, os, re, sys, urllib.request
from datetime import datetime, timezone
from pathlib import Path

MANIFEST_NAME = "keira_firmware.json"
FIRMWARE_RE = re.compile(r"^keira_(LANG_[A-Z]+)\.bin$")
MERGED_RE = re.compile(r"^KeiraOS_merged_(LANG_[A-Z]+)\.bin$")


def api_get(url, token):
    req = urllib.request.Request(url)
    req.add_header("Accept", "application/vnd.github+json")
    if token:
        req.add_header("Authorization", f"Bearer {token}")
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.loads(resp.read().decode("utf-8"))


def download_url(repo, tag, name):
    return f"https://github.com/{repo}/releases/download/{tag}/{name}"


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def make_entry(repo, tag, prerelease, date, files):
    # files: list of (name, size, sha256 or None)
    entry = {
        "version": tag.lstrip("v"),
        "tag": tag,
        "prerelease": prerelease,
        "date": date,
        "languages": [],
        "firmware": {},
        "merged": {},
    }
    for name, size, sha in files:
        for regex, key in ((FIRMWARE_RE, "firmware"), (MERGED_RE, "merged")):
            m = regex.match(name)
            if not m:
                continue
            info = {"url": download_url(repo, tag, name), "size": size}
            if sha:
                info["sha256"] = sha
            entry[key][m.group(1)] = info
    entry["languages"] = sorted(entry["firmware"].keys())
    return entry


def previous_entries(repo, current_tag, count, token):
    releases = api_get(f"https://api.github.com/repos/{repo}/releases?per_page=30", token)
    releases = [r for r in releases if not r["draft"] and r["tag_name"] != current_tag]
    releases.sort(key=lambda r: r.get("published_at") or "", reverse=True)

    entries = []
    for rel in releases:
        if len(entries) >= count:
            break
        tag = rel["tag_name"]
        assets = rel.get("assets", [])

        entry = None
        manifest = next((a for a in assets if a["name"] == MANIFEST_NAME), None)
        if manifest:
            try:
                with urllib.request.urlopen(manifest["browser_download_url"], timeout=30) as resp:
                    data = json.loads(resp.read().decode("utf-8"))
                entry = {k: v for k, v in data.items() if k not in ("versions", "name", "repo")}
                print(f"{tag}: using existing {MANIFEST_NAME}")
            except Exception as e:  # noqa: BLE001
                print(f"{tag}: failed to read {MANIFEST_NAME} ({e}), generating from assets")

        if entry is None:
            files = []
            for a in assets:
                digest = a.get("digest") or ""
                sha = digest[len("sha256:"):] if digest.startswith("sha256:") else None
                files.append((a["name"], a["size"], sha))
            entry = make_entry(repo, tag, rel["prerelease"], rel.get("published_at"), files)
            print(f"{tag}: generated from release assets")

        if not entry.get("firmware"):
            print(f"{tag}: no firmware binaries found, skipping")
            continue
        entries.append(entry)
    return entries


def main():
    parser = argparse.ArgumentParser(description=f"Generate {MANIFEST_NAME}")
    parser.add_argument("--repo", required=True, help="owner/name")
    parser.add_argument("--tag", required=True, help="current release tag, e.g. v2.6.12")
    parser.add_argument("--prerelease", default="false")
    parser.add_argument("--bin-dir", default="out", help="directory with built binaries")
    parser.add_argument("--history", type=int, default=5, help="number of previous versions to include")
    parser.add_argument("--output", default=f"out/{MANIFEST_NAME}")
    args = parser.parse_args()

    token = os.environ.get("GITHUB_TOKEN", "")
    prerelease = args.prerelease.lower() == "true"
    date = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

    files = [(p.name, p.stat().st_size, sha256_file(p)) for p in sorted(Path(args.bin_dir).glob("*.bin"))]
    current = make_entry(args.repo, args.tag, prerelease, date, files)
    if not current["firmware"]:
        print(f"Error: no keira_LANG_*.bin found in {args.bin_dir}")
        return 1

    try:
        history = previous_entries(args.repo, args.tag, args.history, token)
    except Exception as e:  # noqa: BLE001
        # Manifest for the current version is still useful without history
        print(f"Warning: failed to fetch previous releases: {e}")
        history = []

    manifest = {"name": "KeiraOS", "repo": args.repo, **current, "versions": [current] + history}

    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    with open(args.output, "w") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)
    print(f"Written {args.output} with {len(manifest['versions'])} version(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
