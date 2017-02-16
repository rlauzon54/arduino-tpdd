10 rem Directory list test
20 open "com:98n1e" for output as #1
30 rem get first
40 d$="ZZ"+chr$(0)+chr$(26) + space$(24) + "F" + chr$(1) + chr$(158)
50 print #1,d$;
60 input x$
70 rem get next a few times
80 for i = 1 to 10
90 d$="ZZ"+chr$(0)+chr$(26) + space$(24) + "F" + chr$(2) + chr$(157)
100 print #1,d$;
110 input x$
120 next
130 rem get previous some times
140 for i = 1 to 5
150 d$="ZZ"+chr$(0)+chr$(26) + space$(24) + "F" + chr$(3) + chr$(156)
160 print #1,d$;
170 input x$
180 next
190 rem end directory
200 d$="ZZ"+chr$(0)+chr$(26) + space$(24) + "F" + chr$(4) + chr$(155)
210 print #1,d$;
220 input x$
