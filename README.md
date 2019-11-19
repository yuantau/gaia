# Gaia is a simple script engine, a learning work.

## Build
````bash
$ mkdir  build
$ cd build
$ cmake ..
````

## Binding a new function in c

````c
int csub(Gaia::Script * script)
{
	int b = script->toInt();
	int a = script->toInt();
	script->pushInt(a - b);
	return 1;
}

script.pushLocalFunction("csub", csub);
````

````js
print(csub(5,2));   //3
````

using `script->to[Type]` to get the params, right to left.
`return 1` means this function has a return value. 
u need put the return value on the stack first. `script->pushInt(a - b);`

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
