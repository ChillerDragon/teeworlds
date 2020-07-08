#!/bin/bash
# requires GNU sed

if [ "$#" == "0" ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]
then
    echo "usage: $(basename "$0") <input automapper file> [<output automapper file>]"
    echo "remapps indecies if created already"
    exit 0
fi

IN_FILE="$1"
OUT_FILE="$2"
if [ "$#" == "1" ]
then
    OUT_FILE="$IN_FILE"
fi
if [ ! -f "$IN_FILE" ]
then
    echo "Error: could not open file '$IN_FILE'"
    exit 1
fi
if [ -f "$OUT_FILE" ]
then
    echo "Warning '$OUT_FILE' already exists and will be overwritten"
	echo "do you continue? [y/N]"
	yn=""
	read -r -n 1 yn
	echo ""
	if ! [[ "$yn" =~ [yY] ]]
	then
        echo "aborting ..."
        exit 1
	fi
fi

rule_name="$(jq '.tileset[] | keys | .[0]' "$IN_FILE" | xargs)"
total_rules="$(jq ".tileset[0].\"$rule_name\".\"rules\" | length" "$IN_FILE")"
total_rules="$((total_rules+2))" # idk better too much than not enough big wise word english

echo "remapping indecies of $total_rules rules ..."

if [ "$OUT_FILE" == "$IN_FILE" ]
then
    sed -i 's/"rule_index":.*/"rule_index": XXX_RI_XXX,/' "$IN_FILE"
    for((i=0;i<total_rules;i++))
    do
        sed -i "0,/XXX_RI_XXX/s//$i/" "$IN_FILE"
    done
else
    sed 's/"rule_index":.*/"rule_index": XXX_RI_XXX,/' "$IN_FILE" > "$OUT_FILE"
    for((i=0;i<total_rules;i++))
    do
        sed -i "0,/XXX_RI_XXX/s//$i/" "$OUT_FILE"
    done
fi

