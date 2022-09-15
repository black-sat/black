#!/bin/bash

sed -e "s/~/not/g" -e "s/!/not/g" -e "s/X/next/g" -e "s/F/sometime/g" \
    -e "s/G/always/g" -e "s/U/until/g" < $1 | translate
