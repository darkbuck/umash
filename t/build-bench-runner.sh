#!/bin/sh

set -e
BASE=$(dirname $(readlink -f "$0"))
COMMIT=${1:-"WIP"}
CC=${CC:-cc}
CFLAGS=${CFLAGS:- -O2 -std=gnu99 -W -Wall -mpclmul}

cd "${BASE}/../";

if [ "x$COMMIT" = "xWIP" ];
then
    echo "Building umash_bench_runner-WIP.so from the current tree.";
    echo "CC: $CC CFLAGS: $CFLAGS";
    $CC $CFLAGS -DVERSION_SUFFIX="_WIP" -I. \
         '-DUMASH_SECTION="umash_code_WIP"' \
         bench/runner.c umash.c \
         -fPIC --shared -o "umash_bench_runner-WIP.so";

    # Localise any global symbol that does not have the _WIP suffix.
    LOCALIZE=$(nm -g "umash_bench_runner-WIP.so" | \
        awk '(/[0-9a-f]+ [A-Z] / && ! ($3 ~ /_WIP$/)) {print "-L " $3}')

    exec objcopy $LOCALIZE "umash_bench_runner-WIP.so"
fi

SHA=$(git show "$COMMIT" --pretty=format:%H | head -1)
echo "Building umash_bench_runner-$SHA.so";
echo "CC: $CC CFLAGS: $CFLAGS";

mkdir -p bench-worktrees;
git worktree add -f "bench-worktrees/$SHA" "$SHA";
cd "bench-worktrees/$SHA";

# The code in `bench/runner` expects this suffixed section name to
# avoid name collisions when loading multiple versions.
$CC $CFLAGS -DVERSION_SUFFIX="_$SHA" -I. \
    "-DUMASH_SECTION=\"umash_code_$SHA\"" \
    bench/runner.c umash.c \
    -fPIC --shared -o "../../umash_bench_runner-$SHA.so";

cd ../..;
git worktree remove --force "bench-worktrees/$SHA";

# Turn any global symbol that does not include the SHA suffix into a
# local one that will not leak into the global `dlopen` namespace.
LOCALIZE=$(nm -g "umash_bench_runner-$SHA.so" | \
    awk "(/[0-9a-f]+ [A-Z] / && ! (\$3 ~ /_$SHA\$/)) {print \"-L \" \$3}")

objcopy $LOCALIZE "umash_bench_runner-$SHA.so"
