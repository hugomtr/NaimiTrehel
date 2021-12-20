# Projet Prog Repartie

## Our project in a few words
Distributed algorithms can be used for a broad range of fields: telecommunications and network, AI, real-time applications, etc.
The aim of our project, thought by our professor M.Giroudeau and Ms.Bouziane, is to implement an algorithm which realizes mutual exclusion. For this purpose, we have chosen the one of **Naïmi-Tréhel**[^1]. We then had to model messages exchange so have chosen to use xterms. You will be able to find, in addition to our code, our report explaining our choice of implementation, architecture or our work organization.

## Naïmi-Tréhel
Naïmi-Tréhel is a distributed algorithm of mutual exclusion which provides a synchronized access to shared resource ensuring at all times that only one site accesses the critical section. It works with two data structure : </br> </br> 

- **Next Queue** : A distributed queue containing the set of sites waiting for the critical section (CS). The site which owns the token is the beginning of this queue and is, if no query has been sent, the only member of the queue.
- **__Last__** : A logical and dynamical tree which is used to route requests in CS. Last indicates the last site which made a query. </br></br>
  
This algorithm reduces the number of messages by **O(log(n))**.

## Technologies used
During this project, we have used mainly C++ language with some primitives of C.
We used some concepts seen in class as thread programming with POSIX threads[^2] and socket programming[^3].
Our IDE was Visual Studio Code.

## How to test our project
1. Compile the main file using gcc or g++ ``` gcc main.c -o exec ```
2. Before starting running the project, we want you to know that the xterm version of the project can be used by 2 differents ways: 
With 2 parameters : ```./exec id_site number_executions_site``` or with 4 parameters : ```./exec starting_port_nb id_site nb_exec_CS nb_sites```
3. Open as many console as sites you decided (sites decided by modifying the script or through the parameters)
4. Site 0 is considered as the one having the token. Thus, it is waiting for requests. Repeat the step **2** changing the correct parameters.
5. When you see ``` Enter a value if all site has be launched ```, enter one value on at least one of the sites that is different of the site 0.
6. Once this step finished, enter the value on the site 0, this value can be whatever you want, a space or something else, it won't have any impact on the algorithm

## Conclusion
Doing this project taught us much more about how distributed systems work and what they really are, in practise. The Naïmi-Tréhel algorithm is a great one to start seeing those things. We thank our teachers who introduced us the concepts of network, parallel computing and distributed algorithms, and hope we made good use of them through this project.

## Collaborators
* Maitre Hugo
* Wang Yuyang
* Linares Adrien


[^1]: https://fr.wikipedia.org/wiki/Algorithme_de_Naimi-Trehel
[^2]: https://en.wikipedia.org/wiki/Pthreads
[^3]: https://www.ibm.com/docs/en/i/7.1?topic=communications-socket-programming
