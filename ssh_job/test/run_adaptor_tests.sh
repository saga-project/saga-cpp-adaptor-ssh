#!/bin/sh -x

# Copyright (c) 2009 Chris Miceli (cmicel1@cct.lsu.edu), Ashley Zebrowski (azebrowski@cct.lsu.edu)
# Distributed under the Boost Software License, Version 1.0. (See accompanying 
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Header

# Comments:  Added saga-job submit and state to Chris' saga-job run adaptor,
#            and changed the filename/format to match what Ole's using.  It's
#            tested and works with both localhost and qb.loni.org set as REMOTEHOST.
#            Suspend and resume are not tested for, as they are not implemented for the SSH adaptor.
#            Please let me know if this gives you any problems.  (Ashley, 11/20/09)

while getopts h arg
do
   case $arg in
      h) echo "This program tests the functionality of the ssh job adaptor."
         exit 0;;
   esac
done

if test "x$REMOTEHOST" = "x"; then
  REMOTEHOST=localhost
fi

EXIT_FAILURE=0
SUBMIT_FAILURE=0

# dummy temporary file location
FILE=tmp.saga.adaptor.ssh.test.$$


echo "======================================================================="


# test saga-job run
STRING=testing_ssh_job_run
COMMAND="/bin/sh -c \"echo $STRING > $FILE\""

echo              "saga-job run      ssh://$REMOTEHOST/ $COMMAND"
$SAGA_LOCATION/bin/saga-job run      ssh://$REMOTEHOST/ $COMMAND

COMMAND="/bin/sh -c \"cat $FILE\""
echo                      "saga-job run      ssh://$REMOTEHOST/ $COMMAND"
RESULT=`$SAGA_LOCATION/bin/saga-job run      ssh://$REMOTEHOST/ $COMMAND`

if test "$RESULT" = "$STRING"; then
  echo "OK"
else
  echo "Failed: '$RESULT' != '$STRING'"
  EXIT_FAILURE=1
fi


echo "======================================================================="
exit $EXIT_FAILURE;


# Test saga-job submit
STRING=testing_ssh_job_submit
COMMAND="echo $STRING > $FILE && sleep 60"

       echo              "saga-job submit   ssh://$REMOTEHOST/ $COMMAND"
JOBID=`$SAGA_LOCATION/bin/saga-job submit   ssh://$REMOTEHOST/ $COMMAND | head -1`

sleep 1 # wait for submitted job

COMMAND="cat $FILE"
echo                      "saga-job run      ssh://$REMOTEHOST/ $COMMAND"
RESULT=`$SAGA_LOCATION/bin/saga-job run      ssh://$REMOTEHOST/ $COMMAND`


if test "$RESULT" = "$STRING"; then
  echo "OK"
else
  echo "Failed: '$RESULT' != '$STRING'"
  SUBMIT_FAILURE=1
  EXIT_FAILURE=1
fi


echo "======================================================================="


# Test saga-job state (relies on saga-job submit succeeding, obviously)
if [ $SUBMIT_FAILURE = 1 ]; then
  echo "Omitting saga-job state testing after failed submit."
else
  # grab the job id from the previous submit command
  JOBID=`echo $JOBID | sed s/Job\ ID\ :\ //`
  
  echo                      "saga-job state ssh://$REMOTEHOST $JOBID"
  RESULT=`$SAGA_LOCATION/bin/saga-job state ssh://$REMOTEHOST $JOBID`

  if test "$RESULT" = "$JOBID: Running"; then
    echo "OK"
  else
    echo "Failed: '$RESULT' != '$JOBID: Running'"
    EXIT_FAILURE=1
  fi
fi


echo "======================================================================="


COMMAND="/bin/rm -f $FILE"

echo              "saga-job run      ssh://$REMOTEHOST/ $COMMAND"
$SAGA_LOCATION/bin/saga-job run      ssh://$REMOTEHOST/ $COMMAND && echo "OK" || echo "NOK"


echo "======================================================================="


if [ $EXIT_FAILURE = 1 ]
then
  echo "Some tests have failed"
  exit 1
else
  echo "all tests succeeded"
  exit 0
fi

