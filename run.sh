#!/bin/bash

Xaxis=$(xrandr --current | grep '*' | uniq | awk '{print $1}' | cut -d 'x' -f1)


gnome-terminal --geometry 100x30+50+100    -x sh -c "./server 4242; printf \"\nProcesso terminato, verrà chiuso tra 5 secondi...\"; sleep 100;"
gnome-terminal --geometry 100x30+1050+100  -x sh -c "./client 1234; printf \"\nProcesso terminato, verrà chiuso tra 5 secondi...\"; sleep 5;"
gnome-terminal --geometry 100x30+50+800    -x sh -c "./client 1235; printf \"\nProcesso terminato, verrà chiuso tra 5 secondi...\"; sleep 5;"
gnome-terminal --geometry 100x30+1050+800  -x sh -c "./client 1236; printf \"\nProcesso terminato, verrà chiuso tra 5 secondi...\"; sleep 5;"
