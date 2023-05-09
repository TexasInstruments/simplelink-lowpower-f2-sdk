#!/bin/bash

if [[ -z "$PY3" ]]; then
  PY3=python3.7
fi

echo "Python version is : "
$PY3 --version

RTLS_AGENT_DIR=$PWD
UNPI_DIR=$RTLS_AGENT_DIR/unpi
RTLS_DIR=$RTLS_AGENT_DIR/rtls
RTLS_UTIL_DIR=$RTLS_AGENT_DIR/rtls_util

# handle zero arguments
if [[ $# -eq 0 ]]; then
    echo "$0: At least one input parameter required. Try -h for more info"
    echo ""
    exit 4
fi

for i in "$@"; do
    case $i in
        -c)
            echo ""
            echo "=================================================="
            echo "        Removing dist and build of RTLS_UTIL      "
            echo "=================================================="

            pushd $RTLS_UTIL_DIR

            rm -rf dist
            rm -rf build
            rm -rf *.egg-info

            popd

            echo ""
            echo "=================================================="
            echo "          Removing dist and build of RTLS         "
            echo "=================================================="

            pushd $RTLS_DIR

            rm -rf dist
            rm -rf build
            rm -rf *.egg-info

            popd

            echo ""
            echo "=================================================="
            echo "          Removing dist and build of UNPI         "
            echo "=================================================="

            pushd $UNPI_DIR

            rm -rf dist
            rm -rf build
            rm -rf *.egg-info

            popd


            shift # past argument=value
            ;;
        -b)
            echo ""
            echo "=================================================="
            echo "                 Build UNPI package               "
            echo "=================================================="

            pushd $UNPI_DIR

            $PY3 setup.py sdist bdist_egg

            popd

            echo ""
            echo "=================================================="
            echo "                 Build RTLS package               "
            echo "=================================================="

            pushd $RTLS_DIR

            $PY3 setup.py sdist bdist_egg

            popd

            echo ""
            echo "=================================================="
            echo "               Build RTLS_UTIL package            "
            echo "=================================================="

            pushd $RTLS_UTIL_DIR

            $PY3 setup.py sdist bdist_egg

            popd

            shift # past argument=value
            ;;
        -u)
            echo ""
            echo "=================================================="
            echo "          Uninstalling UNPI package               "
            echo "=================================================="

            $PY3 -m pip uninstall -y ti-simplelink-unpi

            echo ""
            echo "=================================================="
            echo "          Uninstalling RTLS package               "
            echo "=================================================="

            $PY3 -m pip uninstall -y ti-simplelink-rtls

            echo ""
            echo "=================================================="
            echo "          Uninstalling RTLS_UTIL package          "
            echo "=================================================="

            $PY3 -m pip uninstall -y ti-simplelink-rtls-util

            shift # past argument=value
            ;;
        -i)
            echo ""
            echo "=================================================="
            echo "            Installing UNPI package               "
            echo "=================================================="

            $PY3 -m pip install $UNPI_DIR/dist/ti-simplelink-unpi-0.3.tar.gz

            echo ""
            echo "=================================================="
            echo "            Installing RTLS package               "
            echo "=================================================="

            $PY3 -m pip install $RTLS_DIR/dist/ti-simplelink-rtls-0.8.tar.gz

            echo ""
            echo "=================================================="
            echo "            Installing RTLS_UTIL package          "
            echo "=================================================="

            $PY3 -m pip install $RTLS_UTIL_DIR/dist/ti-simplelink-rtls-util-1.4.8.tar.gz

            shift # past argument=value
            ;;


        -h)
            echo "package.sh [-h HELP] [-c CLEAN] [-b BUILD] [-i INSTALL] [-u UNINSTALL]"
            echo ""
            echo "Description:"
            echo "      This script is used to basic action on python packages such as clean / build / install / uninstall"
            echo ""
            echo "Parameter List:"
            echo "      -c      Clean packages folder from 'build' / 'dist' / '*.egg-info'"
            echo "      -b      Build packages"
            echo "      -i      Install packages from 'dist' folder using pip command"
            echo "      -u      Uninstall packages using pip command"
            echo "      -h      Help"
            echo ""
            echo "Examples:"
            echo "     package.sh -h              For Help"
            echo "     package.sh -c -b           For clean and build packages"
            echo "     package.sh -u -i           For uninstall old packages and install new packages"
            echo "     package.sh -c -b -u -i     For whole process"
            echo ""
            echo ""

            shift # past argument with no value
            ;;
        *)
            echo "Missing input parameters"
            exit 3
            ;;
    esac
done

