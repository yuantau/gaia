# Gaia is a simple script engine, a learning work.

## Features
+ cstyle syntax
+ cross-platform


## Build
````bash
gaia > mkdir  build
gaia > cd build
gaia > cmake ..
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
````c
gaia.call('hello');
````


## Get script value in c
````c
script.getGlobal("name");
printf("%d\n", script.toString());
````
`getGlobal`push the variable to the top of the stack, then u can use `toString()` get the string value.
