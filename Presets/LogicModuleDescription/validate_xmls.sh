#!/bin/bash

for filename in ./*.xml; do
	echo $filename
	xmllint --noout $filename
	if [ $? == 0 ]; then
	  echo "Success!"
	else
	  echo "Error!"
	  exit 1
	fi
done

exit 0
