# UUID Loader

Experimental Stage-1 Shellcode Loader, using IE COM Object Methods to fetch Shellcode and uses UUID as shellcode.

# Features

- Fetches Cobalt Shellcode from Github using IE-Com 
- Shellcode returns is bunch of UUID Strings
- Uses UuidFromStringA callback to load shellcode into allocated heap area
- Dynamic API Resolving to make IAT Look clean
- Multiple Preliminary Checks before execution
- Implements BlockDLL;s to block non microsoft signed DLL to get loader - Blocking EDR DLL Hooks
- Does Local Process Injection, avoiding touching remote process 

# Upcoming 

- Obfuscated UUIDs and deobfuscate before triggering callback
- Ability to convert UUID to shellcode and inject remote using thread hijacking
- Using SGN or custom shellcode encoder before generating UUID Shellcode
 

# Idea

https://research.nccgroup.com/2021/01/23/rift-analysing-a-lazarus-shellcode-execution-method/

Credits to [slaeryan](https://twitter.com/slaeryan) for IE COM code [Wraith](https://github.com/slaeryan/AQUARMOURY/tree/master/Wraith)
