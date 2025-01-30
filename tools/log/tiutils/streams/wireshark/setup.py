import setuptools

setuptools.setup(
    name="tilogger-wireshark-output",
    version="0.2.0",
    packages=setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "tilogger.output": [
            "wireshark = tilogger_wireshark.main:output_factory_cli",
        ],
    },
    install_requires=["pywin32==228", "typer==0.3.2", "colorama==0.4.4"],
)
