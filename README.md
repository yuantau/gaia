# Gaia is a simple script engine, a learning work.

## Features
+ cstyle syntax
+ cross-platform

## TODO
+ GC
+ Object
+ Recursive

## Build
````bash
$ mkdir  build
$ cd build
$ cmake ..
````

on windows
````bash
cmake --build
````

or osx
````bash
make
````

## Binding a new function in c

````c
int test(Gaia * gaia) {
    printf("test function");
    return 0;
}
gaia.pushLocalFunction("test", test);
````

c function return 0 or 1

##  Call script function from C
````js
function add(a, b) {
    return  a + b;
}
````

````c
script.pushInt(2);
script.pushInt(10);
script.call("add");
printf("%d", script.toInt());
````


## Get script value in c
````js
var name = "yuan";
var tick = 100;
````
````c
script.getGlobal("name");
printf("%s\n", script.toString());
script.getGlobal("tick");
printf("%d\n", script.toInt());
````
`getGlobal`push the variable to the top of the stack, then u can use `toString()` get the string value.
