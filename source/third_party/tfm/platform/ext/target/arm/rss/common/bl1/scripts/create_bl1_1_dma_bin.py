#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
import yaml
import logging, logging.handlers
import os
import argparse

from yaml.loader import SafeLoader

# logging setup
logging.basicConfig(level=logging.WARNING, format='%(name)s - %(levelname)s - %(message)s')

parser = argparse.ArgumentParser()
parser.add_argument("--input_file", help="DMA programs high level yaml file", required=True)
parser.add_argument("--output_dir", help="Output directory", required=True)
args = parser.parse_args()

class Loader(yaml.SafeLoader):
    def __init__(self, stream):
        self._root = os.path.split(stream.name)[0]
        super(Loader, self).__init__(stream)

    # load nested yaml files (if any)
    def include(self, node):
        filepath = os.path.join(self._root, self.construct_scalar(node))
        command_name = os.path.basename(self.construct_scalar(node)).replace('.yaml', '')

        with open(filepath, 'r') as f:
            child_node = yaml.load(f, Loader)
            child_node['command_name'] = command_name
            return child_node

Loader.add_constructor('!include', Loader.include)

# calculate header word from bit fields
def calculate_header_word(header):
    final_val=0
    for i in range(len(header)):
        for key in header[i]:
            val=header[i][key]
            final_val = final_val + (val << i)
    return final_val


def resolve_store_reference(command, key, reference_string):
    return command_storage_locations[reference_string]

def resolve_exec_reference(command, key, reference_string):
    command_location = command_execution_locations[reference_string]
    # Need to set bit 0 if the link should be executed
    if key != 'SRCADDR' and key != 'DESADDR':
        try:
            if command['execute_link']:
                command_location += 1
        except KeyError:
            # Default to performing the link
            command_location += 1
    return command_location

def resolve_size_reference(command, key, reference_string):
    size = sizes[reference_string]
    if key == 'XSIZE':
        size = size // 4
        size &= 0xFFFF
        size |= size << 16
    elif key != 'SRCADDR' and key != 'DESADDR':
        # Convert size to words
        size = size // 4
    return size

def resolve_base_address_reference(command, key, reference_string):
    return location_base_addresses[reference_string]

def resolve_program_reference(command, key, reference_string):
    if "-" in reference_string:
        split = reference_string.split(" - ", maxsplit=1)
        reference_string = split[0].lstrip()
        sub_value = resolve_program_reference(command, key, split[1])
    else:
        sub_value = 0

    if "+" in reference_string:
        split = reference_string.split(" + ", maxsplit=1)
        reference_string = split[0].lstrip()
        add_value = resolve_program_reference(command, key, split[1])
    else:
        add_value = 0

    if "_store_addr" in reference_string:
        value = resolve_store_reference(command, key, reference_string.replace("_store_addr", ""))
    elif "_exec_addr" in reference_string:
        value = resolve_exec_reference(command, key, reference_string.replace("_exec_addr", ""))
    elif "_size" in reference_string:
        value = resolve_size_reference(command, key, reference_string.replace("_size", ""))
    elif "_base_address" in reference_string:
        value = resolve_base_address_reference(command, key, reference_string.replace("_base_address", ""))
    else:
        value = int(reference_string, 0)
    value = value + add_value - sub_value
    logging.debug('resolving reference {} as {}'.format(reference_string, hex(value)))
    return value

# load top level input config file
with open(args.input_file, 'r') as f:
   data = yaml.load(f, Loader)

program_count = len(data["program"])

location_base_addresses={}
location_next_addresses={}
location_word_arrays={}
output_locations={}

command_storage_locations = {}
command_execution_locations = {}
sizes = {}

reserved_keys=['command_name', 'execute_link']

# Setup locations
for location in data['locations']:
    location_word_arrays[location] = []
    location_base_addresses[location] = data['locations'][location]['base_address']
    try:
        reserved_size = data['locations'][location]['reserved']
    except KeyError:
        reserved_size = 0
    location_next_addresses[location] = data['locations'][location]['base_address'] + reserved_size
    try:
        sizes[location] = data['locations'][location]['size']
        size = sizes[location]
    except KeyError:
        size = 'unknown'
        pass
    logging.info('Location {} at {}, size {}, first usable address {}'.format(
        location,
        hex(location_base_addresses[location]),
        size,
        hex(location_next_addresses[location])))

for program in data['program']:
    commands = program['commands']
    sizes[program['name']] = 0
    for command in commands:
        command_size = sum([1 for key in command if key not in reserved_keys]) * 4
        sizes[command['command_name']] = command_size
        sizes[program['name']] += command_size
        command_storage_locations[command['command_name']] = location_next_addresses[program['storage_location']]
        command_execution_locations[command['command_name']] = location_next_addresses[program['execution_location']]
        location_next_addresses[program['storage_location']] += command_size
        # don't increment twice if the storage location is the same as the
        # execution location
        if location_next_addresses[program['storage_location']] != location_next_addresses[program['execution_location']]:
            location_next_addresses[program['execution_location']] += command_size
        output_locations[program['storage_location']] = True
        logging.info('Command {} stored at {}, executed from {}, size {}'.format(
            command['command_name'],
            hex(command_storage_locations[command['command_name']]),
            hex(command_execution_locations[command['command_name']]),
            sizes[command['command_name']],
            ))
    logging.info('Program {} stored at {}, executed from {}, size {}'.format(
        program['name'],
        hex(location_next_addresses[program['storage_location']] - sizes[program['name']]),
        hex(location_next_addresses[program['execution_location']] - sizes[program['name']]),
        sizes[program['name']],
        ))

for program in data['program']:
    commands = program['commands']
    for command in commands:
        command["Header"] = calculate_header_word(command["Header"])
        command_array = []
        for key in command:
            if key in reserved_keys:
                continue
            try:
                command_array.append(int(command[key]))
            except ValueError:
                command_array.append(int(resolve_program_reference(command, key, command[key])))
        location_word_arrays[program['storage_location']] += command_array

for location in output_locations:
    try:
        reserved_size = data['locations'][location]['reserved']
    except KeyError:
        reserved_size = 0
    if len(location_word_arrays[location]) * 4 + reserved_size > sizes[location]:
        raise Exception
    with open(os.path.join(args.output_dir, location + "_dma_ics.bin"), mode="wb") as bin_file:
        with open(os.path.join(args.output_dir, location + "_dma_ics.hex"), mode="wt") as hex_file:
            logging.info('Writing output binary for location {} to file {} of size {}'.format(
                location, os.path.join(args.output_dir, location + "_dma_ics.bin"),
                len(location_word_arrays[location]) * 4))
            logging.info('Writing output hex for location {} to file {} of size {}'.format(
                location, os.path.join(args.output_dir, location + "_dma_ics.hex"),
                len(location_word_arrays[location]) * 4))
            for word in location_word_arrays[location]:
                bin_file.write(word.to_bytes(4, byteorder='little'))
                hex_file.write(word.to_bytes(4, byteorder='big').hex() + '\n')
