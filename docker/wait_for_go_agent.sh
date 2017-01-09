#!/bin/bash

i=0
timeout=$1

while true ; do
  echo -e -n "\r$i"
  if [ $i -eq $timeout ] ; then
    exit 1
  fi
  grep "Successfully registered agent" /var/log/go-agent/go-agent.log &> /dev/null
  status=$?    
  if [ $status -eq 0 -a $i -ge 10 ] ; then   # wait a min 10s
    exit 0
  fi
  sleep 1
  i=$((i + 1))
done
