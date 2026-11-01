set -e
git remote add up https://github.com/boykopovar/AnyPS5.git || true
git fetch -q up main
git fetch -q origin '+refs/heads/*:refs/remotes/origin/*'
cp -r analysis /tmp/analysis
git checkout -q up/main
echo "== upstream main: $(git rev-parse --short HEAD)"
echo "== candidates"
python3 /tmp/analysis/twins.py
echo "== overlap"
python3 /tmp/analysis/overlap.py || true
