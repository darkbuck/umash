#!/bin/sh
set -e
BASE=$(dirname $(readlink -f "$0"))
PYTHON=${PYTHON3:-python3}

(cd "${BASE}/../bench/";
 ${CC:-cc} ${CFLAGS:- -O2 -std=c99 -W -Wall} xoshiro.c exact_test.c \
	   -fPIC --shared -o ../exact.so
)

$PYTHON -m venv "${BASE}/umash-venv/"

. "${BASE}/umash-venv/bin/activate"

pip3 install wheel
pip3 install --prefer-binary -r "${BASE}/requirements.txt"
pip3 install --prefer-binary -r "${BASE}/requirements-bench.txt"

black "${BASE}/"*.py

mkdir -p "${BASE}/protos"
$PYTHON -m grpc_tools.protoc -I"${BASE}/../bench/protos" \
        --python_out="${BASE}/protos/" --grpc_python_out="${BASE}/protos/" \
        "${BASE}/../bench/protos/"*.proto

cd "${BASE}"
exec env PYTHONPATH="$BASE:$BASE/protos/:$PYTHONPATH" \
     $PYTHON exact_test_sampler_server.py "$@"

