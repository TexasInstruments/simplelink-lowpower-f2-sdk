from .itm_transport import itm_parser_main
import coloredlogs, logging

coloredlogs.install(level="DEBUG", fmt="%(asctime)s %(name)20s[%(process)d] %(levelname)8s %(message)s")
itm_parser_main()
