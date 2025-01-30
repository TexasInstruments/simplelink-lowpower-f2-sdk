"""Helper functions used by various modules"""


def build_value(buf):
    """
    Helper function: Turn an iterable into a little-endian integer
    """
    value = 0
    for idx, val in enumerate(buf):
        value += val << (idx * 8)
    return value
