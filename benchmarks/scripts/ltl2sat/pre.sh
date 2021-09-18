#!/bin/bash

sed -e "s/~/\!/g" -e "s/=>/->/g" -e "s/True/TRUE/g" -e "s/False/FALSE/g" -e "s/wX/N/g" < $1
