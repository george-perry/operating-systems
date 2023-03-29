George Perry - gperry@nd.edu
Duncan Park - dpark6@nd.edu

In order to get rid of the warnings concerning strdup we followed this SO page:
https://stackoverflow.com/questions/26284110/strdup-confused-about-warnings-implicit-declaration-makes-pointer-with

Results (using 10 producers, 1000 iterations)

Without Condition variables

1 Consumer results (seconds):
14.9, 15.8, 15.1

2 Consumer results (seconds):
15.6, 14.9,  15.1

3 Consumer results (seconds):
15.2, 14.9, 15.1

4 Consumer results (seconds):
15.7, 16.3, 16.3

5 Consumer results (seconds):
16, 16.3, 16.3

6 Consumer results (seconds):
15.9, 15.9, 16.6

7 Consumer results (seconds):
17.1, 16.8, 17,2

8 Consumer results (seconds):
17.7, 17.1, 17.2

9 Consumer results (seconds):
17.3, 17.3, 17.2

10 Consumer results (seconds):
17.5, 17.4, 17.5


Wit Condition variables

1 Consumer results (seconds):
14.7, 15.6, 15.3

2 Consumer results (seconds):
14.8, 14.3, 15.4

3 Consumer results (seconds):
15.1, 15, 14.7

4 Consumer results (seconds):
14.7, 15.8, 14.9

5 Consumer results (seconds):
15.3, 14.8, 15.3

6 Consumer results (seconds):
15.3, 14.6, 14.7

7 Consumer results (seconds):
14.6, 15.2, 14.9

8 Consumer results (seconds):
15.3, 15.2, 15.3

9 Consumer results (seconds):
15.5, 15.1, 14.9

10 Consumer results (seconds):
15.8, 15.7, 15.4

