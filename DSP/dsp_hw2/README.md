DSP HW2-1: HMM Training and Testing
====================================================

By Fan-Yun Sun(b04902045)

## Runtime Environment

- System: Ubuntu 16.04
- Compiler: gcc version 5.4.0

## Change on the number of states

number of states = 5

![](screenshots/state5.png)

number of states = 10

![](screenshots/state10.png)

number of states = 15

![](screenshots/state15.png)

Increasing the # of states does increase the accuracy dramatically.

## Increase the number of Gaussian Mixtures

![](screenshots/exp1.png)

This experiment is conducted by changing the value of x. Note that all other parameters are fixed in this experiment.

x = 1

![](screenshots/gmm+1.png)

x = 3

![](screenshots/gmm+3.png)

x = 8

![](screenshots/gmm+8.png)

x = 11

![](screenshots/gmm+11.png)

x = 20

![](screenshots/gmm+20.png)

If we fix the number of iterations, increasing the number of gmm can improve the accuracy, but only to a certain extent.
Improvement is very limited.

## Change on the number of iterations

![](screenshots/exp2.png)

I conducted experiments with different number of iterations. Note that all three parameters shown in the graph are all changed to the same number. All other parameters are fixed in this experiment.

Iteration: 3

![](screenshots/iter3.png)

Iteration: 10

![](screenshots/iter10.png)

Iteration: 50

![](screenshots/iter50.png)

Iteration: 100

![](screenshots/iter100.png)

Increasing the iteration does help. But too many iterations may have negative impact.


## Final Results

I changed three files. 

- `lib/proto`: # state -> 15
- `lib/mix2_10.hed`: # of gmm -> 2 +20 (refer to the first figure, x=20)
- `03_training.sh`: # of iterations -> 50

The final result is shown as below.

![](screenshots/result.png)
