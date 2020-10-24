#!/bin/bash


n=6
m=8
x=8

for ((i=1;i<=$#;i++)); 
do

    if [ ${!i} = "-n" ] 
    then ((i++)) 
        n=${!i};

    elif [ ${!i} = "-m" ];
    then ((i++)) 
        m=${!i};  

    elif [ ${!i} = "-x" ];
    then ((i++)) 
        x=${!i};    
    else
    	echo "Unknown argument: ${!i}" 1>&2;
		exit 1
    fi

done;

re='^[0-9]+$'
if ! [[ $n =~ $re ]] ; then
   echo "Expected an integer after -n" >&2; exit 1
fi
if ! [[ $x =~ $re ]] ; then
   echo "Expected an integer after -x" >&2; exit 1
fi
if ! [[ $m =~ $re ]] ; then
   echo "Expected an integer after -m" >&2; exit 1
fi

if [ "$x" -lt "$m" ]; then
	x="$m"
fi


for ((i=1;i<="$n";i++)); do
	l=$(shuf -i "$m"-"$x" -n 1)
	
    cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w "$l" | head -n 1
done




