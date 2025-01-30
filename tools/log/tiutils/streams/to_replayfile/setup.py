import setuptools

setuptools.setup(
    name="tilogger-to-replayfile-output",
    version="0.2.0",
    packages=setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "tilogger.output": [
            "to-replayfile = tilogger_to_replayfile.main:output_factory_cli",
        ],
    },
    install_requires=["typer==0.3.2"],
)
