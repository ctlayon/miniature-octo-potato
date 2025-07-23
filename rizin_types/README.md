# rizin types
[Read this first](https://book.rizin.re/src/analysis/types.html)

> Imagine yourself walking around the park on a hot summer day in July, spinning your keys, minding your own business and out of nowhere an untyped variable hits you in the face. It doesn't hurt, per se, but it does cause you to squint more than is reasonable or comfortable. Before you know it you're muttering to yourself, adding up the offsets of the bases, and realizing things don't stack up how you'd like them to. A little pad here, keeps things nice and tidy, a little pad there goes a long way to help maintain the balance. Things are starting to feel good again, but somethings not quite right. You look around and you realize you're actually in a cuckoo's nest! Stick by stick. Twig by twig. You deconstruct the nest until everything is spread out, flat, in front of you. You smile at the beautiful dragon and sip your orange coffee.

Let's start from the [source](https://github.com/ctlayon/miniature-octo-potato/rizin_types/main.c) and do a practical example with types in rizin:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Struct to hold connection information
struct ConnectionInfo {
    int socket_fd;
    struct sockaddr_in address;
    socklen_t addr_len;
};

int main() {
    int server_fd, client_fd;
    struct ConnectionInfo server_info, client_info;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Setup server_info
    server_info.socket_fd = server_fd;
    server_info.address.sin_family = AF_INET;
    server_info.address.sin_port = htons(8888);
    server_info.address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.addr_len = sizeof(server_info.address);
	// ... truncated
}
```

```bash
gcc -o main main.c
```

By default you get the following output:

```c
▶ rizin main
[0x100003cb0]> af
[0x100003cb0]> pdf

╭ int main(int argc, char **argv, char **envp);
│           0x100003cb0      push  rbp
│           0x100003cb1      mov   rbp, rsp
│           0x100003cb4      sub   rsp, 0x60
│           0x100003cb8      mov   rax, qword [reloc.__stack_chk_guard]
│           0x100003cbf      mov   rax, qword [rax]
│           0x100003cc2      mov   qword [var_10h], rax
│           0x100003cc6      mov   dword [var_44h], 0x00
│           0x100003ccd      mov   edi, 0x02
│           0x100003cd2      mov   esi, 0x01
│           0x100003cd7      xor   edx, edx
│           0x100003cd9      call  sym.imp.socket
│           0x100003cde      mov   dword [var_48h], eax
│           0x100003ce1      cmp   dword [var_48h], 0xffffffff
│       ╭─< 0x100003ce5      jnz   0x100003d01
│       │   0x100003ceb      lea   rdi, qword [str.socket_failed]
│       │   0x100003cf2      call  sym.imp.perror
│       │   0x100003cf7      mov   edi, 0x01
│       │   0x100003cfc      call  sym.imp.exit
│       ╰─> 0x100003d01      mov   eax, dword [var_48h]
│           0x100003d04      mov   dword [var_28h], eax
│           0x100003d07      mov   byte [var_23h], 0x02
│           0x100003d0b      mov   word [var_22h], 8888
│           0x100003d11      lea   rdi, qword [str.127.0.0.1]
│
// ... truncated ...
```

Decompiling using ghidra provides a little more insight, such as `var_28h`, `var_23h`, and `var_22h` may be related:

```c
[0x100003cb0]> pdg
undefined8 main(void)
{
// ... truncated ...

    iVar1 = sym.imp.socket(2, 1, 0);
    if (iVar1 == -1) {
        sym.imp.perror("socket failed");
        sym.imp.exit(1);
    }
    var_28h._5_1_ = 2;
    var_28h._6_2_ = 8888;
    var_28h._0_4_ = iVar1;
    var_20h = sym.imp.inet_addr("127.0.0.1");
// ... truncated ... 
```

If you calculate the offsets for the appropriate base pointer you might expect something like:

```c
struct ConnectionInfo {
    int socket_fd;
    struct sockaddr_in address;
    socklen_t addr_len;
};

socket_fd: 4
sockaddr_in: 
	sin_family: 2
	sin_port:   2
	sin_addr:   4
	sin_zero:   8
socklen_t: 4
```

However rizin does not handle nested structs well and, perhaps due to compiler optimization / byte alignment, the following flattened structure is required (for clarity I prefer to use uint*'s):

```c
struct connection_info{
    uint32_t socket_fd;
	uint8_t pad;
	uint8_t sin_family;
	uint16_t sin_port;
	uint32_t sin_addr;
	uint8_t sin_zero[8];
    uint32_t addr_len;
};
```

This can be saved in a file `connection.h` and be loaded  and applied via:

```bash
to connection.h
afvt var_28h connection_info

# optionally rename the stack variable to con_info
afvn con_info var_28h
```

Which results in the beautiful:

```c
[0x100003d01]> pdb
│           0x100003d01      mov   eax, dword [var_48h]
│           0x100003d04      mov   dword [con_info.socket_fd], eax
│           0x100003d07      mov   byte [con_info.sin_family], 0x02
│           0x100003d0b      mov   word [con_info.sin_port], 8888
│           0x100003d11      lea   rdi, qword [str.127.0.0.1]

	// ... truncated ...
    uVar2 = sym.imp.socket(2, 1, 0);
	// ... truncated ...
    con_info.sin_family = 2;
    con_info.sin_port = 8888;
    con_info.socket_fd = uVar2;
    con_info.sin_addr = sym.imp.inet_addr("127.0.0.1");
    con_info.addr_len = 0x10;
	// ... truncated ...
```

Working with types in rizin is intuitive and not dissimilar to other RE frameworks like IDA or Binja. Cutter additinally bridges this gap for those who prefer a more rubust GUI. Due to the nature of commonly defining header files, they are also extremely portable. Paired with the `ah?` commands you can obtain incredibly clear and readable disassembly with minimal markup.
