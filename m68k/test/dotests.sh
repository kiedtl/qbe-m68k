#!/bin/mksh
#
# Expects to be run from the m68k dir

TEST_FUNCS=(
    hello
    loop
    strlen
    strcat
    div
)

mkdir -p "test/funcs/obj"

ts() {
    chars=("-" "\\" "|" "/")

    c="+"
    esc1=""
    esc2=""

    case "$1" in
        -1) ;;
        0)
            esc1="\x1b[A\r\x1b[K"
            esc2="\x1b[1m"
        ;;
        *)
            c=${chars[$(($1 % ${#chars[*]}))]}
            esc1="\x1b[A\r\x1b[K"
        ;;
    esac

    status="$2"
    status="$(printf '%b' "$status")"

    printf '%b%s %b%-65s\x1b[m%10s\n' \
        "$esc1" "$c" "$esc2" "$name..." "$status"
}

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

for name in ${TEST_FUNCS[*]}
do
    # Reset variables that can be defined in the header of test funcs:
    expect_output=
    expect_D0=

    ts -1 "init"

    src="test/funcs/${name}.c"
    ssa="test/funcs/obj/${name}.ssa"
    asm="test/funcs/obj/${name}.asm"
    obj="test/funcs/obj/${name}.o"
    bin="test/funcs/obj/${name}.bin"
    harn_asm="test/funcs/obj/harness-${name}.asm"
    harn_obj="test/funcs/obj/harness-${name}.o"

    eval $(grep '^//\$' $src | sed 's/^\/\/\$ //g')

    grep '^//\:' $src | sed 's/^\/\/\:[ ]*//g' >| $harn_asm

    ts 1 "cproc"   && (_cproc "$src" "$ssa"                            || break)
    ts 2 "qbe"     && (_qbe   "$ssa" "$asm"                            || break)
    ts 3 "vasm"    && (_vasm  "$asm" "$obj"                            || break)
    ts 4 "harness" && (_vasm  "$harn_asm" "$harn_obj"                  || break)
    ts 5 "vlink"   && (_vlink "$bin" 'test/rt/rt.o' "$harn_obj" "$obj" || break)

    stderr_file=$(mktemp)
    stdout=$(sim "$bin" 2> "$stderr_file")
    stderr=$(cat "$stderr_file")

    if [[ "$expect_output" != "$stdout" ]]
    then
        ts 0 NAH
        printf "Unexpected output (got: %q, expected: %q)\n\n" \
            "$stdout" "$expect_output"
    fi

    actual_D0=()
    while read -r _ _ val _
    do
        actual_D0+=($val)
    done <<< $(grep -a '^G D0' <<< "$stderr")

    if [[ "$expect_D0" != "${actual_D0[*]}" ]]
    then
        ts 0 NAH
        printf "Unexpected D0 values (got: %q, expected: %q)\n\n" \
            "${actual_D0[*]}" "$expect_D0"
    fi

    ts 0 "OK"
done
