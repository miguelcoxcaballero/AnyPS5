set -e
git remote add up https://github.com/boykopovar/AnyPS5.git || true
git fetch -q up main
cp -r analysis /tmp/analysis
git checkout -q up/main
echo "== upstream main: $(git rev-parse --short HEAD)"
python3 /tmp/analysis/focus.py
