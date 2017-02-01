// Copyright (C) 2017 SIB Swiss Institute of Bioinformatics, Lausanne, CH. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

// AOC network of kernels
// [Host] => A -> B -> C => [Host]
// where B is a task that consume the values produced by A and send them to C


#define SIZE 100

__kernel void A(__global int *in, write_only pipe int __attribute__((depth(SIZE))) __attribute__((blocking)) AB) {
	for(int i = 0; i < SIZE; ++i){
		write_pipe(AB, &in[i]);
	}	
}


__kernel void B(read_only pipe int __attribute__((depth(SIZE))) __attribute__((blocking)) AB, write_only pipe int __attribute__((depth(SIZE))) __attribute__((blocking)) BC) {
	int data;
	
	while(1){

		read_pipe(AB, &data);
		write_pipe(BC, &data);
	}
}


__kernel void C(__global int *out, read_only pipe int __attribute__((depth(SIZE))) __attribute__((blocking)) BC) {
	for(int i = 0; i < SIZE; ++i){
		read_pipe(BC, &out[i]);
	}
}