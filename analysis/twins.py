import hashlib, base64, struct, re, subprocess, json
from pathlib import Path

SUF = bytes.fromhex("518D64A635DED8C1E6B039B1C3E55230")
def nid(name):
    d = hashlib.sha1(name.encode() + SUF).digest()
    v = struct.unpack("<Q", d[:8])[0]
    return base64.b64encode(struct.pack(">Q", v), altchars=b"+-").decode()[:11]
print("nid check:", nid("_ZNSt4_PadD1Ev"), "expected XyJPhPqpzMw")

stubs = {}
for p in Path("core/libs/prx/libSceLibcInternal").glob("*.cpp"):
    t = p.read_text(errors="ignore")
    for m in re.finditer(r"(\w+)\s*\([^)]*\)\s*\{([^{}]*)\}\s*APS5_EXPORT\(\"([^\"]+)\",\s*(\w+)\)", t):
        if "NotImplemented" in m.group(2):
            stubs[m.group(3)] = (p.name, m.group(4))
print("libSceLibcInternal stubs:", len(stubs))

impl = {}
for p in Path("core/libs/prx/libc").rglob("*.cpp"):
    t = p.read_text(errors="ignore")
    for m in re.finditer(r"\b(_Z\w+?)_nid_postfix\s*\(", t):
        name = m.group(1)
        i = t.find("{", m.end()); j = t.find(";", m.end())
        if i < 0 or (0 <= j < i):
            continue
        depth = 0; k = i
        while k < len(t):
            if t[k] == "{": depth += 1
            elif t[k] == "}":
                depth -= 1
                if depth == 0: break
            k += 1
        impl[name] = (str(p), "NotImplemented" not in t[i:k])
print("libc mangled defs:", len(impl), "implemented:", sum(1 for v in impl.values() if v[1]))

VB = ("istream", "ostream", "iostream", "strstream", "stringstream", "fstream")
mine = {"s2eGsgUF9vk", "v09ZcAhZzSc", "XFh0C66aEms", "OypvNf3Uq3c", "xiqd+QkuYXc", "pN02FS5SPgg", "YXVCU6PdgZk", "dsJKehuajH4", "X9D8WWSG3As", "XyJPhPqpzMw"}
swap = {"C1": "C2", "C2": "C1", "D1": "D2", "D2": "D1"}
cands = []
for name, (path, ok) in sorted(impl.items()):
    if not ok:
        continue
    ms = list(re.finditer(r"(?<=[A-Za-z0-9_])(C1|C2|D1|D2)(?=E)", name))
    if len(ms) != 1:
        continue
    m = ms[0]
    twin = name[:m.start()] + swap[m.group(1)] + name[m.end():]
    n = nid(twin)
    if n in stubs and n not in mine and twin not in impl:
        cands.append({"nid": n, "twin": twin, "libc": name, "libc_file": path, "stub": stubs[n], "vb_risk": any(v in name for v in VB)})
dem = subprocess.run(["c++filt"], input="\n".join(c["twin"] for c in cands), capture_output=True, text=True).stdout.split("\n")
for c, d in zip(cands, dem):
    c["demangled"] = d
print("CANDIDATES", len(cands))
for c in cands:
    print(json.dumps(c))
json.dump(cands, open("/tmp/cands.json", "w"))
