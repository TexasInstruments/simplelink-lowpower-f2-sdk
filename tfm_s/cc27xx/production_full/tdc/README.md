## Trusted Direct Call (TDC)

A Trusted Direct Call function is a function which exists in the Secure
Processing Environment (SPE) but is callable from the Non-Secure Processing
Environment (NSPE) without incurring the overhead of the Secure Partition
Manager (SPM). When a non-secure thread calls a TDC function, it will elevate
the processor to secure mode and invoke the TDC function. The TDC function
executes on the NS Agent's process stack, so it must run-to-completion and
return back to the NSPE. A TDC function is not reentrant.

Update the CMakeLists.txt file to include your source file(s) in the *tfm_s*
target. You must use a fully qualified path to your source file. If the file is
contained in this folder (tdc), use the following statement to include it in the
build:

<pre>
target_sources(tfm_s
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/File.c
)
</pre>

The build executes in the context of an SDK environment. If your source
file is in the SDK/source repository, use the following statement:

<pre>
target_sources(tfm_s
    PRIVATE
        ${CMAKE_SOURCE_DIR}/../../ti/path/File.c
)
</pre>
