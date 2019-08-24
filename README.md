# flotsam

`flotsam` is a project generator and dependency manager for C. 

## Examples

Create a new application.  Flotsam will create all of the necessary files with the appropriate contents to write and build a new application.

```sh
flotsam new --bin my_new_app
```

Create a new library.  Flotsam will create all of the files and directories necessary to write and build a new library.

```sh
flotsam new --bin my_new_app
```

Update dependencies.  Flotsam will parse the `Flotsam.toml` file and get any missing dependencies.  One the dependency is downloaded, flotsam will attempt to build it.

```sh
flotsam update
```

## Features



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
