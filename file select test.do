5 REM File select test
10 open "com:98n1e" for output as #1
20 d$= "ZZ"+chr$(0)+chr$(26) + "POOL  .BA" + space$(15) + "F" + chr$(0) + chr$(148)
30 print #1,d$;
