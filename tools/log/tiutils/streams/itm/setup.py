import setuptools

setuptools.setup(
    name="tilogger_itm_transport",
    version="0.2.0",
    packages=setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "tilogger.transport": [
            "itm = tilogger_itm_transport.itm_transport:transport_factory_cli",
        ],
        "console_scripts": ["tilogger_itm_viewer = tilogger_itm_transport.itm_transport:itm_raw_viewer"],
    },
    install_requires=["pyserial>=3.4", "typer==0.3.2", "colorama==0.4.4"],
)
