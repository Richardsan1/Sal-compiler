#!/usr/bin/env bash
set -u

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR" || exit 1

pass=0
fail=0

run_expect_success() {
  local file="$1"
  if ./salc --tokens --symtab --trace "$file" >/tmp/salc.out 2>/tmp/salc.err; then
    echo "[OK]   $file (esperado sucesso)"
    pass=$((pass + 1))
  else
    echo "[FAIL] $file (esperado sucesso)"
    cat /tmp/salc.err
    fail=$((fail + 1))
  fi
}

run_expect_failure() {
  local file="$1"
  if ./salc "$file" >/tmp/salc.out 2>/tmp/salc.err; then
    echo "[FAIL] $file (esperado falha)"
    fail=$((fail + 1))
  else
    echo "[OK]   $file (esperado falha)"
    pass=$((pass + 1))
  fi
}

echo "== Compilando salc =="
if ! make >/tmp/salc_make.out 2>/tmp/salc_make.err; then
  echo "Falha no make"
  cat /tmp/salc_make.err
  exit 1
fi

for f in tests/valid/*.sal; do
  run_expect_success "$f"
done

for f in tests/invalid/*.sal; do
  run_expect_failure "$f"
done

echo
echo "Resumo: $pass passou, $fail falhou"

if [[ $fail -ne 0 ]]; then
  exit 1
fi
