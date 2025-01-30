import setuptools

setuptools.setup(
    name="tilogger-stdout-output",
    version="0.2.0",
    packages=setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "tilogger.output": [
            "stdout = tilogger_stdout.main:output_factory_cli",
        ],
    },
    install_requires=["typer==0.3.2"],
)
