#!/bin/bash
set +e
W=$(pwd)
git fetch --no-tags --depth=200 https://github.com/boykopovar/AnyPS5.git main:refs/remotes/origin/main || true
git fetch --unshallow -q || true
echo "=== conventions"
cp -r scratch /tmp/scratch
git checkout -q HEAD~1
git log -1 --format='%h %s'
python3 tools/check_conventions.py --base origin/main 2>&1 | tail -30; echo "conventions exit ${PIPESTATUS[0]}"
echo "=== configure"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache > configure.log 2>&1 || { tail -60 configure.log; exit 1; }
echo "=== build"
cmake --build build --target guest_freetype_ot_trigonometry_tests guest_freetype_corner_tests guest_freetype_ot_memory_tests guest_freetype_stream_tests guest_freetype_stream_libraries --parallel 4 2>&1 | grep -E "error|warning: |FAILED|Linking" | head -40
echo "=== test"
ctest --test-dir build -R "guest_freetype_ot_trigonometry|guest_freetype_corner|guest_freetype_ot_memory|guest_freetype_stream" --output-on-failure 2>&1 | tail -25
echo "=== exports"
SO=$(find build -name 'libSceFreeTypeOt.so' | head -1); echo "$SO"
nm -D --defined-only "$SO" | grep -E "FT_(Cos|Sin|Tan|Atan2|Vector_Unit|Vector_Rotate|Vector_Length|Vector_Polarize|Vector_From_Polar|Angle_Diff)"
echo "=== FreeType 2.4.11"
(cd /tmp && curl -sL -o ft.tgz https://download.savannah.gnu.org/releases/freetype/freetype-old/freetype-2.4.11.tar.gz && tar xzf ft.tgz && cd freetype-2.4.11 && ./configure --prefix=/tmp/ftA --without-zlib --without-bzip2 -q >/dev/null 2>&1 && make -j4 -s >/dev/null 2>&1 && make install -s >/dev/null 2>&1) && echo ft2411-ok
gcc -O2 -c /tmp/scratch/ref.c -o /tmp/ref.o -I/tmp/ftA/include -I/tmp/ftA/include/freetype2
FLAGS=$(python3 - <<'PY'
import json, shlex
cc = [c for c in json.load(open('build/compile_commands.json')) if c['file'].endswith('GuestFreeTypeOtTrigonometry.cpp')][0]
args = shlex.split(cc['command'])[1:]
out = []
skip = False
for x in args:
    if skip:
        skip = False
        continue
    if x in ('-o', '-c', '-MF', '-MT'):
        skip = True
        continue
    if x.endswith('.cpp') or x.startswith('-MD'):
        continue
    out.append(x)
print(' '.join(shlex.quote(x) for x in out))
PY
)
eval g++ $FLAGS -O2 /tmp/scratch/harness.cpp /tmp/ref.o "$W/$SO" /tmp/ftA/lib/libfreetype.a -Wl,-rpath,$(dirname "$W/$SO") -o /tmp/harness && time /tmp/harness
