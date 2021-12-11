# Projet Prog Repartie

## Our project in a few words
Distributed algorithms can be used for a broad range of fields: telecommunications and network, AI, real-time applications, etc.
The aim of our project, thought by our professor Mr.Giroudeau and Ms.Bouziane, is to implement an algorithm which realizes mutual exclusion. For this purpose, we have chosen the one of **Naïmi-Tréhel**[^1]. We then had to model messages exchange so have chosen to use various linux console.

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


## Conclusion

## Collaborators
* Maitre Hugo
* Wang Yuyang
* Linares Adrien


[^1]: https://fr.wikipedia.org/wiki/Algorithme_de_Naimi-Trehel
[^2]: https://en.wikipedia.org/wiki/Pthreads
[^3]: https://www.ibm.com/docs/en/i/7.1?topic=communications-socket-programming
