5 REM File select test
6 maxfiles=2
10 open "com:98n1e" for output as #1
20 open "com:98N1E" for input as #2
30 d$= "ZZ"+chr$(0)+chr$(26) + "POOL  .BA" + space$(15) + "F" + chr$(0) + chr$(148)
40 print #1,d$;
45 close 1
46 rem for some reason the first char isn't there.  I don't know why
50 rem c$=input$(1,2)
60 rem if asc(c$) <> 17 then print "bad return type:"; asc(c$): stop
70 c$=input$(1,2)
80 if asc(c$) <> 28 then print "bad length:";asc(c$): stop
90 f$=""
100 for i=0 to 25
110 c$=input$(1,2)
120 f$=f$+c$: print f$
130 next
140 c$=input$(1,2)
150 if c$ <> "F" then print "Bad attribute:";c$: stop
160 c$=input$(1,2)
170 s=asc(c$)*16
180 c$=input$(1,2)
190 s=s+asc(c$)
200 print "Size:";s
210 c$=input$(1,2)
220 print "Sectors free:"; asc(c$)
230 c$=input$(1,2)
240 print "checksum="; asc(c$)
