# Copyright © 2016 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

LDLIBS = -ldl
LDFLAGS = -Wl,--build-id=sha1
GREP_SHA1 = egrep -o '\b[0-9a-f]{40}\b'

all: build-id so-build-id dlopen-build-id

build-id: test.o build-id.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

so-build-id: so-test.o libbuild-id.so
	$(CC) $(LDFLAGS) $^ -o $@

dlopen-build-id: dlopen-test.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

shared-build-id.o: build-id.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $^ -o $@

libbuild-id.so: shared-build-id.o
	$(CC) $(LDFLAGS) -shared $^ -o $@ $(LDLIBS)

build-id-test.expected: build-id
	file $< | $(GREP_SHA1) &> $@

so-build-id-test.expected: libbuild-id.so
	file $< | $(GREP_SHA1) &> $@

dlopen-build-id-test.expected: libbuild-id.so
	file $< | $(GREP_SHA1) &> $@

build-id-test.result: build-id
	./$< | $(GREP_SHA1) &> $@

so-build-id-test.result: so-build-id libbuild-id.so
	LD_LIBRARY_PATH=. ./$< | $(GREP_SHA1) &> $@

dlopen-build-id-test.result: dlopen-build-id libbuild-id.so
	./$< | $(GREP_SHA1) &> $@

check: build-id-test.expected so-build-id-test.expected dlopen-build-id-test.expected build-id-test.result so-build-id-test.result dlopen-build-id-test.result
	cmp build-id-test.expected build-id-test.result
	cmp so-build-id-test.expected so-build-id-test.result
	cmp dlopen-build-id-test.expected dlopen-build-id-test.result
	@echo PASS

clean:
	rm -f build-id so-build-id dlopen-build-id libbuild-id.so *.o *.result *.expected
