#!/bin/bash

#										  3000 Bytes
cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w 3000 | head -n 1
