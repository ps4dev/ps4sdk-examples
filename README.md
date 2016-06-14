# ps4sdk-examples
Examples and prove of concepts which show something realized with ps4sdk (not necessarily unique to the PS4)

## Examples
* `libless` - These do not use any library functions (not even the sdk) and are good for initial tests
* `posix` - These purely use posix and standard C APIs and should run under any such system
* `freebsd` - These are pure freebsd examples (and should work there too)
* `sce` - These showcase SCE functions
* `ps4sdk` - These are user space programs which use the sdks own (common) capabilities
* `kernel` - These are showing something we can do in the kernel using the sdk
* `failures` - These corner-cases noteworthyly do not work (but could be expected to)

## Noteworthy
* `freebsd/dirent` displays the fs and can be run after you use another elf to escalate your privileges
* `ps4sdk/kernel_execute` shows how to do enter the kernel from user space on your own (it's likely less
convenient and useful then a kernel payload, but has a use-case too). Be aware, that when your program
exits, your resources (all functions and data) will be gone too. Be sure to unlink these user space resources
from the kernel! Use a kernel payload that never ends (sleep on a mutex) instead.
* `ps4sdk/system_call_hook` shows how to temporarily hook from user space. For the reasons above,
a kernel payload is more likely to be useful and stable for any long term attempts. See `kernel/system_call` for
comparison
* `kernel/system_call_hook kernel/function_hook` print the same descriptor all the time, thats correct because thats
what happens. Press the options button to see changes. To influence the return values, use a post hook and alter
args->returns->rax (etc.). Try the function index hook (on 5055) and restart the browser!

## Important
The exploit may not always enter the kernel on the first try. Is you browser crashes,
simply try again and its likely to work. This will be tuned in the future as much as possible.
Especially after you crashed your kernel, this is very common (down right normal) because the
initial resource allocation differs.
