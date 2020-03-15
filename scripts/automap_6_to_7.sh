#!/bin/bash

RULE_FILE="${1:-generic_clear.rules}"
RULE_FILE_NAME="${RULE_FILE%.rules*}"
TILESET_NAME="${2:-$RULE_FILE_NAME}"
BASE_TILE="x"
current=-1
declare -A R_CONDITIONS
declare -A R_INDEX
declare -A R_FLIP

if [ "$1" == "help" ] || [ "$1" == "--help" ] || [ "$1" == "-h" ] || [ "$#" == "0" ]
then
    echo "usage: $(basename "$0") <0.6 rules file> [tileset name]"
    echo "description: prints 0.6 rules json to stdout"
    # exit 0
fi

function err() {
    echo "[-] Error: $1"
}

if [ ! -f "$RULE_FILE" ]
then
    err "Rule file not found '$RULE_FILE'"
    exit 1
fi

function assert_name() {
    if [ "$current_rule" == "" ]
    then
        err "Expected rule name before line $line_num"
        exit 1
    fi
}

while IFS= read -r line
do
    if [[ "$line" =~ ^\[(.*)\] ]]
    then
        current_rule="${BASH_REMATCH[1]}"
    elif [[ "$line" =~ ^\# ]]
    then
        continue
    elif [[ "$line" =~ ^Index\ ([0-9]*)?(.*) ]]
    then
        if [ "$BASE_TILE" == "x" ]
        then
            BASE_TILE="${BASH_REMATCH[1]}"
            continue
        fi
        current="$((current + 1))"
        R_INDEX[$current]="${BASH_REMATCH[1]}"
        R_FLIP[$current]="${BASH_REMATCH[2]}"
        echo "flip $current: ${R_FLIP[$current]}"
    elif [[ "$line" =~ ^Pos\ ([0-9-]*)\ ([0-9-]*)\ ([A-Z]*) ]]
    then
        assert_name
        x="${BASH_REMATCH[1]}"
        y="${BASH_REMATCH[2]}"
        val="${BASH_REMATCH[3]}"
        # TODO: NOTINDEX AND INDEX
        # have different format
        values='(FULL|EMPTY|NOTINDEX|INDEX)'
        if ! [[ "$val" =~ $values ]]
        then
            err "Invalid value '$val' at line $line_num expected $values"
            exit 1
        fi
        R_CONDITIONS[$current]+="{\"x\": $x, \"y\": $y, \"value\": \"${val,,}\"},\\n"
    fi
    line_num="$((line_num + 1))"
done < "$RULE_FILE"


(
echo "{\"tileset\": [ { \"$TILESET_NAME\": { \"basetile\": $BASE_TILE, \"rules\": ["

for r in "${!R_CONDITIONS[@]}"
do
    cond="${R_CONDITIONS[$r]}"
    index="${R_INDEX[$r]}"
    flip="${R_FLIP[$r]}"
    echo "{"
    echo "\"index\": $index,"
    echo "\"rule_number\": $r,"
    echo '"condition": ['
    echo -e "${cond::-3}"
    printf "]"
    if [ -z "$flip" ]
    then
        echo ","
        echo "\"${flip}\": 1"
    fi
    printf "]}"
    if [ "$r" -lt "$current" ]
    then
        echo ","
    fi
done

echo ']}}]}'
) # | jq
