# GetSecondNewestFileName

This is a quick example how to use std::map with struct as key type.

I wanted to use **std::map<struct timespec, std::string>** but this wasn't obvious. 
Using struct as key requires implementing **operator<** which is not present in original **timespec**. 
Simple inheriting of 
>struct mytimespec:public timespec

and defining **operator<** wasn't enough. Note **const** modifier in **operator<** definition.
