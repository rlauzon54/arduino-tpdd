5 maxfiles=2
10 open "com:88N1D" for output as #1
20 open "com:88N1D" for input as #2
30 read l
40 if l = 0 then stop
50 d$ = ""
60 for i = 1 to l: read c:d$ = d$+chr$(c): next
70 print #1,d$;
80 c$=input$(1,2)
90 read c
100 if asc(c$) <> c then print "bad return type:"; asc(c$); " "; c: stop
110 c$=input$(1,2)
120 read l
130 if asc(c$) <> l then print "bad length:";asc(c$); " " ; l: stop
140 for i=1 to l
150 c$=input$(1,2)
160 read c
170 if asc(c$) <> c then print "Bad data:"; i ; " " ; asc(c$) ; " " c: stop
180 next
190 c$=input$(1,2)
200 read c
210 if asc(c$) <> c then print "Bad checksum:"; asc(c$) ; " " c: stop
240 goto 40
250 close 1
260 close 2
500 Data 8
510 DATA 77 49 13 90 90 08 00 247
520 DATA 18 11 00 82 79 79 84 32 32 46 60 62 32 150
530 DATA 10
540 DATA 13 13 77 49 13 90 90 07 00 248
550 DATA 18 01 00 236
560 DATA 8
570 DATA 77 49 13 90 90 08 00 247
580 DATA 18 11 00 82 79 79 84 32 32 46 60 62 32 150
590 DATA 10
600 DATA 13 13 77 49 13 90 90 07 00 248
610 DATA 18 01 00 236
620 DATA 5
630 DATA 90 90 08 00 247
640 DATA 18 11 00 82 79 79 84 32 32 46 60 62 32 150
650 DATA 31
660 DATA 90 90 00 26 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 70 01 158
670 DATA 17 28 65 68 86 69 78 49 46 66 65 32 32 32 32 32 32 32 32 32 32 32 32 32 32 32 70 69 211 40 28
680 DATA 31
690 DATA 90 90 00 26 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 246 02 237
700 DATA 17 28 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 170
710 DATA 0   
