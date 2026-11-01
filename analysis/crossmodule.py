import hashlib, base64, struct, re, json
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

root = Path("core/libs/prx")
mods = {}
names = {}
for lib in sorted(p for p in root.iterdir() if p.is_dir()):
    ex = {}
    for p in lib.rglob("*.cpp"):
        if "/tests/" in str(p):
            continue
        t = p.read_text(errors="ignore")
        b = bodies(t)
        for fn, body in b.items():
            if fn.endswith("_nid_postfix"):
                n = fn[:-len("_nid_postfix")]
                k = nid(n)
                names[k] = n
                ex[k] = {"fn": fn, "file": str(p), "impl": "NotImplemented" not in body, "size": body.count("\n")}
        for m in re.finditer(r"APS5_EXPORT\(\s*\"([^\"]+)\"\s*,\s*(\w+)\s*\)", t):
            body = b.get(m.group(2))
            if body is None:
                continue
            ex[m.group(1)] = {"fn": m.group(2), "file": str(p), "impl": "NotImplemented" not in body, "size": body.count("\n")}
    mods[lib.name] = ex

for p in Path("docs/dev/debt").glob("*.md"):
    for m in re.finditer(r"\|\s*`([A-Za-z_]\w*)`\s*\|\s*`([A-Za-z0-9+\-]{11})`\s*\|", p.read_text(errors="ignore")):
        names.setdefault(m.group(2), m.group(1))

impl_by_nid = {}
for lib, ex in mods.items():
    for k, v in ex.items():
        if v["impl"] and v["size"] >= 2:
            impl_by_nid.setdefault(k, []).append((lib, v["fn"], v["file"], v["size"]))
out = []
for lib, ex in mods.items():
    for k, v in ex.items():
        if v["impl"]:
            continue
        others = [o for o in impl_by_nid.get(k, []) if o[0] != lib]
        if others:
            out.append({"stub_lib": lib, "nid": k, "name": names.get(k), "stub_fn": v["fn"], "stub_file": v["file"], "impl": others})
by = {}
for o in out:
    by.setdefault(o["stub_lib"], 0)
    by[o["stub_lib"]] += 1
print("stub NIDs implemented in another module:", len(out))
print(json.dumps(by, indent=0))
for o in out:
    print(json.dumps(o))
json.dump(out, open("/tmp/cross.json", "w"))
