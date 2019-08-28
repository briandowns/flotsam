# flotsam

[![Build Status](https://travis-ci.org/briandowns/flotsam.svg?branch=master)](https://travis-ci.org/briandowns/flotsam)

`flotsam` is a project generator and dependency manager for C.  

## How It Works

Flotsam is a tool for overall C project development and management.  At the outset, Flotsam can be used to generate a new application or library project.  It goes a step further by managing dependencies through definition in the Flotsam.toml file.  The Flotsam.toml file lives in the root of the repository and defines the project.  A few fields are required to exist and be populated like; name, repository, build, etc.  The dependency section is required however not needed to be populated.  The dependency will be brought down, built, and installed to the system.

### Requirements

Flotsam has strong opinions.  It puts the built dependencies in `/usr/local/lib`.  Flotsam keeps all dependencies in a cache directory in the users home dir.  One of the primary requirements though of Flotsam is that a Flotsam dependency needs to be a git repository and must create a shared object in the form of a `.so` or a `.dylib` after having a single build command ran.  The build command is defined in the Flotsam.toml file.  All examples use `make`.

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

Now run `flotsam update`.  Flotsam parses the Flotsam.toml file, clones, checks out the given branch or tag, performs a build of the dependency, and makes it available for linking and execution.
Then run `flotsam build`.  At this point, if there were not errors, the application has been built and the resulting binary has been placed in the `bin` directory.

Run the application:

```sh
./bin/my_new_app
```

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
