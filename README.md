# mini-parallel-query-engine
[![GitHub license](https://img.shields.io/github/license/jellycsc/mini-parallel-query-engine.svg)](https://github.com/jellycsc/mini-parallel-query-engine/blob/master/LICENSE)

This program is able to query words amongst different indices in parallel, which utilizes multi-core processors to some extent.

## Getting Started

### Prerequisites

* GCC
* Shell (Bash is recommended)

### Download source code and compile
The following instructions are presented using Bash in macOS:
```
# Change to HOME directory
$ cd ~

# Clone this repo and 'cd' into it
$ git clone https://github.com/jellycsc/mini-parallel-query-engine.git
$ cd mini-parallel-query-engine/

# Let's compile.
$ make
gcc -Wall -g -std=gnu99 -c indexer.c
gcc -Wall -g -std=gnu99 -c freq_list.c
gcc -Wall -g -std=gnu99 -c punc.c
gcc -Wall -g -std=gnu99 -o indexer indexer.o freq_list.o punc.o
gcc -Wall -g -std=gnu99 -c queryone.c
gcc -Wall -g -std=gnu99 -c worker.c
gcc -Wall -g -std=gnu99 -o queryone queryone.o worker.o freq_list.o punc.o
gcc -Wall -g -std=gnu99 -c printindex.c
gcc -Wall -g -std=gnu99 -o printindex printindex.o worker.o freq_list.o punc.o
gcc -Wall -g -std=gnu99 -c query.c
gcc -Wall -g -std=gnu99 -o query query.o worker.o freq_list.o punc.o
```

### Usage
```
Usage: indexer [-i FILE] [-n FILE] [-d DIRECTORY_NAME]
Usage: printindex [-i FILE] [-n FILE]
Usage: queryone [-d DIRECTORY_NAME]
Usage: query [-d DIRECTORY_NAME]
```

### Notes 
* The `indexer` is already run and the results are provided in `databse` folder.
* `index` and `filenames` are binary files in little endian.

## Examples
```
$ # Word count for "sort"
$ ./printindex -i database/labs/lab1/index -n database/labs/lab1/filenames 
1    ./pipes.txt
1    ./runtests
6    ./lab1.txt
$ ./query -d database/labs
sort
1    ./pipes.txt
1    ./runtests
6    ./lab1.txt
loops
1    ./lab1.txt
<press ctrl-d here>
```

## Author(s)

| Name                    | GitHub                                     | Email
| ----------------------- | ------------------------------------------ | -------------------------
| Chenjie (Jack) Ni       | [jellycsc](https://github.com/jellycsc)    | nichenjie2013@gmail.com

## Thoughts and future improvements

* Sort the results and display words of top most frequencies.

## Contributing to this project

1. Fork it [![GitHub forks](https://img.shields.io/github/forks/jellycsc/mini-parallel-query-engine.svg?style=social&label=Fork&maxAge=2592000&)](https://github.com/jellycsc/mini-parallel-query-engine/fork)
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -m 'Add some feature'`)
4. Push to your feature branch (`git push origin my-new-feature`)
5. Create a new Pull Request

Details are described [here](https://git-scm.com/book/en/v2/GitHub-Contributing-to-a-Project).

## Bug Reporting [![GitHub issues](https://img.shields.io/github/issues/jellycsc/mini-parallel-query-engine.svg?)](https://github.com/jellycsc/mini-parallel-query-engine/issues/)

Please click `issue` button above↑ to report any issues related to this project  
OR you can shoot an email to <nichenjie2013@gmail.com>

## License
This project is licensed under GNU General Public License v3.0 - see [LICENSE](LICENSE) file for more details.
