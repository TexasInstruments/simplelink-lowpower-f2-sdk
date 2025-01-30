import setuptools

setuptools.setup(
    name="tilogger-from-replayfile-output",
    version="0.2.0",
    packages=setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "tilogger.transport": [
            "from-replayfile = tilogger_from_replayfile.main:transport_factory_cli",
        ],
    },
    install_requires=["typer==0.3.2"],
)
