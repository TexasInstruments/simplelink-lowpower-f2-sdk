import setuptools

setuptools.setup(
    name="tilogger",
    version="0.2.0",
    # author="Core SDK team",
    packages=setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "console_scripts": ["tilogger = tilogger.logger:main"],
    },
    install_requires=[
        "pyserial>=3.4",
        "pyelftools==0.25",
        "construct==2.9.45",
        "appdirs==1.4.3",
        "watchdog==1.0.2",
        "typer==0.3.2",
        "colorama==0.4.4",
        "coloredlogs",
    ],
)
