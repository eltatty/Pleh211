#!/bin/bash

# Attempt 1.

# Check for arguments (files).
if [[ "${#}" -eq 0 ]]
then
	echo "Usage: ${0} FILE_NAME [FILE_NAME]..."
	exit 1
fi

# Loop for every argument (file)
while [[ "${#}" -gt 0 ]]
do 
	# Initializations
	FILE=${1}
	COUNTER=0
	SUM_X=0
	SUM_Y=0
	SUM_XY=0
	SUM_X2=0
	ERR=0
	LENGTH=$(wc --lines < ${1})

	# Take the numbers from each line and put them into arrays. 
	while read LINE
	do
		X[$COUNTER]=$(echo ${LINE} | cut -d ':' -f 1)
		Y[$COUNTER]=$(echo ${LINE} | cut -d ':' -f 2)
		
		# Sum_x -->
		SUM_X=$(echo "${SUM_X} + ${X[$COUNTER]}" | bc -l )

		# Sum_y -->
		SUM_Y=$(echo "${SUM_Y} + ${Y[$COUNTER]}" | bc -l )
		
		# Sum_xy -->
		SUM_XY=$(echo "${SUM_XY} + ${X[$COUNTER]} * ${Y[$COUNTER]}" | bc -l )

		# Sum_x2 -->
		SUM_X2=$(echo "${SUM_X2} + ${X[$COUNTER]}^2" | bc -l )

		((COUNTER ++))
	done < ${FILE}

	# Check for stable vector.
	LMT=$(echo "${LENGTH} * ${SUM_X2} - ${SUM_X} * ${SUM_X}" | bc -l )
	
	if [[ "${LMT}" == 0 ]]
	then 
		echo 'Stable vector of X!'
	else
		# a -->
		A=$(echo "(${LENGTH} * ${SUM_XY} - ${SUM_X} * ${SUM_Y}) / ${LMT} " | bc -l )

		# b -->
		B=$(echo  "(${SUM_Y} - ${A} * ${SUM_X}) / ${LENGTH}" | bc -l )

		# Err -->
		for (( I=0; I<${LENGTH}; I++ ))
		do 		
			ERR=$(echo "${ERR} + (${Y[$I]} - (${A} * ${X[$I]} + ${B}))^2" | bc -l )
		done	
		
		#Debugging
		: '	
		echo "File: ${FILE}"
		echo "Length: ${LENGTH}"
		echo "Containts of X: ${X[@]}"
		echo "Containts of Y: ${Y[@]}"
		echo "Sum_x: ${SUM_X}"
		echo "Sum_y: ${SUM_Y}"
		echo "Sum_xy: ${SUM_XY}"
		echo "Sum_x2: ${SUM_X2}"
		echo "a is: ${A}"
		echo "b is: ${B}"
		#c is 1
		echo "err is: ${ERR}"
		'	

		# Final print.
		echo ${FILE} ${A} ${B} ${ERR} | awk '{printf "FILE:%s, a=%.2f b=%.2f c=1 err=%.2f\n", $1, $2, $3, $4}'
	fi
	shift
done
