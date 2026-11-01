import json, re, subprocess, urllib.request, time
cands = json.load(open("/tmp/cands.json"))
extra = ["FT_Atan2", "FT_Vector_Rotate", "FT_Vector_Length", "FT_Vector_Polarize", "FT_Vector_From_Polar", "shader_scan", "CBRANCH_CDBG", "SCbranchCdbg", "ENDPGM_SAVED", "rdna2_isa_forms"]
keys = [c["nid"] for c in cands] + [c["name"] for c in cands if c.get("name")] + extra
prs = set()
for page in range(1, 40):
    url = f"https://github.com/boykopovar/AnyPS5/pulls?q=is%3Apr+is%3Aopen&page={page}"
    try:
        html = urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"}), timeout=30).read().decode("utf-8", "ignore")
    except Exception as e:
        print("page", page, "error", e); break
    found = set(int(x) for x in re.findall(r'id="issue_(\d+)"', html)) or set(int(x) for x in re.findall(r'/boykopovar/AnyPS5/pull/(\d+)"', html))
    if not found - prs:
        break
    prs |= found
    time.sleep(1)
prs = sorted(prs)
print("open PRs found:", len(prs), prs[:5], "...", prs[-5:])
print("11613 open:", 11613 in prs)
for i in range(0, len(prs), 80):
    chunk = prs[i:i + 80]
    subprocess.run(["git", "fetch", "-q", "up"] + [f"+refs/pull/{n}/head:refs/pr/{n}" for n in chunk], capture_output=True, text=True)
hits = {}
for num in prs:
    base = subprocess.run(["git", "merge-base", "up/main", f"refs/pr/{num}"], capture_output=True, text=True).stdout.strip()
    if not base:
        print("no ref", num); continue
    diff = subprocess.run(["git", "diff", base, f"refs/pr/{num}"], capture_output=True, text=True, errors="ignore").stdout
    for k in keys:
        if k and k in diff:
            hits.setdefault(k, []).append(num)
print("OVERLAP HITS:", json.dumps(hits, indent=1))
