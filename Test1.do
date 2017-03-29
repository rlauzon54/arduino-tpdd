5 maxfiles=2
10 open "com:88N1D" for output as #1
20 open "com:88N1D" for input as #2
30 read l
40 if l = 0 then stop
50 d$ = ""
60 for i = 1 to l
70 read c
80 d$ = d$+chr$(c)
90 next
100 print #1,d$;
110 c$=input$(1,2)
120 read c
130 if asc(c$) <> c then print "bad return type:"; asc(c$): stop
140 c$=input$(1,2)
150 read l
160 if asc(c$) <> l then print "bad length:";asc(c$): stop
170 f$=""
180 for i=1 to l
190 c$=input$(1,2)
200 read c
220 if asc(c$) <> c then print "Bad data:"; asc(c$): stop
230 next
240 goto 40
250 close 1
260 close 2
500 Data 8
510 DATA 4D 31 0D 5A 5A 08 00 F7
520 DATA 12 0B 00 52 4F 4F 54 20 20 2E 3C 3E 20 96
530 DATA 10
540 DATA 0D 0D 4D 31 0D 5A 5A 07 00 F8
550 DATA 12 01 00 EC
560 DATA 8
570 DATA 4D 31 0D 5A 5A 08 00 F7
580 DATA 12 0B 00 52 4F 4F 54 20 20 2E 3C 3E 20 96
590 DATA 10
600 DATA 0D 0D 4D 31 0D 5A 5A 07 00 F8
610 DATA 12 01 00 EC
620 DATA 5
630 DATA 5A 5A 08 00 F7
640 DATA 12 0B 00 52 4F 4F 54 20 20 2E 3C 3E 20 96    
650 DATA 31
660 DATA 5A 5A 00 1A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 46 01 9E
670 DATA 11 1C 41 44 56 45 4E 31 2E 42 41 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 46 45 D3 28 1C
680 DATA 31
690 DATA 5A 5A 00 1A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 F6 02 ED 
700 DATA 11 1C 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 28 AA
710 DATA 0   
