from .uart_transport import uart_parser_main
import coloredlogs, logging

coloredlogs.install(level="DEBUG", fmt="%(asctime)s %(name)20s[%(process)d] %(levelname)8s %(message)s")
uart_parser_main()
