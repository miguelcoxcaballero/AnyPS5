import json, urllib.request, subprocess, os
cands = json.load(open("/tmp/cands.json"))
extra = ["FT_Atan2", "FT_Vector_Rotate", "FT_Vector_Length", "FT_Vector_Polarize", "FT_Vector_From_Polar", "shader_scan", "CBRANCH_CDBG", "SCbranchCdbg", "ENDPGM_SAVED", "rdna2_isa_forms"]
keys = [c["nid"] for c in cands] + [c["twin"] for c in cands] + extra
hdr = {"Accept": "application/vnd.github+json", "User-Agent": "anyps5-overlap"}
tok = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
if tok:
    hdr["Authorization"] = "Bearer " + tok
prs = []
for page in range(1, 8):
    req = urllib.request.Request(f"https://api.github.com/repos/boykopovar/AnyPS5/pulls?state=open&per_page=100&page={page}", headers=hdr)
    try:
        data = json.load(urllib.request.urlopen(req))
    except Exception as e:
        print("api error", e); break
    prs += [(p["number"], p["title"]) for p in data]
    if len(data) < 100:
        break
print("open PRs:", len(prs))
for n in (11613,):
    try:
        p = json.load(urllib.request.urlopen(urllib.request.Request(f"https://api.github.com/repos/boykopovar/AnyPS5/pulls/{n}", headers=hdr)))
        print("PR", n, p["state"], "merged:", p.get("merged"), p["title"])
    except Exception as e:
        print(n, "err", e)
hits = {}
for num, title in prs:
    r = subprocess.run(["git", "fetch", "-q", "up", f"+refs/pull/{num}/head:refs/pr/{num}"], capture_output=True, text=True)
    if r.returncode:
        print("fetch fail", num); continue
    base = subprocess.run(["git", "merge-base", "up/main", f"refs/pr/{num}"], capture_output=True, text=True).stdout.strip()
    diff = subprocess.run(["git", "diff", base, f"refs/pr/{num}"], capture_output=True, text=True, errors="ignore").stdout
    for k in keys:
        if k in diff:
            hits.setdefault(k, []).append((num, title))
print("OVERLAP HITS:", json.dumps(hits, indent=1))
