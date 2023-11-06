from lxml import etree
import sys
import xml.etree.ElementTree as ET

# Define the input and output file names as pairs
file_pairs = [
    ("LM1_SF40.templ", "LM1_SF40.templ_inst", "./generated/LM1_SF40.xml"),
    ("LM1_SR03.templ", "LM1_SR03.templ_inst", "./generated/LM1_SR03.xml"),    
    ("LM1_SR04.templ", "LM1_SR04.templ_inst", "./generated/LM1_SR04.xml"),    
    ("LM1_SR05.templ", "LM1_SR05.templ_inst", "./generated/LM1_SR05.xml"),
    ("LM1_SR20.templ", "LM1_SR20.templ_inst", "./generated/LM1_SR20.xml"),    
    ("LM11_SR90.templ", "LM11_SR90.templ_inst", "./generated/LM11_SR90.xml"),    
]

# Iterate over the file pairs
for input_file, input_inst, output_file in file_pairs:
    print(f"Start processing '{input_file}'...")

    # Parse the input XML file
    parser = etree.XMLParser(
        resolve_entities=False,
        remove_blank_text=True,
        no_network=True,
        load_dtd=True,
    )
    tree = etree.parse(input_file, parser)

    # Register the xi namespace prefix
    xi_namespace = "http://www.w3.org/2001/XInclude"
    etree.register_namespace("xi", xi_namespace)

    # Perform XInclude processing
    tree.xinclude()

    # Save the result to the output XML file
    tree.write(output_file, pretty_print=True, encoding="UTF-8")

    # Set MaxInstCount
    #
    input_inst_tree = ET.parse(input_inst)
    input_inst_root = input_inst_tree.getroot()

    output_tree = ET.parse(output_file)
    output_root = output_tree.getroot()

    # Iterate over items in the first XML document
    for inst_count_item in input_inst_tree.findall('inst_count'):
        id = inst_count_item.get('id')  # LOGIC, NOT, ....
        # Find the corresponding item in the output XML document
        found_item = output_root.find(f"./AFBImplementation/AFBComponent[@Caption='{id}']");
        if found_item is not None:
            max_inst_count = inst_count_item.get('MaxInstCount')  # Get the value of 'MaxInstCount'
            found_item.set('MaxInstCount', max_inst_count)  # Set the 'MaxInstCount' attribute
        else:
            print(f"Error: AFBComponent with Caption '{id}' not found")
            sys.exit(1)
            
    # Validate that all AFBComponent items have non-zero MaxInstCount
    #
    for afb_component in output_root.findall("./AFBImplementation/AFBComponent"):
        max_inst_count = afb_component.get('MaxInstCount')
        if max_inst_count == '0':
            print(f"Error: AFBComponent '{afb_component.get('Caption')}' has MaxInstCount equal to 0")
            sys.exit(1)  # Exit the program with a non-zero status code indicating an error

	# Remove xml:base attribute from all elements
    for element in output_root.iter():
        element.attrib.pop('{http://www.w3.org/XML/1998/namespace}base', None)

    output_tree.write(output_file)
    print(f"File '{output_file}' Ok")
