#!/bin/bash

java -Xms128M -Xmx1G -cp '.:*' -jar repeater/repeater.jar "${@}"
