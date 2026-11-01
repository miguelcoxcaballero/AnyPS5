import hashlib, base64, struct, json, re, subprocess, urllib.request, time

def nid(name):
    d = hashlib.sha1(name.encode() + bytes.fromhex("518D64A635DED8C1E6B039B1C3E55230")).digest()
    return base64.b64encode(struct.pack(">Q", struct.unpack("<Q", d[:8])[0]), altchars=b"+-").decode()[:11]

names = ["FT_Atan2", "FT_Vector_Rotate", "FT_Vector_Length", "FT_Vector_Polarize", "FT_Vector_From_Polar",
         "_ZNSt13bad_exceptionD0Ev", "_ZNSt13bad_exceptionD1Ev", "_ZNSt13bad_exceptionD2Ev",
         "_ZNSt22_System_error_categoryD1Ev", "_ZNSt23_Generic_error_categoryD1Ev",
         "_ZNSt12future_errorD0Ev", "_ZNSt24_Iostream_error_categoryD1Ev"]
nids = {n: nid(n) for n in names}
print("NIDS", json.dumps(nids, indent=0))
for n, k in nids.items():
    r = subprocess.run(["git", "grep", "-n", "-F", k, "--", "core", "docs"], capture_output=True, text=True).stdout.strip()
    print("--", n, k); print(r[:1500])
r = subprocess.run(["git", "grep", "-n", "-E", "_ZNSt9exceptionD[012]Ev_nid_postfix|_ZNSt14error_categoryD[012]Ev_nid_postfix", "--", "core/libs/prx/libc/include"], capture_output=True, text=True).stdout
print("libc header decls:\n" + r)

keys = list(nids.values()) + names

def fetch(url):
    for attempt in range(8):
        try:
            return urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"}), timeout=30).read().decode("utf-8", "ignore")
        except Exception as e:
            wait = 20 * (attempt + 1)
            print("retry", url[-40:], e, "wait", wait); time.sleep(wait)
    return None

prs = set()
for order in ("", "+sort%3Acreated-asc"):
    for page in range(1, 41):
        html = fetch(f"https://github.com/boykopovar/AnyPS5/pulls?q=is%3Apr+is%3Aopen{order}&page={page}")
        if html is None:
            print("gave up", order, page); break
        found = set(int(x) for x in re.findall(r'id="issue_(\d+)"', html)) or set(int(x) for x in re.findall(r'/boykopovar/AnyPS5/pull/(\d+)"', html))
        if not found:
            break
        prs |= found
        time.sleep(2)
prs = sorted(prs)
m = re.search(r"([\d,]+)\s+Open", fetch("https://github.com/boykopovar/AnyPS5/pulls") or "")
print("open PRs found:", len(prs), "page says open:", m.group(1) if m else "?", "min", prs[0], "max", prs[-1], "has 11613:", 11613 in prs)
for i in range(0, len(prs), 50):
    chunk = prs[i:i + 50]
    r = subprocess.run(["git", "fetch", "-q", "up"] + [f"+refs/pull/{n}/head:refs/pr/{n}" for n in chunk], capture_output=True, text=True)
    if r.returncode:
        for n in chunk:
            subprocess.run(["git", "fetch", "-q", "up", f"+refs/pull/{n}/head:refs/pr/{n}"], capture_output=True, text=True)
hits, missing = {}, []
for num in prs:
    base = subprocess.run(["git", "merge-base", "up/main", f"refs/pr/{num}"], capture_output=True, text=True).stdout.strip()
    if not base:
        missing.append(num); continue
    diff = subprocess.run(["git", "diff", base, f"refs/pr/{num}"], capture_output=True, text=True, errors="ignore").stdout
    added = "\n".join(l for l in diff.splitlines() if l.startswith("+") or l.startswith("-"))
    for k in keys:
        if k in added:
            hits.setdefault(k, []).append(num)
print("not fetched:", missing)
print("HITS (changed lines only):", json.dumps(hits, indent=1))
