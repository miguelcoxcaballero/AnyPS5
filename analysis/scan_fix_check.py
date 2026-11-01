import importlib.util, re, sys, subprocess, shutil
from pathlib import Path

def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    m = importlib.util.module_from_spec(spec)
    sys.modules[name] = m
    spec.loader.exec_module(m)
    return m

isa = None
def report(tag, ss):
    global isa
    isa = ss.load_isa_forms("tools/rdna2_isa_forms.txt")
    r = ss.measure(".", isa)
    miss = {f["encoding"] + " " + f["form"]: f["missing"] for f in r["forms"] if f["missing"]}
    unt = {f["encoding"] + " " + f["form"]: f["untranslated"] for f in r["forms"] if f["untranslated"]}
    print(tag, r["instructions"], r["dpp"], "missing:", miss, "untranslated:", unt)

report("BEFORE", load("ss", "tools/shader_scan.py"))
src = Path("tools/shader_scan.py").read_text()
old_pair = 'PAIR = re.compile(r"(?:\\{\\s*|case\\s+)0x([0-9a-fA-F]+)u?\\s*(?:,|:\\s*return)\\s*RdnaOpcode::(\\w+)")'
new_pair = 'PAIR = re.compile(r"(?:\\{\\s*0x([0-9a-fA-F]+)u?\\s*,|((?:case\\s+0x[0-9a-fA-F]+u?\\s*:\\s*)+)return)\\s*RdnaOpcode::(\\w+)")'
old_loop = '''                for m in PAIR.finditer(block):
                    entries[int(m.group(1), 16)] = m.group(2)'''
new_loop = '''                for m in PAIR.finditer(block):
                    for number in [m.group(1)] if m.group(1) else re.findall(r"0x([0-9a-fA-F]+)", m.group(2)):
                        entries[int(number, 16)] = m.group(3)'''
print("old PAIR found:", old_pair in src, "old loop found:", old_loop in src)
src2 = src.replace(old_pair, new_pair).replace(old_loop, new_loop)
shutil.copy("tools/shader_scan.py", "/tmp/orig_scan.py")
Path("tools/shader_scan.py").write_text(src2)
report("AFTER", load("ss2", "tools/shader_scan.py"))
r = subprocess.run([sys.executable, "-m", "unittest", "tools/tests/test_shader_scan.py"], capture_output=True, text=True)
print("existing tests with fix:", r.returncode, r.stderr[-600:])
print(subprocess.run(["git", "diff", "tools/shader_scan.py"], capture_output=True, text=True).stdout)
shutil.copy("/tmp/orig_scan.py", "tools/shader_scan.py")
