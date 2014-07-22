#Shell Project

This project focuses on creating a shell that reads in commands, line by line, from standard input, executes those commands, and writes the output to standard output.

##Features

Execute basic commands (support flags):

```
  ls
  
  ls -l
  
  pwd
```

###Redirections

```
  ls > out.txt
  
  ls 1> out.txt
  
  ls 2> error.txt
  
  kill -9 < procId.txt
```
  
###Piped Commands

```
  ls | wc
```  

###Future Features

- more error handling.
  
- **cd** command.
  
  
