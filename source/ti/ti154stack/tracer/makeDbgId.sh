echo "Creating new dbgid_sys_mst.h for application/MAC data debugging based on $1"
find $1 -name "*.c" -exec perl autodbg.pl dbgid_sys_mst.h -nt -c3 -alias DBGSYS 3 {} +;
