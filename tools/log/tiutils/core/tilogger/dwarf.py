import bisect
from dataclasses import dataclass
from typing import List, Dict, Optional, Tuple, Iterable, Union

from elftools.elf.elffile import ELFFile
from functools import lru_cache

from elftools.common.py3compat import maxint, bytes2str
from elftools.dwarf.descriptions import describe_form_class

from elftools.elf.elffile import ELFFile, DWARFInfo
from elftools.dwarf.die import DIE

import bisect


class RangeDict:
    def __init__(self, my_dict):
        # assert not any(map(lambda x: not isinstance(x, tuple) or len(x) != 2 or x[0] > x[1], my_dict))

        self._mydb = [(k[0], k[1], v) for k, v in my_dict.items()]
        self._mydb.sort()
        self._dblen = len(self._mydb)

    def __getitem__(self, number):
        idx = bisect.bisect_right(self._mydb, (number + 1,))
        if idx < self._dblen and self._mydb[idx - 1][1] > number:
            return self._mydb[idx - 1][2]
        else:
            raise KeyError

    def get(self, number, default=None):
        try:
            return self.__getitem__(number)
        except KeyError:
            return default


# Translation of some of the DWARF attribute tags for printing
tagNice = {
    "DW_TAG_pointer_type": "pointer",
    "DW_TAG_typedef": "typedef({name})",
    "DW_TAG_base_type": "primitive({name})",
    "DW_TAG_volatile_type": "volatile",
    "DW_TAG_array_type": "array",
    "DW_TAG_const_type": "const",
    "DW_TAG_member_type": "member({name})",
    "DW_TAG_member": "{name}",
    "DW_TAG_formal_parameter": "argument",
    "DW_TAG_enumeration_type": "enum({name})",
    "DW_TAG_enumerator": "enumvalue({name})",
    "DW_TAG_subroutine_type": "function",
    "DW_TAG_subprogram": "function({name})",
    "DW_TAG_structure_type": "struct({name})",
    "DW_TAG_unspecified_type": "void",
    "DW_TAG_union_type": "union({name})",
    "DW_TAG_variable": "variable({name})",
    16520: "call_to({name})",
}

symbolTags = [
    "DW_TAG_pointer_type",
    "DW_TAG_typedef",
    "DW_TAG_base_type",
    "DW_TAG_volatile_type",
    "DW_TAG_array_type",
    "DW_TAG_const_type",
    "DW_TAG_member_type",
    "DW_TAG_member",
    "DW_TAG_formal_parameter",
    "DW_TAG_enumeration_type",
    "DW_TAG_enumerator",
    "DW_TAG_subroutine_type",
    "DW_TAG_subprogram",
    "DW_TAG_structure_type",
    "DW_TAG_unspecified_type",
    "DW_TAG_union_type",
    "DW_TAG_variable",
]

DIEOffsetDict = Dict[int, DIE]


def make_die_dict(dwarfinfo: DWARFInfo) -> DIEOffsetDict:
    """
    Make a dictionary to look up DIEs based on their offset, since that is how they reference each other
    :param dwarfinfo: The dwarfinfo member of an ELFFile
    :return: Dictionary of DIE by offset
    """
    ret_dies = {}

    def die_rec(die):
        ret_dies[die.offset] = die
        for child in die.iter_children():
            die_rec(child)

    for cu in dwarfinfo.iter_CUs():
        die_rec(cu.get_top_DIE())

    return ret_dies


DIELocationList = List[Tuple[int, DIE]]


def make_die_location_dict(die_dict: DIEOffsetDict) -> DIELocationList:
    locs = []
    for die in die_dict.values():
        if die.tag in symbolTags:
            loc = die_get_location(die)
            if loc:
                locs.append((loc, die))
    locs.sort(key=lambda x: x[0])
    return locs


def get_die_for_location(location_list: DIELocationList, location: int):
    idx = bisect.bisect_right([x[0] for x in location_list], location)
    return location_list[idx - 1][1]


def find_die_for_symbol(dies: List[DIE] or DIEOffsetDict, symbol: str) -> Optional[DIE]:
    if isinstance(dies, dict):
        dies = dies.values()
    return next(filter(lambda die: die_get_name(die) == symbol, dies), None)


@dataclass
class TypeNode:
    """
    Linked list of DIEs where 'next' is the DIE referenced in the type attribute
    """

    die: DIE
    next: "TypeNode" or None
    children: List["TypeNode"]


def recurse_type(
    die_dict: Dict[int, DIE], die: DIE, curr_chain: List[DIE] = None, force_leaf: bool = False
) -> TypeNode:
    """
    Recurse a DIE and make a linked list of DIEs based on their type attribute
    :param die_dict: dictionary of offset -> DIE in the program
    :param die: DIE to explore
    :param curr_chain: internal use
    :param force_leaf: internal use
    :return: TypeNode linked list
    """
    root = TypeNode(die, None, [])
    curr_chain = [] if curr_chain is None else curr_chain

    curr_chain.append(die)
    type_die = die.attributes.get("DW_AT_type", None)

    if type_die and not force_leaf:
        extra_offset = die.cu.cu_offset if type_die.form != "DW_FORM_ref_addr" else 0
        next_die = die_dict[extra_offset + type_die.value]
        seen_type = True if next_die in curr_chain else False
        root.next = recurse_type(die_dict, next_die, curr_chain[:], force_leaf=seen_type)

    if die.has_children:
        for child in die.iter_children():
            root.children.append(recurse_type(die_dict, child, curr_chain[:]))

    return root


@lru_cache(1024)
def die_get_file_line(die: DIE) -> Union[Tuple[str, str, int], Tuple[None, None, None]]:
    file_index_attr = die.attributes.get("DW_AT_decl_file", None)
    if not file_index_attr:
        return None, None, None

    symbol = die.attributes["DW_AT_name"].value.decode("utf-8")
    file_index = file_index_attr.value
    lineprog = die.dwarfinfo.line_program_for_CU(die.cu)
    file = lineprog["file_entry"][file_index - 1].name.decode("utf-8")
    line = die.attributes["DW_AT_decl_line"].value

    return symbol, file, line


def die_get_name(die: DIE) -> str:
    """
    Extract the name attribute if any
    :param die: A debug info entry
    :return: name as string
    """
    name_attr = die.attributes.get("DW_AT_name", None)
    return name_attr.value.decode("utf-8") if name_attr else ""


@lru_cache(1024)
def die_get_location(die: DIE) -> Optional[int]:
    """
    Extract the location attribute if any
    :param die: A debug info entry
    :return: location as int
    """
    loc_attr = die.attributes.get("DW_AT_location", None)
    if loc_attr:
        if isinstance(loc_attr.value, list) and len(loc_attr.value) and loc_attr.value[0] == 3:
            return sum([x << 8 * i for i, x in enumerate(loc_attr.value[1:])])
    low_pc = die.attributes.get("DW_AT_low_pc", None)
    if low_pc:
        return low_pc.value
    return None


def die_get_subrange(die):
    if die.tag == "DW_TAG_array_type":
        subrange = [child for child in die.iter_children() if child.tag == "DW_TAG_subrange_type"]
        if len(subrange):
            bound = subrange[0].attributes.get("DW_AT_upper_bound", None)
            if bound:
                return bound.value + 1
    return None


def get_all_functions(dwarfinfo: DWARFInfo) -> Dict[int, str]:
    """Iterate all DIEs, add functions to dictionary with entry address as key.

    :param dwarfinfo: Dwarf info object
    :return: Lookup table for addresses vs function names
    """
    funcs = {}

    for CU in dwarfinfo.iter_CUs():
        for die in CU.iter_DIEs():
            try:
                if die.tag == "DW_TAG_subprogram":
                    lowpc = die.attributes["DW_AT_low_pc"].value
                    funcs[lowpc + 1] = die.attributes["DW_AT_name"].value.decode("UTF-8")
            except KeyError:
                continue
    return funcs


def get_all_functions_range(dwarfinfos: List[DWARFInfo]) -> RangeDict:
    """Builds a RangeDict for all functions in DWARFInfos

    Maps any valid instruction address for which there is debug information to a
    tuple of (CU, DIE, funcion_name, filename, line_number)

    Args:
        dwarfinfos (List[DWARFInfo]): List of inputs

    Returns:
        RangeDict: PC -> Function lookup map
    """
    funcs = []

    for dwarfinfo in dwarfinfos:
        for CU in dwarfinfo.iter_CUs():
            for DIE in CU.iter_DIEs():
                try:
                    if DIE.tag == "DW_TAG_subprogram":
                        lowpc = DIE.attributes["DW_AT_low_pc"].value
                        highpc_attr = DIE.attributes["DW_AT_high_pc"]
                        highpc_attr_class = describe_form_class(highpc_attr.form)
                        if highpc_attr_class == "address":
                            highpc = highpc_attr.value
                        elif highpc_attr_class == "constant":
                            highpc = lowpc + highpc_attr.value
                        else:
                            print("Error: invalid DW_AT_high_pc class:", highpc_attr_class)
                            continue

                        symbol = DIE.attributes["DW_AT_name"].value
                        file_index = DIE.attributes["DW_AT_decl_file"].value
                        lineprog = dwarfinfo.line_program_for_CU(CU)
                        file = lineprog["file_entry"][file_index - 1].name
                        line = DIE.attributes["DW_AT_decl_line"].value

                        funcs.append((lowpc, highpc, CU, DIE, symbol, file, line))
                except KeyError:
                    continue

    return RangeDict(
        {
            (lowpc, highpc): (CU, DIE, symbol.decode("utf-8"), file.decode("utf-8"), line)
            for lowpc, highpc, CU, DIE, symbol, file, line in funcs
        }
    )
