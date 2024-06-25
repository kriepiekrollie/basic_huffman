#!/bin/bash

run_command() {
    echo ""
    echo -e "\e[36;1m$1\e[0m"
    echo -e "$3\e[31;1m"
    eval "$3"
    if ! [[ $? -eq 0 ]]; then
        echo -e "\e[0m"
        exit 1
    fi
    echo -e "\e[32m$2\e[0m"
}

original="$1"

run_command "Compiling..." "Finished compiling!" "./compile.sh"
if [ -z "$1" ]; then
    run_command "Generating random input..." "Finished generating input." "tr -dc \"A-Za-z 0-9\" < /dev/urandom | fold -w100|head -n 100000 > original"
    original="original"
fi

run_command "Encoding..." "Finished encoding." "./huffman -e $original encoded"
run_command "Decoding..." "Finished decoding." "./huffman -d encoded decoded"

echo ""
echo -e "\e[36mDiff-checking:$1\e[0m"
echo "diff $original decoded"
diff $original decoded

if diff $original decoded >/dev/null; then
    echo -e "\e[32;1mFiles are identical.\e[0m"
else
    echo -e "\e[31;1mFiles are different.\e[0m"
fi
