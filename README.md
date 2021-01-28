# RealtionalDB
Rudimentary Implementation of my own relational database in cpp
Commands include CREATE, INSERT INTO, REMOVE, QUIT, DELETE, DELETE FROM...WHERE, PRINT, PRINT FROM...WHERE
ex)
CREATE t1 5 string int string bool double name age dob single? gpa 
INSERT INTO t1 4 ROWS
bob 47 08/01/1985 true 4.5
cob 10 09/21/1781 true 3.5
dob 10 18/01/1292 false 2.5
eob 47 04/26/1182 false 1.5
CREATE t2 2 int bool age happy?
INSERT INTO t2 4 ROWS
10 true
9 false
11 true
3 false
PRINT FROM t1 2 single? name WHERE gpa = 1.5
JOIN t1 AND t2 WHERE single? = happy? AND PRINT 3 name 1 age 2 gpa 1
DELETE FROM t2 WHERE happy? = true
JOIN t1 AND t2 WHERE single? = happy? AND PRINT 3 name 1 age 2 gpa 1
REMOVE t2
REMOVE t1
#peace out
QUIT
