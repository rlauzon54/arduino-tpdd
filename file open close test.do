10 rem File open, close test
20 open "com:98n1e" for output as #1
30 print set reference
40 d$= "ZZ"+chr$(0)+chr$(26) + "POOL  .BA" + space$(15) + "F" + chr$(0) + chr$(148)
50 print #1,d$;
60 input x$
70 print open it
80 d$="ZZ"+chr$(1)+chr$(1) + chr$(3) + chr$(250)
90 print #1,d$;
100 input x$
110 print close it
120 d$="ZZ"+chr$(2)+chr$(0) + chr$(253)
130 print #1,d$;
140 input x$
