#!/bin/bash

# This script creates scores for extracting results.

# Check for arguments.
if [[ "${#}" -ne 2 ]]
then
	echo "Usage: ${0} FILE_NAME NUM_OF_MATCHES"
	exit 1
fi

# Initialize array teams, scores.
TEAMS=(Spain Greece Portugal Italy France England Germany Poland Brazil Argentina Uruguay China Japan Australia Netherlands Wales Chile)
SCORES=(0 1 2 3 4 5)

# File to create.
FILE=${1}

# Number of matches played.
NUM=${2}

# File creation 
for (( I=0; I<${NUM}; I++ ))
do
	TEAM1=${TEAMS[${RANDOM} % ${#TEAMS[@]}]}
	TEAM2=${TEAMS[${RANDOM} % ${#TEAMS[@]}]}	
	while [[ "${TEAM1}" == "${TEAM2}" ]] 
	do
		TEAM2=${TEAMS[${RANDOM} % ${#TEAMS[@]}]}
	done
	SCORE1=${SCORES[${RANDOM} % ${#SCORES[@]}]}
	SCORE2=${SCORES[${RANDOM} % ${#SCORES[@]}]}
	echo "${TEAM1}-${TEAM2}:${SCORE1}-${SCORE2}" >> ${FILE}
done	

