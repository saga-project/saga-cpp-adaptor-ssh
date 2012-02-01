#!/bin/sh

# Copyright (c) 2009 Chris Miceli (cmicel1@cct.lsu.edu)
# Distributed under the Boost Software License, Version 1.0. (See accompanying 
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Header
while getopts h arg
do
   case $arg in
      h) echo "This program tests the functionality of the ssh file adaptor."
         exit 0;;
   esac
done

if test "x$REMOTEHOST" = "x"; then
  REMOTEHOST=localhost
fi

EXIT_FAILURE=0

# Create a dummy temporary file location
FILE=/tmp/saga.adaptor.ssh.test.$$

# Give the file some contents
STRING=saga_was_here
`echo "$STRING" > $FILE.src`

echo "Running: saga-file copy file://localhost/$FILE.src ssh://$REMOTEHOST/$FILE.tgt"
RESULT=`$SAGA_LOCATION/bin/saga-file copy file://localhost/$FILE.src ssh://$REMOTEHOST/$FILE.tgt`
if test "$RESULT" != ""
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

echo "Running: saga-file cat ssh://$REMOTEHOST/$FILE.tgt "
RESULT=`$SAGA_LOCATION/bin/saga-file cat ssh://$REMOTEHOST/$FILE.tgt`
if test "$RESULT" != "$STRING"
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

echo "Running: saga-file get_size ssh://$REMOTEHOST/$FILE.tgt"
RESULT=`$SAGA_LOCATION/bin/saga-file get_size ssh://$REMOTEHOST/$FILE.tgt`
if test "$RESULT" != "ssh://$REMOTEHOST/$FILE.tgt : 14 bytes."
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

echo "Running: saga-file move ssh://$REMOTEHOST/$FILE.tgt ssh://$REMOTEHOST/$FILE.mv"
RESULT=`$SAGA_LOCATION/bin/saga-file move ssh://$REMOTEHOST/$FILE.tgt ssh://$REMOTEHOST/$FILE.mv`
if test "$RESULT" != ""
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

echo "Running: saga-file is_link ssh://$REMOTEHOST/$FILE.mv"
RESULT=`$SAGA_LOCATION/bin/saga-file is_link ssh://$REMOTEHOST/$FILE.mv`
if test "$RESULT" != "False"
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

echo "Running: saga-file link ssh://$REMOTEHOST/$mv ssh://$REMOTEHOST/$FILE.lnk"
RESULT=`$SAGA_LOCATION/bin/saga-file link ssh://$REMOTEHOST/$FILE.mv ssh://$REMOTEHOST/$FILE.lnk`
if test "$RESULT" != ""
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

echo "Running: saga-file is_link ssh://$REMOTEHOST/$FILE.lnk"
RESULT=`$SAGA_LOCATION/bin/saga-file is_link ssh://$REMOTEHOST/$FILE.lnk`
if test "$RESULT" != "True"
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

echo "Running: saga-file remove ssh://$REMOTEHOST/$FILE.tgt"
RESULT=`$SAGA_LOCATION/bin/saga-file remove ssh://$REMOTEHOST/$FILE.mv`
if test "$RESULT" != ""
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

#echo "Running: saga-file move ssh://$REMOTEHOST/$FILE.tgt ssh://$REMOTEHOST/$FILE.mv"
#RESULT=`$SAGA_LOCATION/bin/saga-file move ssh://$REMOTEHOST/$FILE.tgt ssh://$REMOTEHOST/$FILE.mv`
#if test "$RESULT" != ""
#then
#   echo "Failed: $RESULT"
#   exit 1
#fi

if [ $EXIT_FAILURE = 1 ]
then
   echo "Some tests have failed!"
   exit 1
else
   exit 0
fi

rm -f $FILE.src
rm -f $FILE.mv
unlink $FILE.lnk

# Verify the cat exited what was expected

