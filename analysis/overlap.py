import json, re, subprocess, urllib.request, time
cross = json.load(open("/tmp/cross.json"))
import hashlib, base64, struct
def nid(name):
    d = hashlib.sha1(name.encode() + bytes.fromhex("518D64A635DED8C1E6B039B1C3E55230")).digest()
    return base64.b64encode(struct.pack(">Q", struct.unpack("<Q", d[:8])[0]), altchars=b"+-").decode()[:11]
trig = ["FT_Atan2", "FT_Vector_Rotate", "FT_Vector_Length", "FT_Vector_Polarize", "FT_Vector_From_Polar"]
extra = trig + [nid(n) for n in trig] + ["rNYLEsL7M0k", "_ZNSt12future_errorD0Ev"]
print("trig NIDs:", {n: nid(n) for n in trig})
keys = [c["nid"] for c in cross] + [c["name"] for c in cross if c.get("name")] + extra
prs = set()
for page in range(1, 120):
    url = f"https://github.com/boykopovar/AnyPS5/pulls?q=is%3Apr+is%3Aopen&page={page}"
    html = None
    for attempt in range(3):
        try:
            html = urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"}), timeout=30).read().decode("utf-8", "ignore")
            break
        except Exception as e:
            print("page", page, "error", e); time.sleep(5)
    if html is None:
        break
    found = set(int(x) for x in re.findall(r'id="issue_(\d+)"', html)) or set(int(x) for x in re.findall(r'/boykopovar/AnyPS5/pull/(\d+)"', html))
    if not found:
        break
    prs |= found
    time.sleep(0.5)
prs = sorted(prs)
print("open PRs found:", len(prs), prs[:5], "...", prs[-5:], "11613 in list:", 11613 in prs)
for i in range(0, len(prs), 50):
    chunk = prs[i:i + 50]
    r = subprocess.run(["git", "fetch", "-q", "up"] + [f"+refs/pull/{n}/head:refs/pr/{n}" for n in chunk], capture_output=True, text=True)
    if r.returncode:
        for n in chunk:
            subprocess.run(["git", "fetch", "-q", "up", f"+refs/pull/{n}/head:refs/pr/{n}"], capture_output=True, text=True)
hits = {}
missing = []
for num in prs:
    base = subprocess.run(["git", "merge-base", "up/main", f"refs/pr/{num}"], capture_output=True, text=True).stdout.strip()
    if not base:
        missing.append(num); continue
    diff = subprocess.run(["git", "diff", base, f"refs/pr/{num}"], capture_output=True, text=True, errors="ignore").stdout
    for k in keys:
        if k and k in diff:
            hits.setdefault(k, []).append(num)
print("PRs not fetched:", missing)
free = [c for c in cross if c["nid"] not in hits and not (c.get("name") and c["name"] in hits)]
print("OVERLAP HITS (extra keys):", json.dumps({k: v for k, v in hits.items() if k in extra}))
print("CROSS CANDIDATES WITH NO OPEN PR:", len(free))
for c in free:
    print(json.dumps(c))
