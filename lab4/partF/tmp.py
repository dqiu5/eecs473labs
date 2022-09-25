def write_to_memory(inst):
    f = open("/dev/memory", "a")
    f.write(inst)
    f.close()

def move_robot(inst):
    write_to_memory(inst)
