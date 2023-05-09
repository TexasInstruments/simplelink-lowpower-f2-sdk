from .logger import main
import coloredlogs, logging

coloredlogs.install(
    level="DEBUG", fmt="%(asctime)s %(name)20s[%(process)d] %(threadName)15s  %(levelname)8s %(message)s"
)
main()
