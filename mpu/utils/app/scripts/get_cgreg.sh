#!/bin/bash
retvalue=$(ql_cmd  at at+cgreg? | grep CGREG | awk -F, '{print $2}' | tr -d ' ')
num=$(( retvalue + 0 ))
exit $num
