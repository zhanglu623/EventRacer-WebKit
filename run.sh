#!/bin/sh
rm -rf races
mkdir races
bin/eventracer/webapp/raceanalyzer ../webkit/ER_actionlog $1
