#!/bin/mksh
#
# Expects to be run from the m68k dir

mkdir -p "test/funcs/obj"

# _cproc SRC DEST
_cproc() {
    cpp -Itest/ -I../third_party/libqbe "$1" | cproc-qbe -tm68k >| "$2"
}

# _qbe SSA ASM
_qbe() {
    ../obj/qbe -Gv -tm68k "$1" >| "$2"
}

# _vasm ASM OBJ
_vasm() {
    vasmm68k_std -quiet -Fvobj "$1" -o "$2"
}

# _vlink BIN OBJ...
_vlink() {
    bin="$1"
    shift 1
    vlink -brawbin1 -evector_table -o "$bin" $@
}

S_ERR="\x1b[31mERR\x1b[m"
test_print() {
    printf '\x1b[A\r\x1b[1m%-75s\x1b[m%b\n' "${1}..." "${2}"
}

for f in 'test/funcs/'*
do
    # Reset variables that can be defined in the header of test funcs:
    expect_output=
    expect_D0=

    # Skip obj dir
    [ -d "$f" ] && continue

    name="$(basename "$f")"
    name="${name%%.*}"

    printf '%-75s\n' "${name}..."

    ssa="test/funcs/obj/${name}.ssa"
    asm="test/funcs/obj/${name}.asm"
    obj="test/funcs/obj/${name}.o"
    bin="test/funcs/obj/${name}.bin"
    harness_asm="test/funcs/obj/harness-${name}.asm"
    harness_obj="test/funcs/obj/harness-${name}.o"

    eval $(grep '^//\$' $f | sed 's/^\/\/\$ //g')

    grep '^//\:' $f | sed 's/^\/\/\:[ ]*//g' >| $harness_asm

    _cproc "$f"   "$ssa"                               || break
    _qbe   "$ssa" "$asm"                               || break
    _vasm  "$asm" "$obj"                               || break
    _vasm  "$harness_asm" "$harness_obj"               || break
    _vlink "$bin" 'test/rt/rt.o' "$harness_obj" "$obj" || break

    stderr_file=$(mktemp)
    stdout=$(sim "$bin" 2> "$stderr_file")
    stderr=$(cat "$stderr_file")

    if [[ "$expect_output" != "$stdout" ]]
    then
        test_print $name $S_ERR
        printf "Unexpected output (got: %q, expected: %q)\n\n" \
            "$stdout" "$expect_output"
        break
    fi

    actual_D0=()
    while read -r _ _ val _
    do
        actual_D0+=($val)
    done <<< $(grep '^G D0' <<< "$stderr")

    if [[ "$expect_D0" != "${actual_D0[*]}" ]]
    then
        test_print $name $S_ERR
        printf "Unexpected D0 values (got: %q, expected: %q)\n\n" \
            "${actual_D0[*]}" "$expect_D0"
        break
    fi

    test_print $name "OK"
done
