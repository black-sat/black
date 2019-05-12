#!/bin/bash

sed -e "s/~/\!/g" -e "s/=>/->/g" -e "s/True/TRUE/g" -e "s/False/FALSE/g" < $1
