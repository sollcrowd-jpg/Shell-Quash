Overview

The Shell-Quash project is a simplified implementation of a Unix-based command-line shell. The goal of this project is to replicate key functionalities of standard shells such as bash and sh, giving the user the ability to execute both built-in and external commands. Through this project, we aim to deepen our understanding of system-level programming, focusing on topics like process forking, signal handling, and input/output (I/O) redirection.

This shell, implemented in C, enables users to navigate through directories, run commands in the foreground and background, manipulate environment variables, and use redirection to handle files. It also introduces error handling and timeout mechanisms for long-running processes, improving its usability and robustness.

Design Choices

1. Shell Prompt Design
One of the first considerations in designing the shell was how to present the command prompt to the user. We chose to follow the convention used by many Unix shells, displaying the current working directory in the prompt. This helps users maintain context as they navigate through different directories.

The shell uses the getcwd() function to retrieve the current directory, which is then displayed as:

bash
Copy code
/current/directory/path> 
For example, if the user is in /home/codio/workspace, the prompt would look like:

arduino
Copy code
/home/codio/workspace>
The prompt is updated dynamically after every command that changes the directory, ensuring that the user always has relevant context.

2. Command Tokenization and Parsing
A crucial part of the shell is parsing the user’s input. We used the strtok() function to split user input into individual tokens. The first token represents the command, while subsequent tokens represent the arguments or flags. We also ensured that the shell could handle commands like:

bash
Copy code
echo Hello World
Here, echo is the command, and Hello World are the arguments.

Additionally, the shell supports environment variable expansion. For instance, the user can type echo $HOME, and the shell will replace $HOME with the actual value of the environment variable by using the getenv() function.

3. Built-in Commands
Several built-in commands were implemented, as these are crucial for shell functionality. We chose to implement commands like cd, pwd, echo, env, setenv, and exit. These commands are handled directly within the shell (unlike external commands, which use process forking).

cd <directory>: This command changes the current working directory using the chdir() function. If the directory does not exist or the user does not have permission, an error message is displayed. If no argument is provided, the command prints an error asking for a valid directory.
pwd: The pwd command prints the current working directory using getcwd(). This is useful for confirming the user's location in the file system.
echo <message>: This command prints the provided message. It supports environment variable expansion, allowing users to type commands like echo $PATH, which will print the value of the PATH variable.
setenv <variable> <value>: This command sets environment variables using setenv(). We chose to allow the user to overwrite existing environment variables by default, which simplifies the behavior.
env: This command prints all the current environment variables using the global environ array.
exit: This command exits the shell by calling exit().
4. External Command Execution
External commands such as ls, cat, and others that are not built into the shell are executed by forking a new process. This was achieved using the fork() and execvp() system calls. Here’s how the process works:

The shell checks whether the command is a built-in command.
If it's not a built-in command, the shell forks a child process using fork().
In the child process, the execvp() function replaces the process image with the command that the user entered.
The parent process waits for the child process to finish using wait(), unless the command is run in the background (with &), in which case the shell immediately returns control to the user.
This design choice ensures that Quash can run all standard Unix commands seamlessly, whether in the foreground or background.

5. Background Process Management
The ability to run background processes is a key feature of modern shells. Quash supports this by allowing users to append an ampersand (&) to their commands. This tells the shell to run the command in the background, allowing the user to continue using the shell for other tasks without waiting for the process to finish.

When a command with & is entered, the shell does not call wait(), allowing the user to continue interacting with the shell immediately. A message is printed showing the process ID (PID) of the background process.

For example:

bash
Copy code
sleep 10 &
This command will run the sleep process in the background, and the shell will return to the prompt instantly.

6. Signal Handling (Ctrl+C)
One common issue with command-line shells is accidentally terminating the shell when a user sends the Ctrl+C (SIGINT) signal. By default, this signal will terminate the shell as well as any running processes. To handle this, we implemented a custom signal handler for SIGINT using the signal() system call.

When a user presses Ctrl+C, the signal is caught, and the shell prints a message notifying the user to use the exit command to quit the shell. The signal handler ensures that the shell process itself remains unaffected by Ctrl+C.

7. I/O Redirection
Quash also supports basic input/output redirection, which is an important feature for any shell. We implemented both input redirection (<) and output redirection (>) using the dup2() system call.

Output Redirection: If the user types a command like ls > output.txt, the shell redirects the standard output of ls to the file output.txt rather than printing to the terminal.
Input Redirection: If the user types cat < input.txt, the shell reads the contents of input.txt as the input to cat.
These features make the shell much more versatile, allowing it to interact with files in a meaningful way.

8. Timeout for Long-Running Processes
A unique feature we implemented is a timeout for long-running processes. Any process that takes more than 10 seconds to complete is automatically terminated using the kill() function. This was achieved by setting a timer with setitimer() and sending a SIGKILL signal to the process when the timer expires.

This feature ensures that users do not have to deal with unresponsive or long-running processes that might hang indefinitely.

Documentation of Code

Main Components
shell.c: The main file that contains the implementation of the shell. It handles user input, command tokenization, built-in command execution, process management, and signal handling.
Makefile: Provides an easy way to compile the shell. Running make will compile the shell.c file into an executable called quash.
How to Compile and Run the Shell
Clone the repository:
bash
Copy code
git clone https://github.com/username/Shell-Quash.git
Compile the shell using make:
bash
Copy code
make
Run the shell:
bash
Copy code
./quash
Conclusion

This project helped me understand the complexities of building a command-line shell from scratch. Through implementing built-in commands, process management, signal handling, and I/O redirection, we gained hands-on experience in system-level programming in C. Although the shell is a simplified version of widely used Unix shells like bash, it demonstrates core functionalities that make it a fully functional shell for executing commands and managing processes.

In the future, this project can be expanded to include more advanced features such as command piping (|), job control, and improved error handling.
