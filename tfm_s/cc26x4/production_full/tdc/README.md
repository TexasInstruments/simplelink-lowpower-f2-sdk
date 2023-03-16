## Trusted Direct Call (TDC)

A Trusted Direct Call function is a function which exists in the
secure partition but is callable from the non-secure partition.
When a non-secure thread calls a TDC function, it will elevate the
processor to secure mode and invoke the TDC function. The TDC function
executes on a secure stack, so it must run-to-completion and return
back to the non-secure side. A TDC function is not reentrant.

Update the CMakeLists.inc file to include your source file in the build.
You must use a fully qualified path to your source file. If the file
is contained in this folder (tdc), use the following statement to include
it in the build:

<pre>
list(APPEND ALL_SRC_C_S "${CMAKE_CURRENT_LIST_DIR}/File.c")
</pre>

The build executes in the context of an SDK environment. If your source
file is in the SDK/source repository, use the following statement:

<pre>
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/path/File.c")
<pre>
