#!/usr/bin/env bash
if (( $# < 2 )); then
    echo Missing input file;
    exit
fi
if [[ ! -f $2 ]]; then
    echo Input file not found;
    exit
fi

flag=0
if (($# == 3 )); then
    if [ "$3" == "-g" ]; then
        flag=1;
    fi
fi
IFS=$'\n'
array=("Seminar" "Project" "Thesis")
content=$(cat $2 | grep '<A .*ser_no=.*>.*</A>')
export IFS
for line in $content
do 
    #echo $line | sed -n 's:<tr.*><.*></.*><.*></.*><.*></.*><.*></.*><.*><A.*>\(.*\)</A></.*></tr>:\1:p'
    #echo $line | grep "><"
    course=$(echo $line | sed  -n 's:.*<TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*<A.*>\(.*\)</A></TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>\(.*\)</TD><TD>.*</TD><TD>\(.*\)</TD><TD>.*</TD><TD.*>.*</TD><TD.*>.*</TD><TD>.*</TD><td>.*</td>.*:\1:p') 
    people=$(echo $line | sed  -n 's:.*<TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*<A.*>\(.*\)</A></TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>\(.*\)</TD><TD>.*</TD><TD>\(.*\)</TD><TD>.*</TD><TD.*>.*</TD><TD.*>.*</TD><TD>.*</TD><td>.*</td>.*:\2:p') 
    time=$(echo $line | sed  -n 's:.*<TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*<A.*>\(.*\)</A></TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>.*</TD><TD>\(.*\)</TD><TD>.*</TD><TD>\(.*\)</TD><TD>.*</TD><TD.*>.*</TD><TD.*>.*</TD><TD>.*</TD><td>.*</td>.*:\3:p') 
    if [ "$people" == "&nbsp;" ]; then
        people="N/A";
    else 
        people=$(echo $people | sed -n 's/<A.*>\(.*\)<\/A>/\1/p');
    fi
    if [ "$time" == "&nbsp;" ]; then
        time="N/A";
    else 
        time=$(echo $time | sed -n 's/[:<(].*//p');
    fi
    flag1=0
    for term in ${array[@]}
    do
        if [ "${course#*$term}" != "$course" ]; then flag1=1; fi
    done
    if (($flag==0 || ($flag==1&&$flag1==0) )); then
        printf "%-50s\t%-20s\t%-10s\n" "$course" "$people" "$time";
    fi
done    
echo $cnt

