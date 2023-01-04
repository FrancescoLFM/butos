import json

def keyboard_load(filename: str) -> dict:
    keyboard = {}

    with open(filename, "r") as infile:
        keyboard = json.load(infile)
    
    return keyboard

def keyboard_to_list(keyboard: dict, select: str) -> list:
    keys = [0 for i in range(2**8)]
    selected = keyboard[select]

    for key in selected:
        keys[int(key)] = selected[key]
    
    return keys

def c_print_array(name: str, keys: int) -> None:
    print("uint8_t " + name + "[] = {", end='')
    for i in range(len(keys)-1):
        c = repr(keys[i]) if keys[i] != "'" else "'\\''"
        print(c, end=', ')
    print(repr(keys[-1]), end='}\n')

def generate_keyboards(keyboard: dict) -> None:
    for ktype in keyboard:
        keys = keyboard_to_list(keyboard, ktype)
        c_print_array(ktype.upper() + '_KEYS', keys)
        print()

keyboard = keyboard_load("keyboard.json")
generate_keyboards(keyboard)