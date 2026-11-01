set -e
git remote add up https://github.com/boykopovar/AnyPS5.git || true
git fetch -q up main
git fetch -q origin '+refs/heads/*:refs/remotes/origin/*'
cp -r analysis /tmp/analysis
git checkout -q up/main
echo "== upstream main: $(git rev-parse --short HEAD)"
echo "== shader scan"
python3 /tmp/analysis/scan_fix_check.py
echo "== twins"
python3 /tmp/analysis/twins.py
echo "== overlap"
python3 /tmp/analysis/overlap.py || true
echo "== template diff"
git diff --stat up/main...origin/feat/libc-internal-exception-base-destructors
git diff up/main...origin/feat/libc-internal-exception-base-destructors | head -400
echo "== trig branch stat"
git diff --stat up/main...origin/feat/freetypeot-trigonometry || true
