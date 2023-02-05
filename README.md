
# System Monitoring Tool
	This C program reports different metrics of utilization of a given system.It calculates 
	memory usage, CPU usage, and also can also display the given users logged in.

## Flags Problem:

	The first problem is to be able to manage the flags the user gives correctly.
	The flags for the assignment (no bonus) were:
		--user  to indicate that only the users usage should be generated
		
		--system to indicate that only the system usage should be generated
		
		--sequential  to indicate that the information will be output sequentially  
		without needing to "refresh" the screen 
		
		--tdelay=T to indicate how frequently to sample in seconds. If not value is 
		indicated the default value will be 1 sec.
		
		--samples=N if used the value N will indicate how many times the statistics are 
		going to be collected and results will  be average and reported based on the N 
		number of repetitions. If not value is indicated the default value will be 10.
		
		The last two arguments can also be considered as positional arguments if not 
		flag is indicated in the corresponding order: samples tdelay.

	Before solving this programmatically, I identified invalid combinations of flags.
		
		The following should be INVALID:
		--user and --system together (contradictory to description)
		--tdelay=T or --samples=N WITH positional arguments
		One positional argument
	
## Flags Solution:
	
		Solving this problem is quite easy. We will simply have integer values for each 
		of the flag, and a flag that keeps track of the  number of integer flags. By 
		using a for loop to iterate over the flags typed in by the user, we can set the 
		appropriate flag.(It is important to note that --user and --system respective 
		integer representations will be set to  1 as default. Set samples and set time 
		to 0. Samples and time to 10 and 1 respectively. Sequential to 0. And number of 
		integers to 0.)
		
		For example, if the user specifies --system, we will simply set the --user 
		integer representation to 0, indicating it to not be represented.
		
		The next problem are the positional flags. This problem is quite easy to solve 
		as well. If the first digit is a character, then we must check if every 
		character is a digit, if it isn't then it is a invalid flag. This is done 
		through the checkIfString  function in the code. We must also check, however, 
		the number of integers currently. For our first occurrence it must be 0. We 
		then confirm if it is a number and set samples and increment number of integers 
		to 1. 
		
		The next part is making sure it's two concsecutive digits. Simply check if the 
		previous flag has its first character as a digit as well and check the number of
		integers field, make sure its 1. 
		
		The reason this works is:
			Suppose I have 8 --user 3 4 This will return false. Why? once 8 is read,
			number of integers is incremented. It becomes 1. Once I read 3, I must 
			check if the previous has its first character as a digit (because number
			of integers is not 0 and it will go to next if statement). 
			
			The specification of number of integers carefully contrives it to only 
			take two consecutive numbers.
			
		Next are the tdelay and samples flags. We simply compare the first 9, 10 
		characters, respectively and see if it matches  our flag. If it does then that's
		great. Additionally check the 11th character to see if it's null. If it is then 
		that means it's a invalid flag. The next part just checks to see if the string 
		after = is actually a number. This can be done using the checkIfString function 
		in the code.
		
		Next are the valid combinations. If --user and --system was called then this 
		means their integer values are now 0. Simply check if this is the case. If num 
		of integers is equal to 2 and the integer values for set samples or set delay 
		are 1, then this is invalid.

		After check if every flag is valid. Simply check the user and  system values to 
		see what to print. Also check sequential value to see if we should not refresh. 
		The printing is done in a for loop, which goes up to the value of samples. 
		Before the next iteration, the program sleeps for the specified time interval.
		
## Problem and Solution for refreshing:
	
		I simply cleared the screen using the escape code "\e[H\e[2J\e[3J" which moves 
		the cursor to the the top left using "\e[H", clears the screen through "\e[2J" 
		and clears the scrollback buffer through "\e[3J" to give it a refreshing effect
		. The problem then becomes, if sequential isn't called. How do I print the past 
		values of memory? This is simple as well. Simply use an array!
		

## Displaying the desired information:
	
		The last problem is to print all the information our user wants.This is just A 
		LOT of reading linux documentations and piazza posts. 
		
		The first problem is memory!
		We use the <sys/sysinfo.h> library for this one. According to 
		https://man7.org/linux/man-pages/man2/sysinfo.2.html You call sysinfo
		(struct sysinfo *info) to get the memory information. 
		Total physcial ram was calculated through totalram. 
		Total virtual ram was calculated through totalram + totalswap.
		Used physical ram was calculated through totalram - freeram
		Used virtual ram was calculated through total virtual ram - free ram - free swap.
		
		Then simply store these values into an array so they can be used for later 
		printing.
		
		The second problem is CPU usage
		
		The first part is quite easy: displaying the cores. I defined the cores to be 
		the number of processors configured in the system. <sys/sysfo.h> happens to have
		a function called get_nprocs_conf(). According to the documentation: The 
		function  get_nprocs_conf() returns the number of processors configured by the 
		operating system.

		The second part is cpu usage percentage. I simply read from the /proc/stat file 
		which had all the statistics across all the processors in the first line. I read
		the first line and using the formula Marcelo gave, calculated the usage. The 
		formula was: (|s2 - s1|/ s1) * 100, where s1 = all times - idle (previous) & 
		s2 = all times - idle (current). The documentation for /proc/stat had idle as 
		the 4th number column, so I simply didn't  add its value to my calculations. 
		Additionally, although there were 10 total number columns. I didn't add the last
		two, as they were already included in previous columns per /proc/stat 
		documentation. I also calculated the baseline s1 prior to the printing process, 
		so that when the iterations begin the percentage could be calculated. By having 
		the return type as a long int, I can save the previous iteration's summation of 
		times without an array and use it for the next iteration!

		The third problem is displaying users.
		
		This was actually quite simply. The utmp file contains  information for users 
		logged in. It also so happens that <sys/utmp.h> has methods that make reading 
		from this file easy. Simply use setutent() to place a struct utmp pointer to the 
		beginning of the utmp file. From there use getutent() to read the file at the 
		location of pointer and assign values to the  fields of struct utmp. Then make 
		sure to check that the login process is normal. This is done through the constant 
		USER_PROCESS. If it is, then we can print the desired information!
		
		The next problem is displaying the system's logistics. Simply use the appropiate
		fields of struct utsname from the <sys/utsname.h> library.
		
		The last problem is displaying the memory usage of the program. Luckily, 
		<sys/resource.h> has a struct rusage with a field for the maximum resident size 
		(ru_maxrss). By passing specfying the who of getrusage(int who, struct *rusage),
		we can set the struct fields to be relevant to the program. Simply use the 
		defined constant RUSAGE_SELF. Then once the rusage struct's values are 
		intialized we can print the memory usage of the program using ru_maxrss.
		
That solves all of our problems!!!

## Function Overview:

#### bool checkIfStringIsNumber(char \*string)
        brief: checks if char \*string is a number 
        param: char* to a string
        return: return true iff every character in char\* (up to null) is a 
        number

#### void header(int samples, int interval)
        brief: Prints the number of int samples  and the int interval of time 
        the user has specified. Also prints the memory usage of the running 
        program
        param: int samples and int interval: these can be set by the user via 
        flags
    
   
   	
#### void sequentialHeader(int iteration)
        brief: Only prints when user specifies sequential header. The header 
        prints the int iteration, as well as the memory usage of the running 
        program.
        
        param: int iteration: this specifies what iteration out of samples sequential is on.
    
 
#### void printSystem(int iteration, int samples, double  \*array, int sequential)
        brief: Calculates and prints the used and total physical/virtual memory.
        
        param: int iteration: the iteration out of samples the program is currently on. 
               int samples: the number of samples specified by user.
               double \*array: used to store and retrieve used and total physical/vritual memory
                               values. 
               int sequential: if user wants sequential, then different print process.
        
        Explanation of printing:
        If sequential is 1 then the program prints newlines until the int iteration, where it 
        will print the physical/virtual memory usage and total, and then proceeds to print 
        newlines until int samples - 1
        
        If sequential is 0 then the program will print the physical/virtual memory usage and 
        total retrieved from \*array until int iteration where it will calculate and then print 
        the iteraton's memory usage. From iteration + 1 to samples - 1 inclusive, it will print 
        new lines.


#### long int cpuInfo(long int previous, int i)
        brief: Calculates change in CPU usage based on the long int previous 
        summation of cpu times excluding idle time.
        
        param: long int previous: this is the saved previous value of the previous iteration. 
                                  This is helpful in finding the percentage use.
                int i: this specifes what iteration program is on.
        

        If int i = -1, then this is baseline iteration and it will only return the summation 
        without printing the percetange use
        
        If int i != -1, then calculate cpu times without idle and take its absolute difference 
        with long int previous and divide by the latter and multiply by 100 to calculate and 
        print percentage use relative to last iteration (OH recommendation).
        
        return: return a long int which is the calculated summation of times (without idle), so 
        it can be used for next iteration.

#### void printUser()
        brief: Prints the users and sessions currently logged into the system using utmp

#### void printLogistics()
        brief: Prints the logistics of the system, inlcuding system name, machine name, version,
        release, and architecture


## How to run program:

		The program works to take any VALID combination of the flags in any VALID order 
		(valid order refers to positional arguments, nothing else) 
		
		Samples and Time are INTEGER values, therefore they have values 
		-2^31 <= samples,time <= 2^31 - 1
		
		DO NOT scroll while the information is displaying. This screws up printing. 
	

		
		
