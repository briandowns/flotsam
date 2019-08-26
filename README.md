# flotsam

[![Build Status](https://travis-ci.org/briandowns/flotsam.svg?branch=master)](https://travis-ci.org/briandowns/flotsam)

`flotsam` is a project generator and dependency manager for C.  

## How It Works

Flotsam is a tool for overall C project development and management.  At the outset, Flotsam can be used to generate a new application or library project.  In the newly created project directory you'll find Once the project has been created development can begin.  

## Example

In the following example we'll create a new application using Flotsam, write a small program that depends on an external library, compile, and run the appliction entirely using Flotsam.

Create a new application.  Flotsam will create all of the necessary files with the appropriate contents to write and build a new application.

```sh
flotsam new --bin my_new_app
```

Add the below code to the `main.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "spinner.h"

int
main(int argc, char** argv)
{
    spinner_t* s = spinner_new(19);
    s->delay = 100000;
    s->prefix = "Running ";
    s->suffix = " I'm a suffix";
    s->final_msg = "\nComplete!\n";
    spinner_start(s);
    sleep(5); // simulate some work
    spinner_stop(s);
    spinner_free(s);
    return 0;
}
```

In the `Flotsam.toml` "dependencies" section, add the following line:

```
"github.com/briandowns/libspinner" = "0.2.2"
```

Now run `flotsam update`.  
And then run `flotsam build`.  At this point, if there were not errors, the application has been built and the resulting binary has been placed in the `bin` directory.

Run the application:

```sh
./bin/my_new_app

## Features

* Create new applications and libraries including file and directory scaffolding.
* Ready to use Dockerfile

## Test

```sh
make test
```

## Installation

```sh
make install
```

## Contributing

Please feel free to open a PR!

## License

loads source code is available under the BSD 2 clause [License](/LICENSE).

## Contact

[@bdowns328](http://twitter.com/bdowns328)
