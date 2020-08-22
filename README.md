# ncc is Not a Compiler Collection

Codes for Nanjing University Principles and Techniques of Compiler Labs.

## Usage

1. Get the repository with submodules.

	```
	git clone --recursive https://github.com/forewing/NCC2020A.git
	```

2. Checkout codes at corresponding tag.

	| Lab | tag |
	| :- | :- |
	| 1 | v0.1.5 |
	| 2 | v0.2.99 |
	| 3 | v0.3.99 |
	| 4 | v0.4.0 |

## Build

`./Code/Makefile` is for my own environment, you may need to replace it with `./Makefile_official` when using standard environment.

## Test

- For your native environment, simply follow the targets defined in `Makefile`.

- `Dockerfile` and `docker-compose.yml` are provided. You may run `docker-compose up` to build & test your codes in standard environment.
