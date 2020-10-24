#!/bin/bash



ans="195a30a1d1561cbc0ae7c488b93d037f6b713354"
out=""

#echo "$ans"

for a in 1 2; do
    for b in 1 2; do
        for c in 1 2; do
            for d in 1 2; do
                for e in 1 2; do
                    s="Base{32,64}_Is_Stupid_But_Sometimes_Useful"
                    if [ $a -eq 1 ]; then
                        s=$(echo "$s" | base64)
                    else
                        s=$(echo "$s" | base32)
                    fi
                    if [ $b -eq 1 ]; then
                        s=$(echo "$s" | base64)
                    else
                        s=$(echo "$s" | base32)
                    fi
                    if [ $c -eq 1 ]; then
                        s=$(echo "$s" | base64)
                    else
                        s=$(echo "$s" | base32)
                    fi
                    if [ $d -eq 1 ]; then
                        s=$(echo "$s" | base64)
                    else
                        s=$(echo "$s" | base32)
                    fi
                    if [ $e -eq 1 ]; then
                        s=$(echo "$s" | base64)
                    else
                        s=$(echo "$s" | base32)
                    fi
                    s=$(echo "$s" | sha1sum | cut -d' ' -f 1)
                    
                    if [[ $s == $ans ]]; then
                        
                        if [ $a -eq 1 ]; then
                            out=$out"base64"
                        else
                            out=$out"base32"
                        fi
                        if [ $b -eq 1 ]; then
                            out=$out" -> base64"
                        else
                            out=$out" -> base32"
                        fi
                        if [ $c -eq 1 ]; then
                            out=$out" -> base64"
                        else
                            out=$out" -> base32"
                        fi
                        if [ $d -eq 1 ]; then
                           out=$out" -> base64"
                        else
                            out=$out" -> base32"
                        fi
                        if [ $e -eq 1 ]; then
                            out=$out" -> base64"
                        else
                            out=$out" -> base32"
                        fi
                    fi
                done
            done
        done
    done
done

echo "$out"
