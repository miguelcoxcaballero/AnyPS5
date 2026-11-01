import hashlib, base64, struct, re, subprocess, json
from pathlib import Path

SUF = bytes.fromhex("518D64A635DED8C1E6B039B1C3E55230")
def nid(name):
    d = hashlib.sha1(name.encode() + SUF).digest()
    v = struct.unpack("<Q", d[:8])[0]
    return base64.b64encode(struct.pack(">Q", v), altchars=b"+-").decode()[:11]

def bodies(text):
    out = {}
    for m in re.finditer(r"\b(\w+)\s*\(([^;{}]*)\)\s*(?:noexcept\s*)?\{", text):
        i = m.end() - 1
        depth = 0; k = i
        while k < len(text):
            c = text[k]
            if c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0: break
            k += 1
        out.setdefault(m.group(1), text[i:k + 1])
    return out

def module(root):
    exports = {}
    for p in Path(root).rglob("*.cpp"):
        if "/tests/" in str(p):
            continue
        t = p.read_text(errors="ignore")
        b = bodies(t)
        for name, body in b.items():
            if name.endswith("_nid_postfix") and name.startswith("_Z"):
                mangled = name[:-len("_nid_postfix")]
                exports[nid(mangled)] = {"name": mangled, "fn": name, "file": str(p), "impl": "NotImplemented" not in body}
        for m in re.finditer(r"APS5_EXPORT\(\s*\"([^\"]+)\"\s*,\s*(\w+)\s*\)", t):
            body = b.get(m.group(2))
            exports[m.group(1)] = {"name": None, "fn": m.group(2), "file": str(p), "impl": body is not None and "NotImplemented" not in body}
    return exports

libc = module("core/libs/prx/libc")
internal = module("core/libs/prx/libSceLibcInternal")
stubs = {n: v for n, v in internal.items() if not v["impl"]}
print("libc exports:", len(libc), "implemented:", sum(v["impl"] for v in libc.values()))
print("libSceLibcInternal exports:", len(internal), "stubs:", len(stubs))

names = {}
for line in Path("docs/dev/debt/libSceLibcInternal.md").read_text().splitlines():
    m = re.match(r"\|\s*`([^`]+)`\s*\|\s*`([^`]+)`\s*\|", line)
    if m:
        names[m.group(2)] = m.group(1)
for n, v in libc.items():
    if v["name"]:
        names.setdefault(n, v["name"])

mine = {"s2eGsgUF9vk", "v09ZcAhZzSc", "XFh0C66aEms", "OypvNf3Uq3c", "xiqd+QkuYXc", "pN02FS5SPgg", "YXVCU6PdgZk", "dsJKehuajH4", "X9D8WWSG3As", "XyJPhPqpzMw"}
VB = ("istream", "ostream", "iostream", "strstream", "stringstream", "fstream")
swap = {"C1": "C2", "C2": "C1", "D1": "D2", "D2": "D1"}
out = []
for n, s in sorted(stubs.items()):
    if n in mine:
        continue
    name = names.get(n)
    if n in libc and libc[n]["impl"]:
        out.append({"kind": "same-nid", "nid": n, "name": name, "stub": s["fn"], "stub_file": s["file"], "libc_fn": libc[n]["fn"], "libc_file": libc[n]["file"]})
        continue
    if not name:
        continue
    ms = list(re.finditer(r"(?<=[A-Za-z0-9_])(C1|C2|D1|D2)(?=E)", name))
    if len(ms) == 1:
        m = ms[0]
        twin = name[:m.start()] + swap[m.group(1)] + name[m.end():]
        tn = nid(twin)
        if tn in libc and libc[tn]["impl"]:
            out.append({"kind": "twin", "nid": n, "name": name, "twin": twin, "stub": s["fn"], "stub_file": s["file"], "libc_fn": libc[tn]["fn"], "libc_file": libc[tn]["file"], "vb_risk": any(v in name for v in VB)})
    if name.endswith("D0Ev"):
        d1 = name[:-4] + "D1Ev"
        tn = nid(d1)
        if tn in libc and libc[tn]["impl"]:
            out.append({"kind": "deleting-from-D1", "nid": n, "name": name, "twin": d1, "stub": s["fn"], "stub_file": s["file"], "libc_fn": libc[tn]["fn"], "libc_file": libc[tn]["file"], "libc_has_D0": nid(name) in libc})
nm = [o.get("name") or "" for o in out]
dem = subprocess.run(["c++filt"], input="\n".join(nm), capture_output=True, text=True).stdout.split("\n")
for o, d in zip(out, dem):
    o["demangled"] = d
print("CANDIDATES", len(out))
for o in out:
    print(json.dumps(o))
json.dump(out, open("/tmp/cands.json", "w"))
