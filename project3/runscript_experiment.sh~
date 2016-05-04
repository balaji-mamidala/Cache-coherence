#!/bin/bash

cd protocols/
make
cd ..
make
echo ""
echo ""

Protocol=("MSI" "MESI" "MOSI" "MOESI" "MOESIF")

E=1
while [ $E -le 8 ]; do
	P=0
	while [ $P -lt 5 ]; do
		rm experiment_output/${Protocol[$P]}"_experiment"$E.txt

		./sim_trace -t traces/"experiment"$E -p ${Protocol[$P]} 2>> experiment_output/${Protocol[$P]}"_experiment"$E.txt
		echo ${Protocol[$P]}"_experiment"$E" generated"	
		
		echo "$E"	>> result_exp_no.xls
		
		echo "${Protocol[$P]}"	>> result_name.xls  
		
		grep -i "Run Time" experiment_output/${Protocol[$P]}"_experiment"$E.txt | awk -F\: '{print$1 $2}' | awk  '{print $3}' OFS=':' >> result_run.xls		
		
		grep -i "Cache Misses" experiment_output/${Protocol[$P]}"_experiment"$E.txt | awk -F\: '{print$1 $2}' | awk  '{print $3}' OFS=':' >> result_cache_miss.xls		
		
		grep -i "Cache Accesses" experiment_output/${Protocol[$P]}"_experiment"$E.txt | awk -F\: '{print$1 $2}' | awk  '{print $3}' OFS=':' >> result_cache_acc.xls		
		
		grep -i "Silent Upgrades" experiment_output/${Protocol[$P]}"_experiment"$E.txt | awk -F\: '{print$1 $2}' | awk  '{print $3}' OFS=':' >> result_silent.xls		
		
		grep -i "Transfers" experiment_output/${Protocol[$P]}"_experiment"$E.txt | awk -F\: '{print$1 $2}' | awk  '{print $3}' OFS=':' >> result_transfer.xls		
		
		P=$((P + 1))  
	done;
	E=$((E + 1)) 
done;

echo "Experiment#:Protocol:Run_Time:Cache_Misses:Cache_Accesses:Silent_upgrade:Transfers"	> experiment_output/result.xls
paste result_exp_no.xls result_name.xls result_run.xls result_cache_miss.xls result_cache_acc.xls result_silent.xls result_transfer.xls > final_result.xls

cat final_result.xls >> experiment_output/result.xls

rm final_result.xls result_exp_no.xls result_name.xls result_run.xls result_cache_miss.xls result_cache_acc.xls result_silent.xls result_transfer.xls

diff -iw experiment_output/result.xls Ref_result.xls > experiment_output/experiment_comparision.log
echo ""

if [[ -s experiment_output/experiment_comparision.log ]] ; then
	echo "Results do not match"
else
	echo "Results match"
	rm experiment_output/experiment_comparision.log
fi
	
echo ""
