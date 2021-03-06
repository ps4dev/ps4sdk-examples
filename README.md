# ps4sdk-examples
Examples, showcases and prove-of-concepts which show something realized with ps4sdk (not necessarily unique to the PS4)

## Examples
* `libless` - These do not use any library functions (not even the sdk) and are good for initial tests or when something breaks
* `posix` - These solely use posix and standard C APIs and should run under any such system
* `freebsd` - These are pure freebsd examples (and should work there too)
* `sce` - These showcase SCE functions
* `ps4sdk` - These are user space programs which use the sdks own (common) capabilities
* `kernel` - These are showing something we can do in the kernel using the sdk
* `failures` - These are corner-cases, which noteworthyly do not work (but could be expected to by somebody)

## Noteworthy
* `freebsd/dirent` displays the fs and can be run after you use another elf to escalate your privileges
* `ps4sdk/kernel_execute` shows how to enter the kernel from user space on your own (it's likely less
convenient and useful then a kernel payload, but has its use-case).
* `ps4sdk/system_call_hook` shows how to temporarily hook from user space. Be aware, that when your program
exits, your resources (all functions and data) will be freed too. **Be sure to unlink** any of these user space resources
from the kernel before you exit main. A kernel payload that never ends (sleep on a mutex) is more likely to be useful and stable
for any long term or persistent (module like) modifications. See `kernel/system_call` for comparison.
* `kernel/system_call_hook`, `kernel/function_hook` print the same descriptor all the time. That's not a bug,
it's what actually happens. Press the options button to see changes. To influence the return values, use a post
hook and alter args->returns->rax (etc.). Try the function index (rindex) hook (on 5055), close the browser,
connect and restart the browser.

## Important
The exploit may not always enter the kernel on the first try. If you browser crashes,
simply try again. It's likely to work. This will be tuned in the future as much as possible.
Especially after you crashed your kernel, this behaviour is very common (down right normal) because the
initial resource allocation differs.
