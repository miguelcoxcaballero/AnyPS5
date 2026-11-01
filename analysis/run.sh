set -e
git remote add up https://github.com/boykopovar/AnyPS5.git || true
git fetch -q up main
cp -r analysis /tmp/analysis
git checkout -q up/main
echo "== upstream main: $(git rev-parse --short HEAD)"
echo "== cross-module candidates"
python3 /tmp/analysis/crossmodule.py
echo "== overlap"
python3 /tmp/analysis/overlap.py || true
echo "== libc D0 pattern"
grep -n "D0Ev_nid_postfix" -A6 core/libs/prx/libc/src/exception/StandardExceptions.cpp | head -60 || true
