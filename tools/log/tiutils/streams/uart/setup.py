import setuptools

setuptools.setup(
    name="tilogger_uart_transport",
    version="0.2.0",
    packages=setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "tilogger.transport": [
            "uart = tilogger_uart_transport.uart_transport:transport_factory_cli",
        ],
    },
    install_requires=["pyserial>=3.4", "typer==0.3.2", "colorama==0.4.4"],
)
