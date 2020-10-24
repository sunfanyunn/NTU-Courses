#!/usr/bin/env bash 
while [[ "$#" -gt 0 ]]; do case $1 in
    -f) infile="$2";  shift 2;;
    -g) graduate="true"; shift;;
    *) break;;
  esac;
done

if [ -z "$infile" ]; then
	echo "Missing input file"; exit 1
fi

if [ ! -f "$infile" ]; then
	echo "Input file not found"; exit 1
fi

grep "print_table" "$infile" \
    | sed 's/<[^>]*>/;/g' \
    | sed 's/;;*/|/g' \
    | while read -r line; do
        name=$(echo "$line" | cut -d'|' -f7)
        professor=$(echo "$line" | cut -d'|' -f12)
        schedule=$(echo "$line" | cut -d'|' -f14 \
            | tr -d ' ' \
            | sed 's/(.*$//g' \
            | sed 's/^\(...\)\(.*\)/\1 \2/g')

        if [ "$professor" = "&nbsp" ]; then
            professor="N/A"
        fi

        if [ "$schedule" = "&nbsp" ]; then
            schedule="N/A"
        fi
        
        if [[ "$graduate" != "true" || ( \
              "$name" != *"Special"*"Project"* && \
              "$name" != *"Seminar"* && \
              "$name" != *"Thesis"* ) ]]; then
            printf "%-50s\t%-20s\t%-10s\n" "$name" "$professor" "$schedule";
        fi
      done
