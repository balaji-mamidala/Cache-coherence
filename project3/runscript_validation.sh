#!/bin/bash

cd protocols/
make
cd ..
make
echo ""
echo ""

Protocol=("MSI" "MESI" "MOSI" "MOESI" "MOESIF")
Pend=0
Pstart=0

if [[ $1 -ge 1 && $1 -le 5 && $2 -ge 1 && $2 -le 5 && $2 -ge $1 ]]; then
	Pstart=$1
	Pend=$2
else
	Pend=5
	Pstart=1	
fi

P=$((Pstart-1))
while [ $P -lt $Pend ]; do
	I=4
	while [ $I -le 16 ]; do
		J=${Protocol[$P]}$I	
		
		./sim_trace -t traces/$I"proc_validation" -p ${Protocol[$P]} 2> validation_output/$J.txt
		echo $J" generated"

		diff -iw traces/$I"proc_validation/"${Protocol[$P]}"_validation.txt" "validation_output/"$J.txt > validation_output/$J"_comparision.log"

		if [[ -s validation_output/$J"_comparision.log" ]] ; then
			echo "Test not successful"
		else
			echo "Test succesful"
			rm validation_output/$J"_comparision.log"
		fi
	
		echo ""
		
		I=$((I * 2)) 					
	done;	 			
	P=$((P + 1))  
done;
