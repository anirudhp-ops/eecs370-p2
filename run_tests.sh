#!/bin/bash
# Assembler tests: 15 error cases (expected exit code) + 5 valid cases
# (exit 0 and output must match test_N.obj.correct), plus CLI-argument checks.
cd "$(dirname "$0")" || exit 1
make -s assembler || exit 1
OUT=$(mktemp)
pass=0; fail=0
ok()  { pass=$((pass+1)); }
bad() { fail=$((fail+1)); echo "FAIL: $1"; }

# <test number> <expected exit code> <description>
errors=(
  "1 1 opcode is case-sensitive (ADD)"
  "2 1 label with no opcode"
  "3 1 register 8 in last operand"
  "4 1 negative register in middle operand"
  "5 1 missing operand"
  "6 1 lw offset 32768 (just over max)"
  "7 1 sw offset -32769 (just under min)"
  "8 1 beq numeric offset out of range"
  "9 1 undefined local label in sw"
  "10 1 beq to undefined global label"
  "11 1 .fill with undefined local label"
  "12 1 duplicate label across text/data"
  "13 1 register with trailing garbage (1x)"
  "14 2 blank line in middle of file"
  "15 1 line too long"
)
for e in "${errors[@]}"; do
    set -- $e; n=$1; want=$2; shift 2
    ./assembler "test_$n.as" "$OUT" > /dev/null 2>&1; got=$?
    [ "$got" -eq "$want" ] && ok || bad "test_$n: $* (expected exit $want, got $got)"
done

valid=(
  "16 register/offset/.fill boundaries, trailing blank lines"
  "17 undefined globals, Stack, case-sensitive labels, 63-char label"
  "18 interleaved .fill, backward/forward branches, relocs"
  "19 pseudo-ops, comments, tabs, CRLF"
  "20 empty file"
)
for v in "${valid[@]}"; do
    set -- $v; n=$1; shift
    ./assembler "test_$n.as" "$OUT" > /dev/null 2>&1; got=$?
    if [ "$got" -ne 0 ]; then bad "test_$n: $* (exit $got, expected 0)"
    elif ! diff -q "$OUT" "test_$n.obj.correct" > /dev/null; then bad "test_$n: $* (output differs)"
    else ok; fi
done

cli() { local d=$1; shift; "$@" > /dev/null 2>&1; [ $? -eq 1 ] && ok || bad "$d"; }
cli "no arguments"       ./assembler
cli "one argument"       ./assembler test_16.as
cli "too many arguments" ./assembler test_16.as "$OUT" extra
cli "missing input file" ./assembler does_not_exist.as "$OUT"
cli "unwritable output"  ./assembler test_16.as /nonexistent_dir/out.obj

rm -f "$OUT"
echo "passed $pass, failed $fail"
[ "$fail" -eq 0 ]
