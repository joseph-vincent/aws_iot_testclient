#!/bin/sh
# stop script on error
set -e

if [ ! -z "$1" ]; then
    CFG_FILE=$1
else
    CFG_FILE=./agent.cfg
fi

ENDPOINT="as4qfk1dzdnka-ats.iot.us-east-1.amazonaws.com"
CA_CERT="/usr/certs/root-CA.crt"
CERT_PEM="/usr/certs/cert.pem"
PRIV_KEY="/usr/certs/private.key"
CLIENTID="uplincgateway_123456789_hpeap"
THINGNAME="uplincgateway_123456789_hpeap"
CUSTOMSTATSTOPIC=""

TOPIC="topic_1"
STATUSTOPIC="/test/agent/output"

COMPRESSION="lz4"
BINARY="/usr/apps/agentCpp"

if [ -f $CFG_FILE ]; then
	echo "Sourcing $CFG_FILE"
	. $CFG_FILE
fi
	echo
echo "ENDPOINT         : $ENDPOINT         " 
echo "CA_CERT          : $CA_CERT          " 
echo "CERT_PEM         : $CERT_PEM         " 
echo "PRIV_KEY         : $PRIV_KEY         " 
echo "COMPRESSION      : $COMPRESSION      "
echo "CLIENTID         : $CLIENTID         "
echo "THINGNAME        : $THINGNAME        "
echo "TOPIC            : $TOPIC            "
echo "STATUSTOPIC      : $STATUSTOPIC      "
echo "CUSTOMSTATSTOPIC : $CUSTOMSTATSTOPIC "

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

#ldd $BINARY

# run pub/sub sample app using certificates downloaded in package
printf "\nRunning pub/sub sample application...\n"
#./raw-pub-sub --endpoint a1a691bn20rvmp-ats.iot.us-east-1.amazonaws.com --ca-file root-CA.crt --cert MyFirstDevice.cert.pem --key MyFirstDevice.private.key --topic sdk/test/cpp 
echo "$BINARY --endpoint $ENDPOINT --ca-file $CA_CERT --cert $CERT_PEM --key $PRIV_KEY --statustopic $STATUSTOPIC --topic $TOPIC --customstatstopic "$CUSTOMSTATSTOPIC" --thingname $THINGNAME --clientid $CLIENTID --compress $COMPRESSION"
$BINARY --endpoint $ENDPOINT --ca-file $CA_CERT --cert $CERT_PEM --key $PRIV_KEY --statustopic $STATUSTOPIC --topic $TOPIC --customstatstopic "$CUSTOMSTATSTOPIC" --thingname $THINGNAME --clientid $CLIENTID --compress $COMPRESSION
