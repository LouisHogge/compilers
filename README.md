# Compilers Project

## Project Description

In this project, you will implement a compiler for the VSOP language. VSOP stands for Very Simple Object-oriented Programming language. It is object-oriented, expression-based and statically typed. All the required information about this language in order to implement your compiler is written in its manual.

More precisely, you will implement the front-end part of a compiler, whose goal is to check the correctness of a VSOP source file and to generate the intermediate representation of this input program. As you will see during the theoretical courses, the front-end part of a compiler is composed of four steps:
1. the Lexical analysis part,where the source file (a stream of characters) is split in to a stream of tokens;
2. the Syntax analysis part, where this stream is turned into a syntax tree;
3. the Semantic analysis part, which annotates the syntax tree;
4. the Code generation part, which generates the intermediate representation from the an- notated syntax tree.

Once the intermediate representation is obtained, you will use an external compiler to generate the final executable.

## How to Use the Project

Use this command to:
- Start a container from the cffs/compilers image
- Open an interactive shell
- Tell Docker to stop and remove the container once you exit the shell Use the linux/amd64 platform
- Link your current directory to the directory compilers of the user vagrant
- Go to the directory compilers of the user vagrant
- Log in as vagrant

```bash
docker run --rm -it \
    --platform linux/amd64 \
    -v "$(pwd):/home/vagrant/compilers" \
    cffs/compilers \
    /bin/sh -c "cd /home/vagrant/compilers; su vagrant"
```