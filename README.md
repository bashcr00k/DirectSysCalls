# DirectSysCalls

a simple introduction into direct syscalls

# what is kernel level

the os is made of rings, ring 0 is the kernel and ring 3 is the user-mode (where we are), the kernel does everything so whenever we want to do something we must ask the kernel for it but if the user can do that directly that would be very unsafe so this rings were created for security

# What Are Direct SysCalls
when we execute something like createfile from the windows api we are calling it from kernel32.dll it is so far from the kernel, which then will call the nt function that correspends to it which is ntcreatefile from ntdll.dll, now we are closer to the kernel ,finally windows will set up the registers then call executes the syscall directly, you can see that in the ntdll.dll here 
![image](https://github.com/user-attachments/assets/35881c1c-5714-4afd-8515-e4c6b2b83150)

as you can see the first line is required to call a syscall the second line is actually moving the syscall id into eax and in the last line it executes the syscall (the other lines are not important just a check) 

So just calling createfile took us through these layers in order to eventually get to the syscall, now in every layer there could be ways to monitor everything we do by edrs and avs functions like createremotethread could be hooked so whenever we use it (and let windows do everything) we will get flagged, and this is the whole idea of direct syscalls get through all the layers directly executing the syscall using our own assembly stub and this is what this code does 

# how my code works
as i explained before in order to execute a syscall we need to get the syscall id which is usually the 4th offset byte starting from the fucntion address but here is the thing this id might change from build to build so we can't really hardcode it we instead need to dynamically get it and that is what the first function of the code does 

in order to get the syscall id we need to look inside ntdll.dll so we need a handle to it, ntdll.dll is loaded by default in all processes so we can just use getmodulehandle in order to get a handle to it, next we need the function address we do that by using getprocaddress now as i said the syscall id is the 4th byte so we just add 4 to extract it 

now that we have the id we need to execute the assembly stub if you look inside you will find 2 functions the first one is used to get the syscall id from the c code into the assembly stub the second one is used to execute the syscall so how does the first one work

here we need to understand the windows calling convention 
Argument  	Register
1st	        RCX
2nd	        RDX
3rd	        R8
4th	        R9
so if we pass one argument to a function it will get stored inside rcx we will use that as you can see i passed one argument to the function and then moved rcx into another register then moved the new register into the variable that contains the syscall id now we have the syscall id inside the stub all we need to do is execute the syscall which is simple read the code 

# Usage 
`HeavensGate -h` to show the help message 


```
id     extracts the syscall id of the given function EX : -id NtCreateFile
-t      executes NtCreateFile as a proof of concept \n
-h      prints this message
```

# Important Notes
as u have noticed i called this heavensgate as a ref to the original poc written by smelly__vx (@RtlMateusz) and am0nsec (@am0nsec) and this will be the topic of my next poc which is about direct syscalls and how we can bypass edr hooks using the method they created ;) 

# ScreenShot
![image](https://github.com/user-attachments/assets/5ca27ae6-3e3e-49dc-998d-d145ec01f30e)







