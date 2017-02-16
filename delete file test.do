10 rem File write test
20 open "com:98n1e" for output as #1
30 print "set reference"
40 d$= "ZZ"+chr$(0)+chr$(26) + "WRITE .DO" + space$(15) + "F" + chr$(0) + chr$(83)
50 print #1,d$;
60 input x$
70 print "delete it"
80 d$="ZZ"+chr$(5)+chr$(0) + chr$(250)
90 print #1,d$;
100 input x$
