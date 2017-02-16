10 rem Directory list test
20 open "com:98n1e" for output as #1
30 print "get first"
40 d$="ZZ"+chr$(0)+chr$(26) + space$(24) + "F" + chr$(1) + chr$(158)
50 print #1,d$;
60 input x$
70 print "get next"
80 for i = 1 to 10
85 print "next ";i
90 d$="ZZ"+chr$(0)+chr$(26) + space$(24) + "F" + chr$(2) + chr$(157)
100 print #1,d$;
110 input x$
120 next
130 print "get previous"
140 for i = 1 to 5
145 print "Prev: ";i
150 d$="ZZ"+chr$(0)+chr$(26) + space$(24) + "F" + chr$(3) + chr$(156)
160 print #1,d$;
170 input x$
180 next
190 print "end directory"
200 d$="ZZ"+chr$(0)+chr$(26) + space$(24) + "F" + chr$(4) + chr$(155)
210 print #1,d$;
220 input x$
