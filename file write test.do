10 rem File write test
20 open "com:98n1e" for output as #1
30 print "set reference"
40 d$= "ZZ"+chr$(0)+chr$(26) + "WRITE .DO" + space$(15) + "F" + chr$(0) + chr$(83)
50 print #1,d$;
60 input x$
70 print "open it"
80 d$="ZZ"+chr$(1)+chr$(1) + chr$(1) + chr$(252) 
90 print #1,d$;
100 input x$
110 print "write a record"
120 d$="ZZ"+chr$(4)+chr$(39)+ "1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ" +chr(10)+chr(13) + chr$(177)
130 print #1,d$;
140 input x$
150 print "write another record"
160 d$="ZZ"+chr$(4)+chr$(34) + "Now is the time for all good men" +chr(10)+chr(13) + chr$(121)
170 print #1,d$;
180 input x$
190 print "close it"
200 d$="ZZ"+chr$(2)+chr$(0) + chr$(253)
210 print #1,d$;
220 input x$