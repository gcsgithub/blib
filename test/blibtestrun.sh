#!/bin/ksh

OS="$( uname -s )"
if [ "${OS}" =  "Darwin" ]
then
	pg="open"
else
	pg="cat"
fi

export BLIB="../src/blib"
export BLIBDBS="./blib_test.sqlite3"
export BLIB_LOG="./blib_CENTS7.log"

typeset -u MYBLIB_GROUP=$( uname -n)
export BLIB_MEDIA="ULTRIUM5"

eval $( ${BLIB} /env )

typeset -i idx=0

LABEL=""

# clear the libary and log for this test
rm -f "${BLIBDBS}"
rm -f "${BLIBDBS%%.sqlite3}.log"

# Add 5 tapes to the library
while [ idx -le 5 ]
do
	label=$( printf "TST%03d" idx )
	eval $( ${BLIB} /add="${label}" /media="${BLIB_MEDIA}" /group="${MYBLIB_GROUP}" )
	idx=idx+1
done


###########################################
function findfreetape # {MEDIA} {count} {rval}
###########################################
{
# not to the casual shell programmer typeset makes these local scope, -n is a named reference
typeset    media="${1}"
typeset -i count=${2}
typeset -n rval="${3}"

	rval=$( blib -q /report /media=${media} /state=FREE | head -${count} | awk '{print $1}' )
	if [ "${rval}" = "" ]
	then
		print -u2 "# Error no free tapes"
		exit 2
	else
		print -u2 "# Found tape ${rval}"
	fi
}


findfreetape ULTRIUM5 1 LABEL

echo "# Start a new backup getting BLIB_BCKID as the reference"
eval $(  ${BLIB} /newbackup /desc="New backup test" /record=$(caltime 0) /expire=$(caltime +7) )
sleep 2
##################################################################################################################

echo "# record a backup of /"
eval $(  ${BLIB} /startbackup="/" /bck_id=${BLIB_BCKID} /label=${LABEL} /record=$(caltime 0) )
sleep 2
${BLIB} /endbackup="/" /bck_id=${BLIB_BCKID} /objinstance=${BLIB_VOLID} /label=${LABEL} /end=$(caltime 0) /size=$RANDOM
sleep 2

echo "# do a second backup of / to see a new objinstance"
eval $(  ${BLIB} /startbackup="/" /bck_id=${BLIB_BCKID} /label=${LABEL} /record=$(caltime 0) )
sleep 2
${BLIB} /endbackup="/" /bck_id=${BLIB_BCKID} /objinstance=${BLIB_VOLID} /label=${LABEL} /end=$(caltime 0) /size=$RANDOM
sleep 2
##############################

echo "# start a backup of /usr"
eval $( ${BLIB} /startbackup="/usr" /bck_id=${BLIB_BCKID} /label=${LABEL} /record=$(caltime 0) )
sleep 2

echo "# get a new tape and continue the backup of /usr"
findfreetape ULTRIUM5 1 LABEL
${BLIB} /change_volume="/usr" /bck_id=${BLIB_BCKID} /objinstance=${BLIB_VOLID} /label=${LABEL} /end=$(caltime 0) /size=$RANDOM
sleep 2

echo "# get yet another new tape and continue the backup of /usr"
findfreetape ULTRIUM5 1 LABEL
${BLIB} /change_volume="/usr" /bck_id=${BLIB_BCKID} /objinstance=${BLIB_VOLID} /label=${LABEL} /end=$(caltime 0) /size=$RANDOM
sleep 2

echo "# make the backup of /usr as finished"
${BLIB} /endbackup="/usr" /bck_id=${BLIB_BCKID} /objinstance=${BLIB_VOLID} /label=${LABEL} /end=$(caltime 0) /size=$RANDOM
sleep 2


echo "# complete the backup id"
${BLIB} /finishbackup /bck_id=${BLIB_BCKID}


echo "# report on the backup"
${BLIB} /report
${BLIB} /listbackups
${BLIB} /reportbackup=${BLIB_BCKID}
${BLIB} /reportbackup=${BLIB_BCKID} /html /output=blibtestrun.html
${BLIB} -v /verify
${pg} ./blibtestrun.html
